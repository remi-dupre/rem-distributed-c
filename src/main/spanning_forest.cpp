/**
 * Program that outputs a spanning forest given any graph.
 */
#include <iostream>

#include "../graph.hpp"
#include "../spanningtree/rem.hpp"


int main() {
    std::ios::sync_with_stdio(false);

    Graph graph;
    std::cin >> graph;
    std::cout << rem_spanning(graph);
}
