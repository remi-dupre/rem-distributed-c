/**
 * Convert a graph from its simple ascii representation to a binary representation.
 */
#include <assert.h>
#include <stdio.h>

#include "../graph.h"


#define BUFFER_SIZE 8192


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
    Node nb_vertices;
    fread(&nb_vertices, sizeof(Node), 1, input);
    fprintf(output, "%lu\n", nb_vertices);

    // Output the entire graph
    Edge* edges = malloc(BUFFER_SIZE);
    const int max_nb_edges = BUFFER_SIZE / sizeof(Edge);

    size_t nb_edges;  // number of edges read from last chunk
    while ((nb_edges = fread(edges, sizeof(Edge), max_nb_edges, input)) > 1) {
        for (size_t i = 0 ; i < nb_edges ; i++) {
            fprintf(output, "%lu %lu\n", edges[i].x, edges[i].y);
        }
    }

    // Close opened files
    if (argc > 2) {
        fclose(input);
        fclose(output);
    }
}
