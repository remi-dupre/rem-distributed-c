/**
 * Some tools to manipulate graphs.
 */
#pragma once

#include <cassert>
#include <functional>
#include <vector>

#include "spanningtree/rem.hpp"
#include "graph.hpp"


/**
 * Verifies that two graphs have the same components.
 */
bool same_components(const Graph& g1, const Graph& g2);

/**
 * Checks wether an input graph is a forest (undirected and acyclic).
 */
bool is_forest(const Graph& graph);

/**
 * Checks wether an input graph is a tree.
 */
bool is_tree(const Graph& graph);

/**
 * Split a graph into p separate parts.
 * Return a vector of size `nb_parts`, each part containing a graph, its internal
 *   vertices and neighbour vertices.
 */
std::vector<Graph> split(const Graph& graph, size_t nb_parts);

/**
 * Redistribute nodes indexes so that if we pickup indexes spaced by (n / p),
 *   they were previously consecutives.
 * To do so, if we cut `n` indexes in `p` parts, we replace index `x` with
 *      f(x) = (p*x) % (p * (n/p)) + (p*x / n)
 * For example with n = 7 and p = 2, the result would be:
 *   x    | 0 1 2 3 4 5 6
 *   f(x) | 0 2 4 6 1 3 5
 */
void redistribute(Graph& graph, int nb_parts);
