/**
 * Body of inlines functions for graph.h
 */

/**
 * Insert a new edge to an existing graph.
 */
static inline void insert_edge(Graph* graph, Edge edge)
{
    assert(edge.x < graph->nb_vertices);
    assert(edge.y < graph->nb_vertices);

    // allocate more space
    reserve(graph, graph->nb_edges + 1);
    graph->edges[graph->nb_edges] = edge;
    graph->nb_edges++;
}
