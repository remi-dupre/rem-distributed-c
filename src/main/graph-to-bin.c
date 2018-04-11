/**
 * Convert a graph from a binary representation to its simple ascii representation.
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

    // Graph's metadata
    uint32_t nb_vertices;
    fscanf(input, "%u", &nb_vertices);
    fwrite(&nb_vertices, sizeof(uint32_t), 1, output);

    // Read all edges
    Edge* edges = malloc(BUFFER_SIZE);

    int i = 0;
    while (fscanf(input, "%u %u", &edges[i].x, &edges[i].y) > 0) {
        assert(edges[i].x < nb_vertices);
        assert(edges[i].y < nb_vertices);

        i++;

        if ((i+1) * sizeof(Edge) > BUFFER_SIZE) {
            fwrite(edges, sizeof(Edge), i, output);
            i = 0;
        }
    }
    fwrite(edges, sizeof(Edge), i, output);

    // Close opened files
    if (argc > 2) {
        fclose(input);
        fclose(output);
    }
}
