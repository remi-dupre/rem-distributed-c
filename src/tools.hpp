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
