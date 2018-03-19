/**
 * Program that outputs a spaning forest given any graph.
 */
#include <iostream>

#include "../graph.hpp"
#include "../spaningtree/rem.hpp"


int main() {
    Graph graph;
    std::cin >> graph;
    std::cout << rem_spaning(graph);
}
