#include <omp.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>


#include "../rem.h"
#include "../task.h"
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

    #pragma omp parallel for
    for (Node i = 0 ; i < nb_vertices ; i++)
        uf_parent[i] = i;

    // Catch graph from file
    Graph graph = *new_empty_graph(nb_vertices);

    // Read file by chunks
    int loaded = 0;
    Edge* buffer = malloc(BUFF_SIZE * sizeof(Edge));

    do {
        insert_edges(&graph, buffer, loaded);
        loaded = fread(buffer, sizeof(Edge), BUFF_SIZE, input);
    } while (loaded > 0);


    time_t timer;

    #pragma omp parallel
    {
        TaskHeap tasks = empty_task_heap();

        #pragma omp for
        for (size_t i = 0 ; i < graph.nb_edges ; i++) {
            push_task(&tasks, graph.edges[i]);
        }

        #pragma omp barrier
        timer = time_ms();

        while (tasks.nb_tasks > 0) {
            TaskHeap new_tasks = empty_task_heap();

            for (size_t i = 0 ; i < tasks.nb_tasks ; i++) {
                Edge edge = tasks.tasks[i];

                #define p(x) (uf_parent[x])
                while (p(edge.x) != p(edge.y)) {
                    if (p(edge.x) > p(edge.y)) {
                        if(edge.x == p(edge.x)) {
                            p(edge.x) = p(edge.y);
                            push_task(&new_tasks, edge);
                        }
                        else {
                            const Node z = p(edge.x);
                            p(edge.x) = p(edge.y);
                            edge.x = z;
                        }
                    }
                    else {
                        if (edge.y == p(edge.y)) {
                            p(edge.y) = p(edge.x);
                            push_task(&new_tasks, edge);
                        }
                        else {
                            const Node z = p(edge.y);
                            p(edge.y) = p(edge.x);
                            edge.y = z;
                        }
                    }
                }
                #undef p
            }

            free(tasks.tasks);
            tasks = new_tasks;

            new_tasks = empty_task_heap();
            for (size_t i = 0 ; i < tasks.nb_tasks ; i++) {
                if (!rem_compare(tasks.tasks[i], uf_parent))
                    push_task(&new_tasks, tasks.tasks[i]);
            }

            free(tasks.tasks);
            tasks = new_tasks;
        }
    }

    timer = time_ms() - timer;


    // Output to log filss
    struct stat stat_buff;
    bool csv_head = stat("rem.csv", &stat_buff);

    char time_str[1024];
    time_t t = time(NULL);
    strftime(time_str, 1024, "%c", localtime(&t));

    FILE* logs = fopen("mpitest.log", "a");
    fprintf(logs, ">> Shared algorithm: %s (%s)\n", argv[1], time_str);
    fprintf(logs, "%ld ms\n\n", timer);
    fclose(logs);

    FILE* csv = fopen("shared.csv", "a");
    if (csv_head) {
        fprintf(csv, "date;input;time\n");
    }
    fprintf(csv, "%s;%s;%ld\n", time_str, argv[1], timer);

    printf("%u\n", nb_vertices);
    for (Node i = 0 ; i < nb_vertices ; i++) {
        printf("%u %u\n", i, uf_parent[i]);
    }

    // Close input file
    if (argc > 1)
        fclose(input);
}