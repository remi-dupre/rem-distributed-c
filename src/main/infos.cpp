/**
 * Give some general informations about a graph.
 */
#include <cstdio>
#include <iostream>

#include "../graph.hpp"
#include "../tools.hpp"


int main() {
    Graph graph;
    std::cin >> graph;

    printf("This graph has %lu vertices and %lu edges\n", graph.nb_vertices, graph.edges.size());

    if (is_tree(graph))
        printf("This graph is a tree.\n");
    else if (is_forest(graph))
        printf("This graph is a forest.\n");
    else
        printf("This graph is not a forest.\n");
}
