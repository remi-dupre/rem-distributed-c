#include <mpi.h>
#include <stdio.h>

#include "../rem_distributed.h"


int main(int argc, char** argv) {
    MPI_Init(NULL, NULL);

    // Open input file
    FILE* input = stdin;

    if (argc > 1)
        input = fopen(argv[1], "rb");

    // Get general MPI informations
    int process, nb_process;
    MPI_Comm_rank(MPI_COMM_WORLD, &process);
    MPI_Comm_size(MPI_COMM_WORLD, &nb_process);

    // Start REM algorithm
    RemContext* context = new_context();

    if (process == 0)
        send_graph(input, context);
    else
        recv_graph(context);

    filter_border(context);
    process_distributed(context);

    debug_structure(context);

    // Close input file
    if (argc > 1)
        fclose(input);

    MPI_Finalize();
}
