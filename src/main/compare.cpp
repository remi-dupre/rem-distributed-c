/**
 * Program that compares two graphs taken from standart input.
 */
#include <iostream>

#include "../graph.hpp"
#include "../tools.hpp"


int main() {
    std::ios::sync_with_stdio(false);

    Graph graph1, graph2;
    std::cin >> graph1 >> graph2;

    if (same_components(graph1, graph2)) {
        std::cout << "The two graphs have the same components" << std::endl;
        return 0;
    }
    else {
        std::cout << "The two graphs have different components" << std::endl;
        return 1;
    }
}
