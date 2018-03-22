#include <chrono>
#include <fstream>
#include <iostream>
#include <mpi.h>

#include "../spanningtree/rem_distributed.hpp"

using namespace std::chrono;


int main(int argc, const char** argv) {
    std::ios::sync_with_stdio(false);

    MPI_Init(nullptr, nullptr);

    int time_start = duration_cast< milliseconds >(
        system_clock::now().time_since_epoch()
    ).count();

    // Get general MPI informations
    int process, nb_process;
    MPI_Comm_rank(MPI_COMM_WORLD, &process);
    MPI_Comm_size(MPI_COMM_WORLD, &nb_process);

    RemDistributed rem_engine(0, MPI_COMM_WORLD);

    if (process == 0) {
        if (argc < 2) {
            std::cerr << "Please enter a file name" << std::endl;
            return 1;
        }

        std::ifstream file;
        file.open(argv[1], std::ios::in);
        rem_engine.sendGraph(file);
        file.close();
    }

    int time_read = duration_cast< milliseconds >(
        system_clock::now().time_since_epoch()
    ).count();

    rem_engine.loadGraph();

    int time_load = duration_cast< milliseconds >(
        system_clock::now().time_since_epoch()
    ).count();

    rem_engine.initTasks();

    do {
        rem_engine.dequeueTasks();
    } while (rem_engine.spreadTasks());
    std::cout << "---------- " << process << " ----------" << std::endl;
    rem_engine.debug();

    int time_process = duration_cast< milliseconds >(
        system_clock::now().time_since_epoch()
    ).count();

    std::cout << std::endl;
    std::cout << "Time to read file: " << time_read - time_start << "ms" << std::endl;
    std::cout << "Time to load graph: " << time_load - time_read << "ms" << std::endl;
    std::cout << "Time processing: " << time_process - time_load << "ms" << std::endl;

    MPI_Finalize();
}
