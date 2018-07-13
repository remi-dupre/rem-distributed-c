#include "rem_distributed.h"

extern inline void insert_edge(Graph*, Edge);


RemContext* new_context()
{
    RemContext* context = malloc(sizeof(RemContext));

    context->communicator = MPI_COMM_WORLD;
    MPI_Comm_rank(context->communicator, &context->process);
    MPI_Comm_size(context->communicator, &context->nb_process);

    context->nb_vertices = 0;
    context->uf_parent = malloc(0);

    context->border_graph = new_empty_graph(0);
    context->buffer_graph = new_empty_graph(0);

    #ifdef TIMERS
        context->nb_steps = 0;
        context->time_step_proc = malloc(100000 * sizeof(time_t));
        context->time_step_comm = malloc(100000 * sizeof(time_t));
    #endif

    return context;
}

void change_nb_vertices(RemContext* context, Node nb_vertices)
{
    assert(context->uf_parent != NULL);
    assert(context->border_graph != NULL);

    context->nb_vertices = nb_vertices;

    free(context->uf_parent);
    free(context->border_graph);
    free(context->buffer_graph);

    // allocates memory only for owned vertices
    size_t owned_vertices = (nb_vertices + context->nb_process - context->process - 1) / context->nb_process;
    context->uf_parent = malloc(owned_vertices * sizeof(Node));

    if (context->uf_parent == NULL) {
        perror("not enough memory");
        exit(-1);
    }

    context->border_graph = new_empty_graph(nb_vertices);
    context->buffer_graph = new_empty_graph(nb_vertices);

    #pragma omp parallel for num_threads(NB_THREADS)
    for (Node i = 0 ; i < owned_vertices ; i++)
        context->uf_parent[i] = i;
}

void send_graph(FILE* file, RemContext* context)
{
    assert(context->process == 0);

    int* buffer_load = malloc(context->nb_process * sizeof(int));
    int* buffer_disp = malloc(context->nb_process * sizeof(int));
    size_t* edges_received = malloc(context->nb_process * sizeof(size_t));
    Edge* buffer = malloc(context->nb_process * (MAX_COM_SIZE / sizeof(Edge)) * sizeof(Edge));

    for (int i = 0 ; i < context->nb_process ; i++) {
        buffer_load[i] = 0;
        buffer_disp[i] = i * (MAX_COM_SIZE / sizeof(Edge));
        edges_received[i] = 0;
    }

    // Read number of edges and nodes
    Node nb_nodes;
    fread(&nb_nodes, sizeof(Node), 1, file);

    // Send the number of nodes to everyone
    change_nb_vertices(context, nb_nodes);
    MPI_Bcast(
        &context->nb_vertices, 1, MPI_NODE,
        0, context->communicator
    );

    // Node* f = malloc(context->nb_vertices * sizeof(Node));
    // Node x = 0;

    // for (Node i = 0 ; i < context->nb_vertices ; i++) {
        // f[i] = x;
        // x = f_next(x, context);
    // }

    // Read edges from files while sending
    Edge* edges = malloc(FILE_BUFF_SIZE);
    const int max_loads_size = FILE_BUFF_SIZE / sizeof(Edge);

    // Distribute edges among process
    do {
        // Read a chunk from the file
        const Node load_size = fread(edges, sizeof(Edge), max_loads_size, file);

        for (Node i = 0 ; i <= load_size ; i++) {
            // Check if we need to insert "stop message"
            const bool is_last_edge = i == load_size && feof(file);
            int edge_owner = -1;

            if (i == load_size && !is_last_edge) {
                // This last step of the loop shouldn't insert anything
                break;
            }
            else if (is_last_edge) {
                // Insert a fake edge
                Edge fake_edge = {
                    .x = context->nb_vertices,
                    .y = context->nb_vertices
                };
                for (int p = 0 ; p < context->nb_process ; p++) {
                    const int buff_pos = buffer_disp[p] + buffer_load[p];
                    memcpy(&buffer[buff_pos], &fake_edge, sizeof(Edge));
                    buffer_load[p]++;
                }
            }
            else {
                // Insert a regular edge to buffer
                // edges[i].x = f[edges[i].x];
                // edges[i].y = f[edges[i].y];

                assert(edges[i].x < context->nb_vertices);
                assert(edges[i].y < context->nb_vertices);

                // Add datas to buffer
                const int owner_x = owner(edges[i].x);
                const int owner_y = owner(edges[i].y);

                edge_owner = (edges_received[owner_x] < edges_received[owner_y]) ? owner_x : owner_y;
                edges_received[edge_owner]++;

                const int buff_pos = buffer_disp[edge_owner] + buffer_load[edge_owner];
                memcpy(&buffer[buff_pos], &edges[i], sizeof(Edge));
                buffer_load[edge_owner]++;
            }

            if (!is_last_edge)
                assert(buffer_load[edge_owner] * sizeof(Edge) <= MAX_COM_SIZE);

            if (is_last_edge || (buffer_load[edge_owner] + 2) * sizeof(Edge) > MAX_COM_SIZE) {
                // Send buffer sizes
                int my_size;
                MPI_Scatter(
                    buffer_load, 1, MPI_INT,
                    &my_size, 1, MPI_INT,
                    0, context->communicator
                );

                // Send data
                Edge* recv_buff = malloc(my_size * sizeof(Edge));

                if (recv_buff == NULL) {
                    perror("Not enough memory to receive data");
                    exit(-1);
                }

                MPI_Scatterv(
                    buffer, buffer_load, buffer_disp, MPI_EDGE,
                    recv_buff, my_size, MPI_EDGE,
                    0, context->communicator
                );

                // Eliminate fake edge
                if (my_size > 0 && recv_buff[my_size-1].x == context->nb_vertices) {
                    assert(recv_buff[my_size-1].y == context->nb_vertices);
                    my_size--;
                }

                // Save owned edges
                for (int i = 0 ; i < my_size ; i++) {
                    assert(owner(recv_buff[i].x) == 0 || owner(recv_buff[i].y) == 0);
                    assert(recv_buff[i].x < context->nb_vertices);
                    assert(recv_buff[i].y < context->nb_vertices);

                    if (owner(recv_buff[i].x) != 0) {
                        const Node z = recv_buff[i].x;
                        recv_buff[i].x = recv_buff[i].y;
                        recv_buff[i].y = z;
                    }
                }
                insert_edges(context->buffer_graph, recv_buff, my_size);

                free(recv_buff);

                // Empty buffers
                for (int p = 0 ; p < context->nb_process ; p++)
                    buffer_load[p] = 0;
            }
        }
    } while (!feof(file));

    //free(f);
    free(buffer_load);
    free(buffer_disp);
    free(edges_received);
    free(buffer);
    free(edges);
}

