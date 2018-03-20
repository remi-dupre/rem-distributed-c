#include <mpi.h>

#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include "../communication/messages.hpp"
#include "../graph.hpp"
#include "../tools.hpp"
#include "../communication/onetomany.hpp"


int main() {
    MPI_Init(nullptr, nullptr);

    // Get general MPI informations
    int process, nb_process;
    MPI_Comm_rank(MPI_COMM_WORLD, &process);
    MPI_Comm_size(MPI_COMM_WORLD, &nb_process);

    // Get graphs to send
    std::vector<std::string> data;

    if (process == 0) {
        Graph full_graph;
        std::cin >> full_graph;

        std::vector<Graph> graphs = split(full_graph, nb_process);

        for (int i = 0 ; i < nb_process ; i++) {
            std::stringstream ss;
            ss << bin_format(graphs[i]);
            data.push_back(ss.str());
        }
    }

    // Send datas
    OneToMany link(0, MPI_COMM_WORLD);
    if (process == 0) {
        link.send(data);
    }
    std::istringstream input(link.receive());

    Graph my_graph;
    input >> bin_format(my_graph);
    std::cout << my_graph;

    MPI_Finalize();
}
