#include <mpi.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "../rem_distributed.h"
#include "../tools.h"


int main(int argc, char** argv) {
    // Initialisation of mpi
    MPI_Init(NULL, NULL);

    // Get general MPI informations
    int process, nb_process;
    MPI_Comm_rank(MPI_COMM_WORLD, &process);
    MPI_Comm_size(MPI_COMM_WORLD, &nb_process);

    // Timing values
    long t_start;
    long t_end_local;
    long t_end_send;
    long t_end_process;

    MPI_Type_contiguous(sizeof(Node), MPI_CHAR, &MPI_NODE);
    MPI_Type_commit(&MPI_NODE);

    MPI_Type_contiguous(sizeof(Edge), MPI_CHAR, &MPI_EDGE);
    MPI_Type_commit(&MPI_EDGE);

    FILE* input = NULL;

    // Open input file
    if (process == 0) {
        input = stdin;

        if (argc > 1) {
            input = fopen(argv[1], "rb");
            if (input == NULL) {
                perror("Error while accessing input file");
                return -1;
            }
        }
    }

    // Start REM algorithm
    t_start = time_ms();
    RemContext* context = new_context();

    if (process == 0)
        send_graph(input, context);
    else
        recv_graph(context);

    t_end_send = time_ms();

    flush_buffered_graph(context);
    // filter_border(context);  // now handled inside flush
    // flatten(context);

    t_end_local = time_ms();

    process_distributed(context);

    t_end_process = time_ms();

    debug_structure(context);

    // Close input file
    if (process == 0 && argc > 1)
        fclose(input);

    // Initialise log files
    #ifdef TIMERS
        if (process == 0) {
            FILE* file = fopen("mpitest.time.csv", "a");
            fprintf(file, "process;time flushing;time inserting;time filtering;prefilter size;postfilter size\n");
            fclose(file);

            file = fopen("mpitest.steps.csv", "a");
            fprintf(file, "process;iteration;processing time;communication time\n");
            fclose(file);
        }

        MPI_Barrier(MPI_COMM_WORLD);
    #endif

    MPI_Finalize();

    // Debug timings
    debug_timers(context);

    if (process == 0) {
        struct stat buffer;
        bool csv_head = stat("mpitest.csv", &buffer);

        FILE* logs = fopen("mpitest.log", "a");
        FILE* csv = fopen("mpitest.csv", "a");

        if (csv_head) {
            fprintf(csv, "start time;command;np;threads;ipc;sending;local;distributed\n");
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
        fprintf(logs, "Number of threads per process: %d\n", NB_THREADS);
        fprintf(logs, "Number of iterations per cycle: %d\n", MAX_LOCAL_ITER);
        fprintf(logs, "System's sizes: node = %luB, edge = %luB\n", sizeof(Node), sizeof(Edge));
        fprintf(logs, "--\n");
        fprintf(logs, "Time spent sending datas: %ldms\n", time_sending);
        fprintf(logs, "Time spent localy processing: %ldms\n", time_localp);
        fprintf(logs, "Time spent distributedly processing: %ldms\n\n", time_process);

        // Write csv file
        fprintf(csv, "%s;", time_str);
        fprintf(csv, argv[argc-1]);
        fprintf(csv, ";%d;%d;%d", nb_process, NB_THREADS, MAX_LOCAL_ITER);
        fprintf(csv, ";%ld;%ld;%ld\n", time_sending, time_localp, time_process);

        // Close files
        fclose(csv);
        fclose(logs);
    }
}
