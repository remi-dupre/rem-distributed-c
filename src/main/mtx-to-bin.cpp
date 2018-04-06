/**
 * Convert a graph from mtx format to binary format.
 * Read from the first specified file and writes to the second.
 * If there is no second file specified, it reads from stdin and writes to stdout.
 *
 * This program is handling "Coordinate Text File"
 * MTX format docmentation: https://math.nist.gov/MatrixMarket/formats.html
 */
#include <cassert>
#include <iostream>
#include <fstream>
#include <string>

#include "../buffer/streambuff.hpp"


int main(int argc, char** argv)
{
    std::istream* input = &std::cin;
    std::ostream* output = &std::cout;

    std::string in_filename;
    std::string out_filename;

    std::ifstream in_file;
    std::ofstream out_file;

    // Open files
    if (argc > 1) {
        in_filename = argv[1];
        out_filename = argc > 2 ? argv[2] : in_filename + ".bin";

        in_file.open(in_filename.c_str(), std::ios::in);
        out_file.open(out_filename.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);

        input = &in_file;
        output = &out_file;
    }

    OStreamBuff obuff(*output);

    std::string comment;
    int n, m, nz;
    uint32_t i, j;
    double val;

    while (input->peek() == '%')
        std::getline(*input, comment);

    *input >> n >> m >> nz;
    assert(n == m);

    obuff.write(reinterpret_cast<char*>(&n), sizeof(uint32_t));

    for (int k = 0 ; k < nz ; k++) {
        *input >> i >> j >> val;

        i--;
        j--;

        obuff.write(reinterpret_cast<char*>(&i), sizeof(uint32_t));
        obuff.write(reinterpret_cast<char*>(&j), sizeof(uint32_t));

        std::string in_filename;
        std::string out_filename;

        std::ifstream in_file;
        std::ofstream out_file;
    }

    // Close files
    if (argc > 1) {
        in_file.close();
        out_file.close();
    }
}
