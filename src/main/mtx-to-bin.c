/**
 * Convert a graph from a binary representation to its simple ascii representation.
 */
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../graph.h"


#define BUFFER_SIZE 8192


int main(int argc, char** argv)
{
    char* line = malloc(BUFFER_SIZE);

    FILE* input = stdin;
    FILE* output = stdout;

    int index_from_zero = strncmp(argv[argc - 1], "indexfromzero", 13) == 0;

    // Open files for input and output
    if (argc > 1) {
        input = fopen(argv[1], "rb");

        if (argc == 2 || strcmp(argv[2], "indexfromzero") == 0)
            output = fopen(strcat(argv[1], ".bin"), "w");
        else
            output = fopen(argv[2], "w");
    }

    // Ignore comment lines
    char c;

    while ((c = getc(input)) == '%')
        while (getc(input) != '\n');

    ungetc(c, input);

    // Graph's metadata line
    Node nb_vertices;
    fgets(line, BUFFER_SIZE, input);
    sscanf(line, "%lu", &nb_vertices);
    fwrite(&nb_vertices, sizeof(Node), 1, output);

    // Read all edges
    Edge* edges = malloc(BUFFER_SIZE);

    int i = 0;


    while (!feof(input)) {
        // Consume the whole line
        fgets(line, BUFFER_SIZE, input);

        // Catch the edge
        int rv = sscanf(line, "%lu %lu", &edges[i].x, &edges[i].y);

        if (!index_from_zero) {
            edges[i].x--;
            edges[i].y--;
        }

        if (rv != 2)
            continue;

        assert(edges[i].x < nb_vertices);
        assert(edges[i].y < nb_vertices);

        i++;

        if ((i+1) * sizeof(Edge) > BUFFER_SIZE) {
            fwrite(edges, sizeof(Edge), i, output);
            i = 0;
        }

        // Consume whitespaces
        fscanf(input, " ");
    }
    fwrite(edges, sizeof(Edge), i, output);

    // Close opened files
    if (argc > 2) {
        fclose(input);
        fclose(output);
    }
}
