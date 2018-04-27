#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "../graph.h"


int main(int argc, char** argv)
{
    // Seed random with microsecond accuracy
    struct timeval t1;
    gettimeofday(&t1, NULL);
    srand(t1.tv_usec * t1.tv_sec);

    if (argc < 2) {
        perror("Excepting generation parameters.");
        return 1;
    }

    if (strcmp(argv[1], "complete") == 0) {
        if (argc < 3) {
            perror("Number of vertices is excepted.");
            return 1;
        }

        const Node nb_vertices = atoi(argv[2]);
        printf("%u\n", nb_vertices);

        for (Node x = 0 ; x < nb_vertices ; x++)
            for (Node y = 0 ; y < x ; y++)
                printf("%u %u\n", x, y);
    }

    if (strcmp(argv[1], "random") == 0) {
        if (argc < 4) {
            perror("Nomber of vertices and edges are excepted.");
            return 1;
        }

        const Node nb_vertices = atoi(argv[2]);
        const Node nb_edges = atoi(argv[3]);
        printf("%u\n", nb_vertices);

        for (Node i = 0 ; i < nb_edges ; i++) {
            const Node x = rand() % nb_vertices;
            const Node y = rand() % nb_vertices;
            printf("%u %u\n", x, y);
        }
    }

    if (strcmp(argv[1], "density") == 0) {
        if (argc < 4) {
            perror("Nomber of vertices and density are excepted.");
            return 1;
        }

        const Node nb_vertices = atoi(argv[2]);
        const float density = atof(argv[3]);

        for (Node x = 0 ; x < nb_vertices ; x++)
            for (Node y = 0 ; y < nb_vertices ; y++)
                if (rand() < density * RAND_MAX)
                    printf("%u %u\n", x, y);
    }
}
