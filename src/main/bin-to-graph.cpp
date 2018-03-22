/**
 * Convert a graph from binary format to ascii format.
 * Read from the first specified file an writes to the second.
 * If no file is specified, inputs/outputs to stdin/stdout.
 */
#include <iostream>
#include <fstream>
#include <string>

#include "../communication/messages.hpp"
#include "../graph.hpp"


int main(int argc, char** argv)
{
    std::ios::sync_with_stdio(false);

    // Open files
    bool input_file = argc > 1;

    std::istream* input = &std::cin;
    std::ostream* output = &std::cout;

    std::string in_filename;
    std::string out_filename;

    std::ifstream in_file;
    std::ofstream out_file;

    if (input_file) {
        in_filename = argv[1];
        out_filename = argc > 2 ? argv[2] : in_filename + ".bin";

        in_file.open(in_filename.c_str());
        out_file.open(out_filename.c_str());

        input = &in_file;
        output = &out_file;
    }

    // Transfer datas
    size_t n = bin_readn(*input);
    *output << n << '\n';

    while (true) {
        size_t x = bin_readn(*input);
        size_t y = bin_readn(*input);

        if (input->
            eof())
            break;

        *output << x << ' ' << y << '\n';
    }

    // Close files
    if (input_file) {
        in_file.close();
        out_file.close();
    }
}
