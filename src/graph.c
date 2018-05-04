#include "graph.h"


Graph* new_empty_graph(Node nb_vertices)
{
    Graph* graph = malloc(sizeof(Graph));
    graph->nb_vertices = nb_vertices;
    graph->nb_edges = 0;
    graph->container_size = 0;
    graph->edges = NULL;
    return graph;
}

void delete_graph(Graph* graph)
{
    if (graph->edges)
        free(graph->edges);

    free(graph);
}

bool shrink(Graph* graph, size_t end_edge)
{
    assert(end_edge <= graph->container_size);

    if (end_edge == graph->container_size)
        return false;

    if (end_edge < graph->nb_edges)
        graph->nb_edges = end_edge;

    graph->edges = realloc(graph->edges, end_edge * sizeof(Edge));
    graph->container_size = end_edge;
    return true;
}

void insert_edges(Graph* graph, const Edge* edges, size_t nb_edges)
{
    // allocate more space
    reserve(graph, graph->nb_edges + nb_edges);

    memcpy(&graph->edges[graph->nb_edges], edges, nb_edges * sizeof(Edge));
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
