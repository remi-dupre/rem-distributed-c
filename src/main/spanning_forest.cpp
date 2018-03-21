/**
 * Program that outputs a spanning forest given any graph.
 */
#include <iostream>

#include "../graph.hpp"
#include "../spanningtree/rem.hpp"


int main() {
    Graph graph;
    std::cin >> graph;
    std::cout << rem_spanning(graph);
}
