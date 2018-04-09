#include "graph.h"


Graph* new_empty_graph(int nb_vertices)
{
    Graph* graph = malloc(sizeof(Graph));
    graph->nb_vertices = nb_vertices;
    graph->nb_edges = graph->container_size = 0;
    graph->edges = malloc(0);
    return graph;
}

void insert_edge(Graph* graph, uint32_t x, uint32_t y)
{
    // allocate more space
    if (graph->nb_edges == graph->container_size) {
        graph->container_size = 2 * graph->container_size + 1;

        // create new container
        Edge* new_container = malloc(graph->container_size * sizeof(Edge));
        memcpy(new_container, graph->edges, graph->nb_edges * sizeof(Edge));

        // swap containers
        free(graph->edges);
        graph->edges = new_container;
    }

    graph->edges[graph->nb_edges].x = x;
    graph->edges[graph->nb_edges].y = y;
    graph->nb_edges++;
}

Graph* read_as_ascii(FILE* file)
{
    int n, x, y;

    fscanf(file, "%d\n", &n);
    assert(n >= 0);

    Graph* graph = new_empty_graph(n);

    while(fscanf(file, "%d %d\n", &x, &y) == 1)
        insert_edge(graph, x, y);

    return graph;
}
