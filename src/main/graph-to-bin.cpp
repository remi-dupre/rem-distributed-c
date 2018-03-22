/**
 * Convert a graph from ascii format to binary format.
 * Read from the first specified file an writes to the second.
 * If no second file is specified, inputs/outputs to stdin/stdout
 */
#include <iostream>
#include <fstream>

#include "../communication/messages.hpp"
#include "../graph.hpp"
#include "../parameters.hpp"


int main(int argc, const char** argv)
{
    std::ios::sync_with_stdio(false);

    bool input_file = argc > 1;

    std::istream* input = &std::cin;
    std::ostream* output = &std::cout;

    std::string in_filename;
    std::string out_filename;

    std::ifstream in_file;
    std::ofstream out_file;

    // Allocate bigger buffers for files
    char in_buffer[file_buff_size];
    char out_buffer[file_buff_size];

    in_file.rdbuf()->pubsetbuf(in_buffer, file_buff_size);
    out_file.rdbuf()->pubsetbuf(out_buffer, file_buff_size);

    // Open files
    if (input_file) {
        in_filename = argv[1];
        out_filename = argc > 2 ? argv[2] : in_filename + ".bin";

        in_file.open(in_filename.c_str(), std::ios::in);
        out_file.open(out_filename.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);

        input = &in_file;
        output = &out_file;
    }

    // Transfer data
    size_t current_int;
    while (*input >> current_int) {
        bin_write(current_int, *output);
    }

    // Close files
    if (input_file) {
        in_file.close();
        out_file.close();
    }
}
