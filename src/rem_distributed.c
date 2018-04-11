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
    char* buffer = malloc(context->nb_process * MAX_COM_SIZE * sizeof(char));

    for (int i = 0 ; i < context->nb_process ; i++) {
        buffer_load[i] = 0;
        buffer_disp[i] = i * MAX_COM_SIZE;
    }

    // Read number of edges and nodes
    uint32_t nb_nodes, nb_edges;
    fread(&nb_nodes, sizeof(uint32_t), 1, file);
    fread(&nb_edges, sizeof(uint32_t), 1, file);

    // Send the number of nodes to everyone
    change_nb_vertices(context, nb_nodes);
    MPI_Bcast(
        &context->nb_vertices, 1, MPI_INT,
        0, context->communicator
    );

    // Read all edges at once
    Edge* edges = malloc(nb_edges * sizeof(Edge));
    fread(edges, sizeof(Edge), nb_edges, file);

    // Distribute edges among process
    for (uint i = 0 ; i < nb_edges ; i++) {
        // Add datas to buffer
        int edge_owner = owner(edges[i].x);
        int buff_pos = buffer_disp[edge_owner] + buffer_load[edge_owner];
        memcpy(&buffer[buff_pos], &edges[i], sizeof(Edge));
        buffer_load[edge_owner] += sizeof(Edge);

        // If the last edge is reached, add fake edge to send to everyone
        if (i + 1 == nb_edges) {
            Edge fake_edge = {
                .x = context->nb_vertices,
                .y = context->nb_vertices
            };
            for (int p = 0 ; p < context->nb_process ; p++) {
                int buff_pos = buffer_disp[p] + buffer_load[p];
                memcpy(&buffer[buff_pos], &fake_edge, sizeof(Edge));
                buffer_load[p] += sizeof(Edge);
            }
        }

        assert(buffer_load[edge_owner] <= MAX_COM_SIZE);
        if (buffer_load[edge_owner] + 2*sizeof(Edge) > MAX_COM_SIZE || i + 1  == nb_edges) {
            // Send buffer sizes
            int my_size;
            MPI_Scatter(
                buffer_load, 1, MPI_INT,
                &my_size, 1, MPI_INT,
                0, context->communicator
            );

            // Send data
            Edge* recv_buff = malloc(buffer_load[0] * sizeof(char));
            int nb_edges = my_size / sizeof(Edge);
            MPI_Scatterv(
                buffer, buffer_load, buffer_disp, MPI_CHAR,
                recv_buff, my_size, MPI_CHAR,
                0, context->communicator
            );

            for (int i = 0 ; i < nb_edges ; i++)
                register_edge(recv_buff[i], context);

            free(recv_buff);

            // Empty buffers
            for (int p = 0 ; p < context->nb_process ; p++)
                buffer_load[p] = 0;
        }
    }

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
        assert(buffer_load <= MAX_COM_SIZE);
        assert(buffer_load % sizeof(Edge) == 0);

        // Receive datas
        MPI_Scatterv(
            NULL, NULL, NULL, MPI_INT,
            buffer, buffer_load, MPI_INT,
            0, context->communicator
        );

        for (uint i = 0 ; i < (buffer_load / sizeof(Edge)) ; i++)
            finished = !register_edge(buffer[i], context);
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
    #define p(x) (context->uf_parent[x])

    if (owner(p(node)) != context->process || p(node) == node) {
        return node;
    }
    else {
        uint32_t root = local_root(p(node), context);
        p(node) = root;
        return root;
    }

    #undef p
}

void filter_border(RemContext* context)
{
    // Compress local structure to get a tree height of one
    for (uint32_t node = 0 ; node < context->nb_vertices ; node++)
        if (owner(node) == context->process)
            local_root(node, context);

    // Copy disjoint set structure in order not to alter it
    uint32_t* uf_copy = malloc(context->nb_vertices * sizeof(uint32_t));

    for (uint i = 0 ; i < context->nb_vertices ; i++)
        uf_copy[i] = context->uf_parent[i];

    // Create a new graph in which we will push edges to keep
    Graph* new_border = new_empty_graph(context->nb_vertices);

    #define p(x) uf_copy[x]
    for (int i = 0 ; i < context->border_graph->nb_edges ; i++) {
        uint r_x = context->border_graph->edges[i].x;
        uint r_y = context->border_graph->edges[i].y;

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
        to_send_displ[p] = p * MAX_COM_SIZE;
        fake_sizes[p] = -1;
    }

    // Processing loop that stops when every process has no more data to send
    while (true) {
        // If this process has nothing to do, send a fake size to everyone
        bool did_nothing = is_empty(todo);

        // Execute tasks from the queue
        #define p(x) (context->uf_parent[(x)])
        #define lroot(x) ((owner(p(x)) == context->process) ? p(x) : x)

        for (int t = 0 ; t < MAX_LOCAL_ITER && !is_empty(todo) ; t++) {
            Edge r = pop_task(&todo);
            r.x = lroot(r.x);

            if (p(r.x) < r.y) {
                int to_send_pos = (to_send_displ[owner(r.y)] + to_send_sizes[owner(r.y)]) / sizeof(Edge);
                to_send[to_send_pos].x = r.y;
                to_send[to_send_pos].y = p(r.x);
                to_send_sizes[owner(r.y)] += sizeof(Edge);
            }
            else if (p(r.x) > r.y) {
                if (p(r.x) == r.x) {
                    p(r.x) = r.y;
                }
                else {
                    int to_send_pos = (to_send_displ[owner(p(r.x))] + to_send_sizes[owner(p(r.x))]) / sizeof(Edge);
                    to_send[to_send_pos].x = p(r.x);
                    to_send[to_send_pos].y = r.y;
                    to_send_sizes[owner(p(r.x))] += sizeof(Edge);

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
                assert(recv_sizes[p] % sizeof(Edge) == 0);
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
        Edge* recv_datas = malloc(total_size);
        MPI_Alltoallv(
            to_send, to_send_sizes, to_send_displ, MPI_CHAR,
            recv_datas, recv_sizes, recv_displ, MPI_CHAR,
            context->communicator
        );

        // Load tasks in the queue
        for (uint t = 0 ; t < total_size / sizeof(Edge) ; t++)
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
    for (uint i = 0 ; i < context->nb_vertices ; i++)
        if (owner(i) == context->process)
            my_size += sizeof(Edge);

    Edge* edges = malloc(my_size * sizeof(Edge));
    int edge_pos = 0;
    for (uint i = 0 ; i < context->nb_vertices ; i++) {
        if (owner(i) == context->process) {
            edges[edge_pos].x = i;
            edges[edge_pos].y = context->uf_parent[i];
            edge_pos++;
        }
    }
    assert(edge_pos * (int) sizeof(Edge) == my_size);

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
    Edge* all_edges = malloc(total_size);
    MPI_Gatherv(
        edges, my_size, MPI_CHAR,
        all_edges, sizes, displ, MPI_CHAR,
        0, context->communicator
    );

    // Display edges
    if (context->process == 0)
        for (uint i = 0 ; i < total_size / sizeof(Edge) ; i++)
            printf("%u %u\n", all_edges[i].x, all_edges[i].y);

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
