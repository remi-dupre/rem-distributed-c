/**
 * Generate a graph and writes it on standart output.
 * Available commands are:
 *  - `generate complete n`: generate a complete graph of size n
 *  - `generate random n m`: generate a graph with n vertices and m edges
 */
#include <iostream>
#include <string>

#include "../graph.hpp"


int main(int argc, const char* argv[]) {
    if (argc < 2)
        std::cerr << "Usage: `generate type options`" << std::endl;
    if (argc >= 2) {
        std::string type(argv[1]);

        if (type == "complete" && argc > 2)
            std::cout << complete_graph(atoi(argv[2]));
        if (type == "random" && argc > 3)
            std::cout << random_graph(atoi(argv[2]), atoi(argv[3]));
    }
}
