#include <mpi.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>

#include "../rem_distributed.h"


int main(int argc, char** argv) {
    // Timing values
    struct timeval t_start;
    struct timeval t_end_send;
    struct timeval t_end_process;

    // Initialisation of mpi
    MPI_Init(NULL, NULL);

    MPI_Type_contiguous(sizeof(Edge), MPI_CHAR, &MPI_EDGE);
    MPI_Type_commit(&MPI_EDGE);

    // Open input file
    FILE* input = stdin;

    if (argc > 1)
        input = fopen(argv[1], "rb");

    // Get general MPI informations
    int process, nb_process;
    MPI_Comm_rank(MPI_COMM_WORLD, &process);
    MPI_Comm_size(MPI_COMM_WORLD, &nb_process);

    // Start REM algorithm
    gettimeofday(&t_start, NULL);
    RemContext* context = new_context();

    if (process == 0)
        send_graph(input, context);
    else
        recv_graph(context);

    gettimeofday(&t_end_send, NULL);
    filter_border(context);
    process_distributed(context);

    gettimeofday(&t_end_process, NULL);
    debug_structure(context);

    // Close input file
    if (argc > 1)
        fclose(input);

    MPI_Finalize();

    // Debug timings
    if (process == 0) {
        FILE* logs = fopen("mpitest.log", "a");

        size_t time_sending_ms = (t_end_send.tv_sec - t_start.tv_sec) * 1000;
        time_sending_ms += (t_end_send.tv_usec - t_start.tv_usec) / 1000;

        size_t time_process_ms = (t_end_process.tv_sec - t_end_send.tv_sec) * 1000;
        time_process_ms += (t_end_process.tv_usec - t_end_send.tv_usec) / 1000;

        fprintf(logs, ">>");
        for (int i = 0 ; i < argc ; i++)
            fprintf(logs, " %s", argv[i]);

        char time_str[1024];
        time_t t = time(NULL);
        strftime(time_str, 1024, "%c", localtime(&t));
        fprintf(logs, " (%s)\n", time_str);

        fprintf(logs, "Number of process: %d\n", nb_process);
        fprintf(logs, "System's sizes: node = %luB, edge = %luB\n", sizeof(Node), sizeof(Edge));
        fprintf(logs, "--\n");
        fprintf(logs, "Time spent sending datas: %ldms\n", time_sending_ms);
        fprintf(logs, "Time spent processing: %ldms\n\n", time_process_ms);

        fclose(logs);
    }
}
