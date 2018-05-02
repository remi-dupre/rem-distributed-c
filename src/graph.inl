/**
 * Body of inlines functions for graph.h
 */


static inline void reserve(Graph* graph, size_t min_size)
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
