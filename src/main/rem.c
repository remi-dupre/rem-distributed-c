#include <stdio.h>

#include "../rem.h"
#include "../tools.h"


// Number of edges to load simultaneously
#define BUFF_SIZE 4096


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

    // Catch graph from file
    Graph* graph = new_empty_graph(nb_vertices);

    // Read file by chunks
    int loaded = 0;
    Edge* buffer = malloc(BUFF_SIZE * sizeof(Edge));

    do {
        insert_edges(graph, buffer, loaded);
        loaded = fread(buffer, sizeof(Edge), BUFF_SIZE, input);
    } while (loaded > 0);

    // Execute raw REM
    long time = time_ms();
    rem_update(graph->edges, graph->nb_edges, uf_parent);
    time = time_ms() - time;

    FILE* logs = fopen("mpitest.log", "a");
    fprintf(logs, ">> Basic algorithm\n");
    fprintf(logs, "%ld ms\n\n", time);

    printf("%u\n", nb_vertices);
    for (Node i = 0 ; i < nb_vertices ; i++) {
        printf("%u %u\n", i, uf_parent[i]);
    }

    // Close input file
    if (argc > 1)
        fclose(input);
}
