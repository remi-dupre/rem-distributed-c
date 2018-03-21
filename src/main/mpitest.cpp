#include <mpi.h>

#include <fstream>
#include <iostream>

#include "../spaningtree/rem_distributed.hpp"


int main(int argc, const char** argv) {
    MPI_Init(nullptr, nullptr);

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

    rem_engine.loadGraph();

    std::cout << "---------- " << process << " ----------" << std::endl;
    rem_engine.debug();

    MPI_Finalize();
}
