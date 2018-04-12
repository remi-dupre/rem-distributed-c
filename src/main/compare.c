#include <stdio.h>

#include "../graph.h"
#include "../rem.h"


// Number of edges we load at once
#define LOAD_SIZE 1024

int main(int argc, char** argv) {
    if (argc < 3) {
        perror("Please provide the name of two files.");
        return 404;
    }

    FILE* file1 = fopen(argv[1], "r");
    FILE* file2 = fopen(argv[2], "r");

    // Skip comments from files
    char c;

    while ((c = fgetc(file1)) == '%')
        while (fgetc(file1) != '\n');
    ungetc(c, file1);

    while ((c = fgetc(file2)) == '%')
        while (fgetc(file2) != '\n');
    ungetc(c, file2);

    uint32_t nb_vertices1;
    uint32_t nb_vertices2;

    fscanf(file1, "%u", &nb_vertices1);
    fscanf(file2, "%u", &nb_vertices2);

    if (nb_vertices1 != nb_vertices2) {
        printf("The two graph have separate components (graphs have different number of vertices).\n");
        fclose(file1);
        fclose(file2);
        return 1;
    }

    // Init disjoint set structures
    uint32_t* uf1 = malloc(nb_vertices1 * sizeof(uint32_t));
    uint32_t* uf2 = malloc(nb_vertices2 * sizeof(uint32_t));

    for (unsigned i = 0 ; i < nb_vertices1 ; i++)
        uf1[i] = uf2[i] = i;

    // Load edges from files
    Edge edge;

    while (fscanf(file1, "%u %u", &edge.x, &edge.y) != EOF)
        rem_update(&edge, 1, uf1);

    while (fscanf(file2, "%u %u", &edge.x, &edge.y) != EOF)
        rem_update(&edge, 1, uf2);

    fclose(file1);
    fclose(file2);

    // Check if graphs are the same
    for (uint32_t i = 0 ; i < nb_vertices1 ; i++) {
        if (repr(i, uf1) != repr(i, uf2)) {
            printf("The two graph have separate components (%u is represented by %u in the first one and %u in the second one).\n", i, repr(i, uf1), repr(i, uf2));
            return 1;
        }
    }

    printf("The two graphs have the same components.");
    return 0;
}
