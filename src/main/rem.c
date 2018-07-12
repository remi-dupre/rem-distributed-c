#include <stdbool.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>


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

    if (uf_parent == NULL) {
        perror("Not enough memory available");
        exit(-1);
    }

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
    time_t timer = time_ms();
    rem_update(graph->edges, graph->nb_edges, uf_parent);
    timer = time_ms() - timer;

    // Output to log filss
    struct stat stat_buff;
    bool csv_head = stat("rem.csv", &stat_buff);

    char time_str[1024];
    time_t t = time(NULL);
    strftime(time_str, 1024, "%c", localtime(&t));

    FILE* logs = fopen("mpitest.log", "a");
    fprintf(logs, ">> Basic algorithm: %s (%s)\n", argv[1], time_str);
    fprintf(logs, "%ld ms\n\n", timer);
    fclose(logs);

    FILE* csv = fopen("rem.csv", "a");
    if (csv_head) {
        fprintf(csv, "date;input;time\n");
    }
    fprintf(csv, "%s;%s;%ld\n", time_str, argv[1], timer);

    printf("%lu\n", nb_vertices);
    for (Node i = 0 ; i < nb_vertices ; i++) {
        printf("%lu %lu\n", i, uf_parent[i]);
    }

    // Close input file
    if (argc > 1)
        fclose(input);
}
