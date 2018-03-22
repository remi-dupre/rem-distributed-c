/**
 * Convert a graph from ascii format to binary format.
 * Read from the first specified file an writes to the second.
 *  - if no second file is specified, outputs to stdout
 *  - if no first file is specified, outputs to stdin
 */
#include <iostream>
#include <fstream>

#include "../communication/messages.hpp"
#include "../graph.hpp"


int main(int argc, const char** argv)
{
    bool input_file = argc > 1;
    bool output_file = argc > 2;

    std::istream* input = &std::cin;
    std::ostream* output = &std::cout;

    std::ifstream in_file;
    std::ofstream out_file;

    if (input_file) {
        in_file.open(argv[1]);
        input = &in_file;
    }

    if (output_file) {
        out_file.open(argv[2]);
        output = &out_file;
    }

    Graph graph;
    *input >> graph;
    bin_write(graph, *output);

    if (input_file)
        in_file.close();

    if (output_file)
        out_file.close();
}
