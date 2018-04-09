/**
 * Convert a graph from a binary representation to its simple ascii representation.
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

    // Graph's metadata
    uint32_t nb_vertices, nb_edges;
    fscanf(input, "%u %u", &nb_vertices, &nb_edges);

    fwrite(&nb_vertices, sizeof(uint32_t), 1, output);
    fwrite(&nb_edges, sizeof(uint32_t), 1, output);

    // Read all edges
    Edge* edges = malloc(nb_edges * sizeof(Edge));

    for (uint i = 0 ; i < nb_edges ; i++)
        fscanf(input, "%u %u", &edges[i].x, &edges[i].y);

    // Write edges
    fwrite(edges, sizeof(Edge), nb_edges, output);

    // Close opened files
    if (argc > 2) {
        fclose(input);
        fclose(output);
    }
}
