#include "rem_distributed.h"


RemContext* new_context()
{
    RemContext* context = malloc(sizeof(RemContext));

    context->communicator = MPI_COMM_WORLD;
    MPI_Comm_rank(context->communicator, &context->process);
    MPI_Comm_size(context->communicator, &context->nb_process);

    context->nb_vertices = 0;
    context->uf_parent = malloc(0);
    context->border_graph = new_empty_graph(0);

    return context;
}

void change_nb_vertices(RemContext* context, int nb_vertices)
{
    assert(context->uf_parent != NULL);
    assert(context->border_graph != NULL);

    context->nb_vertices = nb_vertices;

    free(context->uf_parent);
    free(context->border_graph);
    context->uf_parent = malloc(nb_vertices * sizeof(uint32_t));
    context->border_graph = new_empty_graph(nb_vertices);

    for (int i = 0 ; i < nb_vertices ; i++)
        context->uf_parent[i] = i;
}

void send_graph(FILE* file, RemContext* context)
{
    assert(context->process == 0);

    int* buffer_load = malloc(context->nb_process * sizeof(int));
    int* buffer_disp = malloc(context->nb_process * sizeof(int));
    Edge* buffer = malloc(context->nb_process * (MAX_COM_SIZE / sizeof(Edge)) * sizeof(Edge));

    for (int i = 0 ; i < context->nb_process ; i++) {
        buffer_load[i] = 0;
        buffer_disp[i] = i * (MAX_COM_SIZE / sizeof(Edge));
    }

    // Read number of edges and nodes
    uint32_t nb_nodes;
    fread(&nb_nodes, sizeof(uint32_t), 1, file);

    // Send the number of nodes to everyone
    change_nb_vertices(context, nb_nodes);
    MPI_Bcast(
        &context->nb_vertices, 1, MPI_INT,
        0, context->communicator
    );

    // Read edges from files while sending
    Edge* edges = malloc(FILE_BUFF_SIZE);
    const int max_loads_size = FILE_BUFF_SIZE / sizeof(Edge);

    // Distribute edges among process
    do {
        // Read a chunk from the file
        const unsigned load_size = fread(edges, sizeof(Edge), max_loads_size, file);

        for (unsigned i = 0 ; i <= load_size ; i++) {
            // Check if we need to insert "stop message"
            const bool is_last_edge = i == load_size && feof(file);
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
                assert(edges[i].x < context->nb_vertices);
                assert(edges[i].y < context->nb_vertices);

                // Add datas to buffer
                const int edge_owner = owner(edges[i].x);
                const int buff_pos = buffer_disp[edge_owner] + buffer_load[edge_owner];
                memcpy(&buffer[buff_pos], &edges[i], sizeof(Edge));
                buffer_load[edge_owner]++;
            }

            assert(buffer_load[owner(edges[i].x)] * sizeof(Edge) <= MAX_COM_SIZE);

            if (is_last_edge || (buffer_load[owner(edges[i].x)] + 2) * sizeof(Edge) > MAX_COM_SIZE) {
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

                // Register owned edges
                for (int i = 0 ; i < my_size ; i++) {
                    assert(owner(recv_buff[i].x) == 0 || recv_buff[i].x == context->nb_vertices);
                    assert(recv_buff[i].x <= context->nb_vertices);
                    assert(recv_buff[i].y <= context->nb_vertices);

                    register_edge(recv_buff[i], context);
                }

                free(recv_buff);

                // Empty buffers
                for (int p = 0 ; p < context->nb_process ; p++)
                    buffer_load[p] = 0;
            }
        }
    } while (!feof(file));

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

        for (int i = 0 ; i < buffer_load ; i++) {
            assert(owner(buffer[i].x) == context->process || buffer[i].x == context->nb_vertices);
            assert(buffer[i].x <= context->nb_vertices);
            assert(buffer[i].y <= context->nb_vertices);

            finished = !register_edge(buffer[i], context);
        }
    }

    free(buffer);
}

bool register_edge(Edge edge, RemContext* context)
{
    if (edge.x == context->nb_vertices) {
        // We received a fake edge
        assert(edge.y == context->nb_vertices);
        return false;
    }
    assert(owner(edge.x) == context->process || owner(edge.y) == context->process);

    if (owner(edge.x) == context->process && owner(edge.y) == context->process) {
        // We own this edge, insert it via rem's algorithm
        #define p(x) context->uf_parent[x]

        while (p(edge.x) != p(edge.y)) {
            if (p(edge.x) < p(edge.y)) {
                if (p(edge.y) == edge.y) {
                    p(edge.y) = p(edge.x);
                    continue;
                }
                else {
                    uint32_t save_p_y = p(edge.y);
                    p(edge.y) = p(edge.x);
                    edge.y = save_p_y;
                }
            }
            else {
                if (p(edge.x) == edge.x) {
                    p(edge.x) = p(edge.y);
                    continue;
                }
                else {
                    uint32_t save_p_x = p(edge.x);
                    p(edge.x) = p(edge.y);
                    edge.x = save_p_x;
                }
            }
        }

        #undef p
    }
    else {
        // This edge is in the border, we just need to keep it for later
        insert_edge(context->border_graph, edge);
    }

    return true;
}

uint32_t local_root(uint32_t node, RemContext* context)
{
    assert(node < context->nb_vertices);
    assert(owner(node) == context->process);

    #define p(x) (context->uf_parent[x])

    if (owner(p(node)) != context->process || p(node) == node) {
        return node;
    }
    else {
        const uint32_t lroot = local_root(p(node), context);
        p(node) = lroot;
        return lroot;
    }

    #undef p
}

