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

    context->uf_parent = malloc(nb_vertices * sizeof(Node));
    context->border_graph = new_empty_graph(nb_vertices);
    context->buffer_graph = new_empty_graph(nb_vertices);

    for (Node i = 0 ; i < nb_vertices ; i++)
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
        &context->nb_vertices, 1, MPI_INT,
        0, context->communicator
    );

    Node* f = malloc(context->nb_vertices * sizeof(Node));
    Node x = 0;

    for (Node i = 0 ; i < context->nb_vertices ; i++) {
        f[i] = x;
        x = f_next(x, context);
    }

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
            int edge_owner;

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
                edges[i].x = f[edges[i].x];
                edges[i].y = f[edges[i].y];

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

    free(f);
    free(buffer_load);
    free(buffer);
    free(edges);
}

void recv_graph(RemContext* context)
{
    assert(context->process != 0);

    // Receive graph's total number of nodes
    MPI_Bcast(
        &context->nb_vertices, 1, MPI_INT,
        0, context->communicator
    );
    change_nb_vertices(context, context->nb_vertices);

    // Receive all edges
    bool finished = false;

    Edge* buffer = malloc(MAX_COM_SIZE);
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
        Graph* tmp = new_empty_graph(context->nb_vertices);
    #endif

    #ifndef TIMERS
        // We will filter on the fly

    #endif

    Graph* border_graph = context->border_graph;

    Node* border_components = malloc(context->nb_vertices * sizeof(Node));
    for (Node i = 0 ; i < context->nb_vertices ; i++)
        border_components[i] = i;

    int process = context->process;
    int nb_process = context->nb_process;

    size_t nb_edges = context->buffer_graph->nb_edges;
    Edge* edges = context->buffer_graph->edges;
    Node* uf_parent = context->uf_parent;

    #define own(x) ((int) (x) % nb_process)

    #define NB_THREADS 4
    #ifndef NB_THREADS
        #define NB_THREADS 1
    #endif
    #pragma omp parallel for num_threads(NB_THREADS)
    for (size_t i = 0 ; i < nb_edges ; i++) {
        assert(own(edges[i].x) == process);

        // Catch internal edges
        if (own(edges[i].y) == process) {
            #ifdef TIMERS
                // If we keep track of timings, do the rem insertion later
                #pragma omp critical
                insert_edge(tmp, edges[i]);
            #else
                // We own this edge, insert it via rem's algorithm
                #pragma omp critical
                rem_insert(edges[i], uf_parent);
            #endif
        }
        // else {
        //     #ifdef TIMERS
        //         // This edge is in the border, we just need to keep it for later
        //         #pragma omp critical
        //         insert_edge(border_graph, edges[i]);
        //     #else
        //         // Insert the edge only if it provides information
        //         if (!rem_insert(edges[i], border_components)) {
        //             #pragma omp critical
        //             insert_edge(border_graph, edges[i]);
        //         }
        //     #endif
        // }
    }

    // Catch border edges
    #pragma omp parallel for num_threads(NB_THREADS)
    for (size_t i = 0 ; i < nb_edges ; i++) {
        assert(own(edges[i].x) == process);

        if (own(edges[i].y) != process && !rem_insert(edges[i], border_components)) {
            insert_edge(border_graph, edges[i]);
        }
    }

    // Process timers and free memory
    #ifdef TIMERS
        context->time_flushing = time_ms() - context->time_flushing;

        context->time_inserting = time_ms();
        rem_update(tmp->edges, tmp->nb_edges, uf_parent);
        context->time_inserting = time_ms() - context->time_inserting;

        delete_graph(tmp);
        // filter_border(context);
    #endif

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
    Node* uf_parent = context->uf_parent;

    #define p(x) uf_parent[x]
    #define owning(x) ((int) ((x) % nb_process) == process)

    for (Node i = 0 ; i < nb_vertices ; i++) {
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
    #define p(x) (uf_parent[x])

    static int anc_container_size = 0;
    static Node* ancesters = NULL;
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
    Edge* to_send = malloc(context_cpy.nb_process * MAX_LOCAL_ITER * sizeof(Edge));
    int* to_send_sizes = malloc(context_cpy.nb_process * sizeof(int));
    int* to_send_displ = malloc(context_cpy.nb_process * sizeof(int));
    int* fake_sizes = malloc(context_cpy.nb_process * sizeof(int));

    for (int p = 0 ; p < context_cpy.nb_process ; p++) {
        to_send_sizes[p] = 0;
        to_send_displ[p] = p * MAX_LOCAL_ITER;
        fake_sizes[p] = -1;
    }

    // Processing loop that stops when every process has no more data to send
    while (true) {
        // If this process has nothing to do, send a fake size to everyone
        bool did_nothing = is_empty_heap(todo);

        // Execute tasks from the queue
        #define p(x) (context_cpy.uf_parent[(x)])
        #define own(x) ((int) (x) % context_cpy.nb_process)
        #define lroot(x) (local_root(x, context_cpy.uf_parent, context_cpy.process, context_cpy.nb_process))

        #ifdef TIMERS
            context->time_step_proc[context->nb_steps] = time_ms();
        #endif

        int t = 0;
        while (t < MAX_LOCAL_ITER && !is_empty_heap(todo)) {
            Edge r = pop_task(&todo);
            assert(own(r.x) == context_cpy.process);

            t++;
            r.x = lroot(r.x);

            if (p(r.x) < r.y) {
                int to_send_pos = to_send_displ[own(r.y)] + to_send_sizes[own(r.y)];
                to_send[to_send_pos].x = r.y;
                to_send[to_send_pos].y = p(r.x);
                to_send_sizes[own(r.y)] += 1;
            }
            else if (p(r.x) > r.y) {
                if (p(r.x) == r.x) {
                    p(r.x) = r.y;
                }
                else {
                    int to_send_pos = to_send_displ[own(p(r.x))] + to_send_sizes[own(p(r.x))];
                    to_send[to_send_pos].x = p(r.x);
                    to_send[to_send_pos].y = r.y;
                    to_send_sizes[own(p(r.x))] += 1;

                    p(r.x) = r.y;
                }
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
            edges[edge_pos].y = context->uf_parent[i];
            edge_pos++;
        }
    }
    assert(edge_pos == my_size);

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
        Node* g = malloc(context->nb_vertices * sizeof(Node));
        Node x = 0;

        for (Node i = 0 ; i < context->nb_vertices ; i++) {
            g[x] = i;
            x = f_next(x, context);
        }

        // Print number of nodes
        printf("%u\n", context->nb_vertices);

        // Print edges
        for (int i = 0 ; i < total_size ; i++)
            printf("%u %u\n", g[all_edges[i].x], g[all_edges[i].y]);

        free(g);
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
    //     printf("# (%u, %u)\n", context->border_graph->edges[i].x, context->border_graph->edges[i].y);
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
