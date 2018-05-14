/**
 * Body of inlines functions for graph.h
 */


static inline void reserve(Graph* graph, size_t min_size)
{
    if (min_size > graph->container_size) {
        while (min_size > graph->container_size)
            graph->container_size = 2 * graph->container_size + 1;

        // create new container
        graph->edges = realloc(graph->edges, graph->container_size * sizeof(Edge));
    }
}

/**
 * Insert a new edge to an existing graph.
 */
static inline void insert_edge(Graph* graph, Edge edge)
{
    assert(edge.x < graph->nb_vertices);
    assert(edge.y < graph->nb_vertices);

    // allocate more space
    size_t nb_edges;
    #pragma omp critical
    {
        nb_edges = graph->nb_edges;
        reserve(graph, nb_edges + 1);
        graph->nb_edges++;
    }
    graph->edges[nb_edges] = edge;
}
