/**
 * Convert a graph from its simple ascii representation to a binary representation.
 */
#include <stdio.h>

#include "../graph.h"


int main(int argc, char** argv)
{
    FILE* input = stdin;
    FILE* output = stdout;

    // Open files for input and output
    if (argc > 2) {
        input = fopen(argv[1], "rb");
        output = fopen(argv[2], "w");
    }

    // Read entirely the file
    uint32_t nb_vertices, nb_edges;
    fread(&nb_vertices, sizeof(uint32_t), 1, input);
    fread(&nb_edges, sizeof(uint32_t), 1, input);

    Edge* edges = malloc(nb_edges * sizeof(Edge));
    fread(edges, sizeof(Edge), nb_edges, input);

    // Output the entire graph
    fprintf(output, "%u %u\n", nb_vertices, nb_edges);

    for (uint i = 0 ; i < nb_edges ; i++)
        fprintf(output, "%u %u\n", edges[i].x, edges[i].y);

    // Close opened files
    if (argc > 2) {
        fclose(input);
        fclose(output);
    }
}