void recv_graph(RemContext* context)
{
    assert(context->process != 0);

    // Receive graph's total number of nodes
    MPI_Bcast(
        &context->nb_vertices, 1, MPI_NODE,
        0, context->communicator
    );
    change_nb_vertices(context, context->nb_vertices);

    // Receive all edges
    bool finished = false;

    Edge* buffer = malloc(MAX_COM_SIZE);

    if (buffer == NULL) {
        perror("Not enough memory to init receive buffer");
        exit(-1);
    }

    int buffer_load;

    while (!finished) {
        // Receive size of incoming datas
        MPI_Scatter(
            NULL, 0, MPI_INT,
            &buffer_load, 1, MPI_INT,
            0, context->communicator
        );
        assert(buffer_load * sizeof(Edge) <= MAX_COM_SIZE);

        // Receive datas
        MPI_Scatterv(
            NULL, NULL, NULL, MPI_EDGE,
            buffer, buffer_load, MPI_EDGE,
            0, context->communicator
        );

        // Just pass if the buffer is empty
        if (buffer_load == 0) {
            continue;
        }

        // If there is a fake edge it is the last one
        if (buffer[buffer_load-1].x == context->nb_vertices) {
            assert(buffer[buffer_load-1].y == context->nb_vertices);
            finished = true;
            buffer_load--;
        }

        // Save new edges
        for (int i = 0 ; i < buffer_load ; i++) {
            assert(owner(buffer[i].x) == context->process || owner(buffer[i].y) == context->process);
            assert(buffer[i].x < context->nb_vertices);
            assert(buffer[i].y < context->nb_vertices);

            if (owner(buffer[i].x) != context->process) {
                const Node z = buffer[i].x;
                buffer[i].x = buffer[i].y;
                buffer[i].y = z;
            }
        }
        insert_edges(context->buffer_graph, buffer, buffer_load);

        for (size_t i = 0 ; i < context->buffer_graph->nb_edges ; i++)
            assert(context->buffer_graph->edges[i].x < context->nb_vertices);

    }

    free(buffer);
}

