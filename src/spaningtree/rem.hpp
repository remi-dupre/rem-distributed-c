/**
 * Implementation of REM's algorithm to calculate spaning forests.
 */
#pragma once

#include <tuple>
#include <vector>

#include "../graph.hpp"


/**
 * Return a spaning tree given a graph.
 * If initial is specified, this will start processing from an initial state.
 * If initial is not constant, this function will modify it in place.
 * The resulting forest only contains new edges to add.
 */
Graph rem_spaning_inplace(const Graph& graph, std::vector<size_t>& initial);
Graph rem_spaning(const Graph& graph, const std::vector<size_t>& initial);
Graph rem_spaning(const Graph& graph);

/**
 * Return components of the graph using disjoint-set datastructure.
 * If initial is specified, this will start processing from an initial state.
 */
std::vector<size_t> rem_components(const Graph& graph, const std::vector<size_t>& initial);
std::vector<size_t> rem_components(const Graph& graph);