void filter_border(RemContext* context)
{
    // Copy disjoint set structure in order not to alter it
    uint32_t* uf_copy = malloc(context->nb_vertices * sizeof(uint32_t));

    for (unsigned i = 0 ; i < context->nb_vertices ; i++)
        uf_copy[i] = context->uf_parent[i];

    // Create a new graph in which we will push edges to keep
    Graph* new_border = new_empty_graph(context->nb_vertices);

    #define p(x) uf_copy[x]
    for (int i = 0 ; i < context->border_graph->nb_edges ; i++) {
        unsigned r_x = context->border_graph->edges[i].x;
        unsigned r_y = context->border_graph->edges[i].y;

        while (p(r_x) != p(r_y)) {
            if (p(r_x) < p(r_y)) {
                if (r_y == p(r_y)) {
                    p(r_y) = p(r_x);
                    insert_edge(new_border, context->border_graph->edges[i]);
                    continue;
                }
                else {
                    uint32_t save_p_y = p(r_y);
                    p(r_y) = p(r_x);
                    r_y = save_p_y;
                }
            }
            else {
                if (r_x == p(r_x)) {
                    p(r_x) = p(r_y);
                    insert_edge(new_border, context->border_graph->edges[i]);
                    continue;
                }
                else {
                    uint32_t save_p_x = p(r_x);
                    p(r_x) = p(r_y);
                    r_x = save_p_x;
                }
            }
        }
    }
    #undef p

    delete_graph(context->border_graph);
    context->border_graph = new_border;
}

void process_distributed(RemContext* context)
{
    // Enqueue all initial edges as tasks to process
    TaskQueue todo = empty_task_queue();

    for (int i = 0 ; i < context->border_graph->nb_edges ; i++)
        push_task(&todo, context->border_graph->edges[i]);

    // Create buffers of tasks to send to other process
    Edge* to_send = malloc(context->nb_process * MAX_LOCAL_ITER * sizeof(Edge));
    int* to_send_sizes = malloc(context->nb_process * sizeof(int));
    int* to_send_displ = malloc(context->nb_process * sizeof(int));
    int* fake_sizes = malloc(context->nb_process * sizeof(int));

    for (int p = 0 ; p < context->nb_process ; p++) {
        to_send_sizes[p] = 0;
        to_send_displ[p] = p * MAX_LOCAL_ITER;
        fake_sizes[p] = -1;
    }

    // Processing loop that stops when every process has no more data to send
    while (true) {
        // If this process has nothing to do, send a fake size to everyone
        bool did_nothing = is_empty(todo);

        // Execute tasks from the queue
        #define p(x) (context->uf_parent[(x)])
        #define lroot(x) (local_root(x, context))

        for (int t = 0 ; t < MAX_LOCAL_ITER && !is_empty(todo) ; t++) {
            Edge r = pop_task(&todo);
            assert(owner(r.x) == context->process);

            r.x = lroot(r.x);

            if (p(r.x) < r.y) {
                int to_send_pos = to_send_displ[owner(r.y)] + to_send_sizes[owner(r.y)];
                to_send[to_send_pos].x = r.y;
                to_send[to_send_pos].y = p(r.x);
                to_send_sizes[owner(r.y)] += 1;
            }
            else if (p(r.x) > r.y) {
                if (p(r.x) == r.x) {
                    p(r.x) = r.y;
                }
                else {
                    int to_send_pos = to_send_displ[owner(p(r.x))] + to_send_sizes[owner(p(r.x))];
                    to_send[to_send_pos].x = p(r.x);
                    to_send[to_send_pos].y = r.y;
                    to_send_sizes[owner(p(r.x))] += 1;

                    p(r.x) = r.y;
                }
            }

        }

        #undef lroot
        #undef p

        // Share buffer sizes with other process
        int* recv_sizes = malloc(context->nb_process * sizeof(int));
        int* recv_displ = malloc(context->nb_process * sizeof(int));
        MPI_Alltoall(
            did_nothing ? fake_sizes : to_send_sizes, 1, MPI_INT,
            recv_sizes, 1, MPI_INT,
            context->communicator
        );

        int end_count = 0; // number of process who had nothing to do
        int total_size = 0;
        for (int p = 0 ; p < context->nb_process ; p++) {
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
        if (end_count == context->nb_process) {
            free(recv_sizes);
            free(recv_displ);
            break;
        }

        // Share tasks with other process
        Edge* recv_datas = malloc(total_size * sizeof(Edge));
        MPI_Alltoallv(
            to_send, to_send_sizes, to_send_displ, MPI_EDGE,
            recv_datas, recv_sizes, recv_displ, MPI_EDGE,
            context->communicator
        );

        // Load tasks in the queue
        for (int t = 0 ; t < total_size ; t++)
            push_task(&todo, recv_datas[t]);

        // Empty send buffers
        for (int p = 0 ; p < context->nb_process ; p++)
            to_send_sizes[p] = 0;

        // Free reception buffers
        free(recv_sizes);
        free(recv_displ);
        free(recv_datas);
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
    for (unsigned i = 0 ; i < context->nb_vertices ; i++)
        if (owner(i) == context->process)
            my_size++;

    Edge* edges = malloc(my_size * sizeof(Edge));
    int edge_pos = 0;
    for (unsigned i = 0 ; i < context->nb_vertices ; i++) {
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
        // Print number of nodes
        printf("%d\n", context->nb_vertices);

        // Print edges
        for (int i = 0 ; i < total_size ; i++)
            printf("%u %u\n", all_edges[i].x, all_edges[i].y);
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
        printf("# Border graph contains %d edges.\n", context->border_graph->nb_edges);

    // for (int i = 0 ; i < context->border_graph->nb_edges ; i++)
    //     printf("# (%u, %u)\n", context->border_graph->edges[i].x, context->border_graph->edges[i].y);
}