void flush_buffered_graph(RemContext* context)
{
    #ifdef TIMERS
        context->time_flushing = time_ms();

        context->prefilter_size = 0;
        context->postfilter_size = 0;
    #endif

    Node actual_nb_vertices = (context->nb_vertices + context->nb_process - context->process - 1) / context->nb_process;
    Graph* border_graph = context->border_graph;
    Graph* internal_graph = new_empty_graph((context->nb_vertices + context->nb_process - 1) / context->nb_process);

    Node* border_components = malloc(actual_nb_vertices * sizeof(Node));

    if (border_components == NULL) {
        perror("Not enough memory available (border_components)");
        exit(-1);
    }

    for (Node i = 0 ; i < actual_nb_vertices ; i++)
        border_components[i] = i;

    int process = context->process;
    int nb_process = context->nb_process;

    size_t nb_edges = context->buffer_graph->nb_edges;
    Edge* edges = context->buffer_graph->edges;
    Node* uf_parent = context->uf_parent;

    // Prepare border_graph to catch filtered edges
    border_graph->nb_edges = 0;

    #define own(x) ((int) (x) % nb_process)

    #pragma omp parallel num_threads(NB_THREADS)
    {
        // Store localy border edges catched by this process
        Graph* local_internal_graph_ptr = new_empty_graph(actual_nb_vertices);
        Graph local_internal_graph = *local_internal_graph_ptr;
        free(local_internal_graph_ptr);

        Graph* local_border_graph = new_empty_graph(context->nb_vertices);

        #ifdef TIMERS
            context->time_inserting = time_ms();
        #endif

        #pragma omp for
        for (size_t i = 0 ; i < nb_edges ; i++) {
            assert(own(edges[i].x) == process);

            if (own(edges[i].y) == process) {
                // We own this edge, insert it via rem's algorithm
                edges[i].x /= nb_process;
                edges[i].y /= nb_process;

                if (!rem_insert_inplace(&edges[i], uf_parent)) {
                    insert_edge(&local_internal_graph, edges[i]);
                }
            }
        // #ifdef TIMERS
        //     }
        //
        //     #pragma omp barrier
        //     #pragma omp single
        //     {
        //         context->time_inserting = time_ms() - context->time_inserting;
        //         context->time_filtering = time_ms();
        //     }
        //
        //     #pragma omp for
        //     for (size_t i = 0 ; i < nb_edges ; i++) {
        //         if (own(edges[i].y) == process);
        // #endif
            else {
                if (true) {//!rem_insert(edges[i], border_components)) {
                    #ifdef TIMERS
                        #pragma omp atomic
                        context->postfilter_size++;
                    #endif

                    insert_edge(local_border_graph, edges[i]);
                }
            }
        }

        #ifdef TIMERS
            #pragma omp barrier
            #pragma omp single
            {
                context->time_filtering = time_ms() - context->time_filtering;
                context->prefilter_size = nb_edges;
            }
        #endif

        // Filter edges pushed to local_internal_graph
        #pragma omp barrier
        size_t new_size = 0;

        for (size_t i = 0 ; i < local_internal_graph.nb_edges ; i++) {
            if (!rem_compare(local_internal_graph.edges[i], uf_parent)) {
                local_internal_graph.edges[new_size] = local_internal_graph.edges[i];
                new_size++;
            }
        }
        local_internal_graph.nb_edges = new_size;

        // position where this thread will insert in full graphs
        size_t border_pos_insertion, internal_pos_insertion;

        #pragma omp critical (prepare_insertion)
        {
            border_pos_insertion = border_graph->nb_edges;
            border_graph->nb_edges += local_border_graph->nb_edges;

            internal_pos_insertion = internal_graph->nb_edges;
            internal_graph->nb_edges += local_internal_graph.nb_edges;
        }

        #pragma omp barrier

        #pragma omp single nowait
        reserve(border_graph, border_graph->nb_edges);

        #pragma omp single
        reserve(internal_graph, internal_graph->nb_edges);

        assert(border_pos_insertion + local_border_graph->nb_edges <= border_graph->nb_edges);
        assert(internal_pos_insertion + local_internal_graph.nb_edges <= internal_graph->nb_edges);

        memcpy(
            border_graph->edges + border_pos_insertion,
            local_border_graph->edges,
            local_border_graph->nb_edges * sizeof(Edge)
        );

        memcpy(
            internal_graph->edges + internal_pos_insertion,
            local_internal_graph.edges,
            local_internal_graph.nb_edges * sizeof(Edge)
        );

        delete_graph(local_border_graph);
        free(local_internal_graph.edges);
        // delete_graph(&local_internal_graph);
    }

    #ifdef TIMERS
        context->time_flushing = time_ms() - context->time_flushing;
    #endif

    rem_shared_update(internal_graph->edges, internal_graph->nb_edges, uf_parent, NB_THREADS, true);

    for (Node i = 0 ; i < actual_nb_vertices ; i++) {
        uf_parent[i] *= nb_process;
        uf_parent[i] += process;
    }

    delete_graph(internal_graph);
    delete_graph(context->buffer_graph);

    context->buffer_graph = new_empty_graph(context->nb_vertices);
    free(border_components);

    #undef own
}

