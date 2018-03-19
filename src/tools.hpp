/**
 * Some tools to manipulate graphs.
 */
#pragma once

#include <cassert>
#include <functional>
#include <vector>

#include "spaningtree/rem.hpp"
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
