/**
 * Program that compares two graphs taken from standart input.
 */
#include <fstream>
#include <iostream>

#include "../graph.hpp"
#include "../tools.hpp"


int main(int argc, char** argv) {
    std::ios::sync_with_stdio(false);

    if (argc < 3) {
        std::cerr << "You should specify two file names" << std::endl;
    }

    std::ifstream file_a, file_b;
    Graph graph1, graph2;

    file_a.open(argv[1]);
    file_a >> graph1;
    file_a.close();

    file_b.open(argv[2]);
    file_b >> graph2;
    file_b.close();

    if (same_components(graph1, graph2)) {
        std::cout << "The two graphs have the same components" << std::endl;
        return 0;
    }
    else {
        std::cout << "The two graphs have different components" << std::endl;
        return 1;
    }
}