void filter_border(RemContext* context)
{
    #ifdef TIMERS
        context->time_filtering = time_ms();
    #endif

    size_t nb_edges = context->border_graph->nb_edges;
    Edge* edges = context->border_graph->edges;

    // Copy disjoint set structure in order not to alter it
    Node* uf_copy = malloc(context->nb_vertices * sizeof(Node));
    memcpy(uf_copy, context->uf_parent, context->nb_vertices * sizeof(Node));

    // Create a new graph in which we will push edges to keep
    Graph* new_border = new_empty_graph(context->nb_vertices);

    for (size_t i = 0 ; i < nb_edges ; i++)
        if (!rem_insert(edges[i], uf_copy))
            insert_edge(new_border, edges[i]);

    #ifdef TIMERS
        context->prefilter_size = context->border_graph->nb_edges;
        context->postfilter_size = new_border->nb_edges;
    #endif

    delete_graph(context->border_graph);
    context->border_graph = new_border;

    #ifdef TIMERS
        context->time_filtering = time_ms() - context->time_filtering;
    #endif
}

void flatten(RemContext* context)
{
    int process = context->process;
    int nb_process = context->nb_process;

    Node nb_vertices = context->nb_vertices;
    Node actual_nb_vertices = (nb_vertices + nb_process - process - 1) / nb_vertices;
    Node* uf_parent = context->uf_parent;

    #define p(x) uf_parent[x / nb_process]
    #define owning(x) ((int) ((x) % nb_process) == process)

    for (Node i = 0 ; i < actual_nb_vertices ; i++) {
        if (owning(p(i)) && owning(p(p(i)))) {
            p(i) = p(p(i));
        }
    }

    #undef owning
    #undef p
}

static inline Node local_root(Node node, Node* uf_parent, int process, int nb_process)
{
    #define own(x) ((int) (x) % nb_process)
    #define p(x) (uf_parent[x / nb_process])

    int anc_container_size = 0;
    Node* ancesters = NULL;
    int nb_ancesters = 0;

    while (p(node) != node && own(p(node)) == process) {
        if (nb_ancesters == anc_container_size) {
            anc_container_size = 2 * anc_container_size + 1;
            ancesters = realloc(ancesters, anc_container_size * sizeof(Node));
        }
        ancesters[nb_ancesters] = node;
        nb_ancesters++;

        node = p(node);
    }

    for (int i = 0 ; i < nb_ancesters - 1 ; i++)
        p(ancesters[i]) = node;

    return node;

    #undef p
    #undef own
}

