#include <mpi.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "../rem_distributed.h"
#include "../tools.h"


int main(int argc, char** argv) {
    // Timing values
    long t_start;
    long t_end_local;
    long t_end_send;
    long t_end_process;

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
    t_start = time_ms();
    RemContext* context = new_context();

    if (process == 0)
        send_graph(input, context);
    else
        recv_graph(context);

    t_end_send = time_ms();

    flush_buffered_graph(context);
    filter_border(context);
    // flatten(context);

    t_end_local = time_ms();

    process_distributed(context);

    t_end_process = time_ms();

    debug_structure(context);
    debug_timers(context);

    // Close input file
    if (argc > 1)
        fclose(input);

    MPI_Finalize();

    // Debug timings
    if (process == 0) {
        struct stat buffer;
        bool csv_head = stat("mpitest.csv", &buffer);

        FILE* logs = fopen("mpitest.log", "a");
        FILE* csv = fopen("mpitest.csv", "a");

        if (csv_head) {
            fprintf(csv, "start time;command;np;ipc;sending;local;distributed\n");
        }

        long time_sending = t_end_send - t_start;
        long time_localp = t_end_local - t_end_send;
        long time_process = t_end_process - t_end_local;

        char time_str[1024];
        time_t t = time(NULL);
        strftime(time_str, 1024, "%c", localtime(&t));

        // Write logs file
        fprintf(logs, ">>");
        for (int i = 0 ; i < argc ; i++)
            fprintf(logs, " %s", argv[i]);
        fprintf(logs, " (%s)\n", time_str);

        fprintf(logs, "Number of process: %d\n", nb_process);
        fprintf(logs, "Number of iterations per cycle: %d\n", MAX_LOCAL_ITER);
        fprintf(logs, "System's sizes: node = %luB, edge = %luB\n", sizeof(Node), sizeof(Edge));
        fprintf(logs, "--\n");
        fprintf(logs, "Time spent sending datas: %ldms\n", time_sending);
        fprintf(logs, "Time spent localy processing: %ldms\n", time_localp);
        fprintf(logs, "Time spent distributedly processing: %ldms\n\n", time_process);

        // Write csv file
        fprintf(csv, "%s;", time_str);
        fprintf(csv, argv[argc-1]);
        fprintf(csv, ";%d;%d", nb_process, MAX_LOCAL_ITER);
        fprintf(csv, ";%ld;%ld;%ld\n", time_sending, time_localp, time_process);

        // Close files
        fclose(csv);
        fclose(logs);
    }
}
