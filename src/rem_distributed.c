#include "rem_distributed.h"


RemContext* init_context()
{
    RemContext* context = malloc(sizeof(RemContext));

    context->communicator = MPI_COMM_WORLD;
    MPI_Comm_rank(context->communicator, &context->process);
    MPI_Comm_size(context->communicator, &context->nb_process);

    return context;
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
    assert(context->process != 0);

    // Receive graph's total number of nodesntext->nb_vertices = nb_nodes;
    MPI_Bcast(
        &context->nb_vertices, 1, MPI_INT,
        0, context->communicator
    );

    // Receive all edges
    bool finished = false;
    while (!finished) {

    }

    // Read all edges at once
    Edge* edges = malloc(nb_edges * sizeof(Edge));
    fread(edges, sizeof(Edge), nb_edges, file);

    for (uint i = 0 ; i < nb_edges ; i++) {
        int owner = owning_process(context, edges[i].x);

        // Add datas to buffer
        int buff_pos = buffer_disp[owner] + buffer_load[owner];
        memcpy(&buffer[buff_pos], &edges[i], sizeof(Edge));
        buffer_load[owner] += sizeof(Edge);

        // If the last edge is reached, add fake edge to send to everyone
        Edge fake_edge = {
            .x = context->nb_vertices,
            .y = context->nb_vertices
        };
        for (int p = 0 ; p < context->nb_process ; p++) {
            int buff_pos = buffer_disp[p] + buffer_load[p];
            memcpy(&buffer[buff_pos], &fake_edge, sizeof(Edge));
            buffer_load[p] += sizeof(Edge);
        }

        assert(buffer_load[owner] <= MAX_COM_SIZE);
        if (buffer_load[owner] + 2*sizeof(Edge) > MAX_COM_SIZE || i + 1  == nb_edges) {
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
    // CHECK OWNERSHIP
    // IF INSIDE, EXECUTE SAMER
}