void process_distributed(RemContext* context)
{
    const RemContext context_cpy = *context;

    // Enqueue all initial edges as tasks to process
    TaskHeap todo = empty_task_heap();
    push_tasks(&todo, context_cpy.border_graph->edges, context_cpy.border_graph->nb_edges);

    // Create buffers of tasks to send to other process
    Edge* to_send = malloc(context_cpy.nb_process * MAX_LOCAL_ITER * NB_THREADS * sizeof(Edge));
    int* to_send_sizes = malloc(context_cpy.nb_process * sizeof(int));
    int* to_send_displ = malloc(context_cpy.nb_process * sizeof(int));
    int* fake_sizes = malloc(context_cpy.nb_process * sizeof(int));

    if (to_send == NULL) {
        perror("Not enough memory available (to_send)");
        exit(-1);
    }

    for (int p = 0 ; p < context_cpy.nb_process ; p++) {
        to_send_sizes[p] = 0;
        to_send_displ[p] = p * MAX_LOCAL_ITER * NB_THREADS;
        fake_sizes[p] = -1;
    }

    // Processing loop that stops when every process has no more data to send
    while (true) {
        // If this process has nothing to do, send a fake size to everyone
        bool did_nothing = is_empty_heap(todo);

        // Execute tasks from the queue
        #define p(x) (context_cpy.uf_parent[(x) / context_cpy.nb_process])
        #define own(x) ((int) (x) % context_cpy.nb_process)
        #define lroot(x) (local_root(x, context_cpy.uf_parent, context_cpy.process, context_cpy.nb_process))

        #ifdef TIMERS
            context->time_step_proc[context->nb_steps] = time_ms();
        #endif

        // Pull edges from the heap, parallelized
        #pragma omp parallel num_threads(NB_THREADS)
        {
            int thread_id = omp_get_thread_num();
            size_t nb_pop = (todo.nb_tasks + thread_id) / NB_THREADS;

            if (nb_pop > MAX_LOCAL_ITER)
                nb_pop = MAX_LOCAL_ITER;

            size_t begin, end;

            #pragma omp critical (pull_tasks)
            {
                end = todo.nb_tasks;
                todo.nb_tasks -= nb_pop;
                begin = todo.nb_tasks;
            }

            assert(begin <= end);
            Graph* loc_to_send[context_cpy.nb_process];

            for (int i = 0 ; i < context_cpy.nb_process ; i++)
                loc_to_send[i] = new_empty_graph(context_cpy.nb_vertices);

            // Handle tasks
            for (size_t i = begin ; i < end ; i++) {
                Edge r = todo.tasks[i];
                assert(own(r.x) == context_cpy.process);

                r.x = lroot(r.x);

                if (p(r.x) < r.y) {
                    Edge inserted = {.x = r.y, .y = p(r.x)};
                    insert_edge(loc_to_send[own(inserted.x)], inserted);
                }
                else if (p(r.x) > r.y) {
                    if (p(r.x) == r.x) {
                        p(r.x) = r.y;
                    }
                    else {
                        Edge inserted = {.x = p(r.x), .y = r.y};
                        insert_edge(loc_to_send[own(inserted.x)], inserted);
                        p(r.x) = r.y;
                    }
                }
            }

            // Insert new tasks in the full structure
            size_t insert_pos[context_cpy.nb_process];

            #pragma omp critical (push_tasks)
            {
                for (int p = 0 ; p < context_cpy.nb_process ; p++) {
                    assert(to_send_sizes[p] < MAX_LOCAL_ITER * NB_THREADS);
                    insert_pos[p] = to_send_displ[p] + to_send_sizes[p];
                    to_send_sizes[p] += loc_to_send[p]->nb_edges;
                }
            }

            for (int p = 0 ;  p < context_cpy.nb_process ; p++) {
                memcpy(
                    to_send + insert_pos[p],
                    loc_to_send[p]->edges,
                    loc_to_send[p]->nb_edges * sizeof(Edge)
                );
                delete_graph(loc_to_send[p]);
            }
        }

        #ifdef TIMERS
            context->time_step_proc[context->nb_steps] = time_ms() - context->time_step_proc[context->nb_steps];
        #endif

        #undef lroot
        #undef own
        #undef p

        #ifdef TIMERS
            context->time_step_comm[context->nb_steps] = time_ms();
        #endif

        // Share buffer sizes with other process
        int* recv_sizes = malloc(context->nb_process * sizeof(int));
        int* recv_displ = malloc(context->nb_process * sizeof(int));
        MPI_Alltoall(
            did_nothing ? fake_sizes : to_send_sizes, 1, MPI_INT,
            recv_sizes, 1, MPI_INT,
            context_cpy.communicator
        );

        int end_count = 0; // number of process who had nothing to do
        int total_size = 0;
        for (int p = 0 ; p < context_cpy.nb_process ; p++) {
            recv_displ[p] = total_size;

            if (recv_sizes[p] == -1) {
                recv_sizes[p] = 0;
                end_count++;
            }
            else {
                total_size += recv_sizes[p];
            }
        }

        // If no one sends data, the algorithm is done
        if (end_count == context_cpy.nb_process) {
            free(recv_sizes);
            free(recv_displ);
            break;
        }

        // Share tasks with other process
        Edge* recv_datas = malloc(total_size * sizeof(Edge));

        if (recv_datas == NULL) {
            perror("Not enough memory available (recv_datas)");
            exit(-1);
        }

        MPI_Alltoallv(
            to_send, to_send_sizes, to_send_displ, MPI_EDGE,
            recv_datas, recv_sizes, recv_displ, MPI_EDGE,
            context_cpy.communicator
        );

        // Load tasks in the queue
        push_tasks(&todo, recv_datas, total_size);

        // Empty send buffers
        for (int p = 0 ; p < context_cpy.nb_process ; p++)
            to_send_sizes[p] = 0;

        // Free reception buffers
        free(recv_sizes);
        free(recv_displ);
        free(recv_datas);

        #ifdef TIMERS
            context->time_step_comm[context->nb_steps] = time_ms() - context->time_step_comm[context->nb_steps];
            context->nb_steps++;
        #endif
    }

    // Free allocated memory
    free(to_send);
    free(to_send_sizes);
    free(to_send_displ);
    free(fake_sizes);
}

