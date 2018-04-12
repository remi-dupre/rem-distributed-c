/**
 * Implementation of basic rem algorithm.
 */
#ifndef rem
#define rem

#include "graph.h"


/**
 * Updates a disjoint set structure with a set of new Edges.
 */
void rem_update(const Edge* edges, int nb_edges, uint32_t* uf_parent);

#endif
