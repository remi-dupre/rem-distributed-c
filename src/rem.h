/**
 * Implementation of basic rem algorithm.
 */
#ifndef rem
#define rem

#include <omp.h>

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
 * Updates a disjoint set structure with a set of new Edges, using different threads.
 * For performance purpose, the array "edges" is modified in place.
 * The parameter skip_insert may be set to true when the first insertion step can be skiped.
 */
void rem_shared_update(Edge* edges, size_t nb_edges, Node* uf_parent, int nb_threads, bool skip_insert);

/**
 * Returns the root of representing edge in a union find.
 * Do standart compression steps.
 */
Node repr(Node node, Node* uf_parent);

#endif
