#include <mpi.h>
#include <stdio.h>

#include "../rem_distributed.h"


int main() {
    MPI_Init(NULL, NULL);

    // Get general MPI informations
    int process, nb_process;
    MPI_Comm_rank(MPI_COMM_WORLD, &process);
    MPI_Comm_size(MPI_COMM_WORLD, &nb_process);

    // Start REM algorithm
    RemContext* context = new_context();

    if (process == 0)
        send_graph(stdin, context);
    else
        recv_graph(context);

    debug_context(context);

    MPI_Finalize();
}
