#include <stdio.h>

#include "../rem.h"


// Number of edges to load simultaneously
#define BUFF_SIZE 1024


int main(int argc, char** argv) {
    // Open input file
    FILE* input = stdin;

    if (argc > 1)
        input = fopen(argv[1], "rb");

    // Prepare union find structure
    Node nb_vertices;
    fread(&nb_vertices, sizeof(Node), 1, input);

    Node* uf_parent = malloc(nb_vertices * sizeof(Node));

    for (Node i = 0 ; i < nb_vertices ; i++)
        uf_parent[i] = i;

    // Read file by chunks
    int loaded = 0;
    Edge* buffer = malloc(BUFF_SIZE * sizeof(Edge));

    do {
        loaded = fread(buffer, sizeof(Edge), BUFF_SIZE, input);
        rem_update(buffer, loaded, uf_parent);
    } while (loaded > 0);

    printf("%u\n", nb_vertices);
    for (Node i = 0 ; i < nb_vertices ; i++) {
        printf("%u %u\n", i, uf_parent[i]);
    }

    // Close input file
    if (argc > 1)
        fclose(input);
}
