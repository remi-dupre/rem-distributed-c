#include <mpi.h>

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include "../communication/messages.hpp"
#include "../graph.hpp"
#include "../tools.hpp"
#include "../communication/onetomany.hpp"
#include "../communication/manytomany.hpp"


int main(int argc, const char** argv) {
    MPI_Init(nullptr, nullptr);

    // Get general MPI informations
    int process, nb_process;
    MPI_Comm_rank(MPI_COMM_WORLD, &process);
    MPI_Comm_size(MPI_COMM_WORLD, &nb_process);

    // A test of many to many communication
    std::vector<std::string> datat = {
        "this is for process 0 from " + std::to_string(process) + " ; ",
        "this is for process 1 from " + std::to_string(process) + " ; ",
        "this is for process 2 from " + std::to_string(process) + " ; ",
        "this is for process 3 from " + std::to_string(process) + " ; ",
    };

    ManyToMany mtm(MPI_COMM_WORLD);
    mtm.send(datat);
    std::cout << process << ": " << mtm.receive_merged() << std::endl;

    // Get graphs to send
    std::vector<std::string> data;

    if (process == 0) {
        if (argc < 2) {
            std::cerr << "Please enter a file name" << std::endl;
            return 1;
        }

        std::ifstream file;
        file.open(argv[1], std::ios::in);

        Graph full_graph;
        file >> full_graph;

        file.close();

        std::vector<Graph> graphs = split(full_graph, nb_process);

        for (int i = 0 ; i < nb_process ; i++) {
            std::stringstream ss;
            ss << bin_format(graphs[i]);
            data.push_back(ss.str());
        }
    }


    // Send datas
    OneToMany channel(0, MPI_COMM_WORLD);
    if (process == 0) {
        channel.send(data);
    }
    auto a = channel.receive();
    std::istringstream input(a);

    Graph my_graph;
    input >> bin_format(my_graph);
    std::cout << process << ": " << my_graph.edges.size() << " edges" << std::endl;

    MPI_Finalize();
}
