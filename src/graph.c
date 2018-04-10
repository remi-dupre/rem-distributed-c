#include "graph.h"


Graph* new_empty_graph(int nb_vertices)
{
    Graph* graph = malloc(sizeof(Graph));
    graph->nb_vertices = nb_vertices;
    graph->nb_edges = graph->container_size = 0;
    graph->edges = malloc(0);
    return graph;
}

void delete_graph(Graph* graph)
{
    free(graph->edges);
    free(graph);
}

void reserve(Graph* graph, int min_size)
{
    if (min_size > graph->container_size) {
        while (min_size > graph->container_size)
            graph->container_size = 2 * graph->container_size + 1;

        // create new container
        Edge* new_container = malloc(graph->container_size * sizeof(Edge));
        memcpy(new_container, graph->edges, graph->nb_edges * sizeof(Edge));

        // swap containers
        free(graph->edges);
        graph->edges = new_container;
    }
}

void insert_edge(Graph* graph, Edge edge)
{
    assert((int) edge.x < graph->nb_vertices);
    assert((int) edge.y < graph->nb_vertices);

    // allocate more space
    reserve(graph, graph->nb_edges + 1);
    graph->edges[graph->nb_edges] = edge;
    graph->nb_edges++;
}

void insert_edges(Graph* graph, const Edge* edges, int nb_edges)
{
    // allocate more space
    reserve(graph, graph->nb_edges + nb_edges);

    memcpy(&graph->edges[graph->nb_edges], edges, nb_edges);
    graph->nb_edges += nb_edges;
}

Graph* read_as_ascii(FILE* file)
{
    int n;
    Edge edge;

    fscanf(file, "%d\n", &n);
    assert(n >= 0);

    Graph* graph = new_empty_graph(n);

    while(fscanf(file, "%u %u\n", &edge.x, &edge.y) == 1)
        insert_edge(graph, edge);

    return graph;
}