void debug_structure(const RemContext* context)
{
    // Collect the list of nodes owned by this process
    int my_size = 0;
    for (Node i = 0 ; i < context->nb_vertices ; i++)
        if (owner(i) == context->process)
            my_size++;

    Edge* edges = malloc(my_size * sizeof(Edge));
    int edge_pos = 0;
    for (Node i = 0 ; i < context->nb_vertices ; i++) {
        if (owner(i) == context->process) {
            edges[edge_pos].x = i;
            edges[edge_pos].y = context->uf_parent[i / context->nb_process];
            edge_pos++;
        }
    }

    // Collect size from all process
    int* sizes = malloc(context->nb_process * sizeof(int));
    int* displ = malloc(context->nb_process * sizeof(int));

    MPI_Gather(
        &my_size, 1, MPI_INT,
        sizes, 1, MPI_INT,
        0, context->communicator
    );

    int total_size = 0;
    if (context->process == 0) {
        for (int p = 0 ; p < context->nb_process ; p++) {
            displ[p] = total_size;
            total_size += sizes[p];
        }
    }
    else {
        for (int p = 0 ; p < context->nb_process ; p++)
            displ[p] = sizes[p] = 0;
    }

    // Collect the list of edges
    Edge* all_edges = malloc(total_size * sizeof(Edge));
    MPI_Gatherv(
        edges, my_size, MPI_EDGE,
        all_edges, sizes, displ, MPI_EDGE,
        0, context->communicator
    );

    // Display edges
    if (context->process == 0) {
        // Node* g = malloc(context->nb_vertices * sizeof(Node));
        // Node x = 0;

        // for (Node i = 0 ; i < context->nb_vertices ; i++) {
            // g[x] = i;
            // x = f_next(x, context);
        // }

        // Print number of nodes
        printf("%lu\n", context->nb_vertices);

        // Print edges
        for (int i = 0 ; i < total_size ; i++)
            // printf("%lu %lu\n", g[all_edges[i].x], g[all_edges[i].y]);
            printf("%lu %lu\n", all_edges[i].x, all_edges[i].y);
        //free(g);
    }

    free(edges);
    free(sizes);
    free(displ);
}

void debug_context(const RemContext* context)
{
    printf("## Process %d\n", context->process);

    if (context->border_graph == NULL)
        printf("# Border graph has been flushed.\n");
    else
        printf("# Border graph contains %ld edges.\n", context->border_graph->nb_edges);

    // for (int i = 0 ; i < context->border_graph->nb_edges ; i++)
    //     printf("# (%lu, %lu)\n", context->border_graph->edges[i].x, context->border_graph->edges[i].y);
}

void debug_timers(const RemContext* context)
{
    context = context; // Just avoid "unused variable" warning

    #ifdef TIMERS

    FILE* file;

    while ((file = fopen("mpitest.time.csv", "a")) == NULL);
    fprintf(
        file, "%d;%lu;%lu;%lu;%d;%d\n", context->process,
        context->time_flushing, context->time_inserting,
        context->time_filtering,
        context->prefilter_size, context->postfilter_size
    );
    fclose(file);

    while ((file = fopen("mpitest.steps.csv", "a")) == NULL);
    for (int iteration = 0 ; iteration < context->nb_steps ; iteration++) {
        fprintf(file, "%d;%d;", context->process, iteration);
        fprintf(file, "%lu;%lu\n", context->time_step_proc[iteration], context->time_step_comm[iteration]);
    }

    fclose(file);

    #endif
}
