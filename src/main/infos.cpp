/**
 * Give some general informations about a graph.
 */
#include <cstdio>
#include <fstream>
#include <iostream>

#include "../graph.hpp"
#include "../tools.hpp"


int main(int argc, char** argv) {
    std::ios::sync_with_stdio(false);

    if (argc < 2) {
        std::cerr << "Please specify a file name." << std::endl;
        return 1;
    }

    Graph graph;

    std::ifstream file;
    file.open(argv[1]);
    file >> graph;
    file.close();

    printf("This graph has %lu vertices and %lu edges\n", graph.nb_vertices, graph.edges.size());

    if (is_tree(graph))
        printf("This graph is a tree.\n");
    else if (is_forest(graph))
        printf("This graph is a forest.\n");
    else
        printf("This graph is not a forest.\n");
}
