/**
 * Implementation of basic rem algorithm.
 */
#ifndef rem
#define rem

#include "graph.h"


/**
 * Include inline functions
 *   void rem_insert(Edge edge, Node* uf_parent)
 */
#include "rem.inl"

/**
 * Updates a disjoint set structure with a set of new Edges.
 */
void rem_update(const Edge* edges, size_t nb_edges, Node* uf_parent);

/**
 * Returns the root of representing edge in a union find.
 * Do standart compression steps.
 */
Node repr(Node node, Node* uf_parent);

#endif
