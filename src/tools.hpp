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
 *              (n + 1) p x
 *      f(x) = -------------  %  n
 *                   n
 */
void redistribute(Graph& graph, int nb_parts);
