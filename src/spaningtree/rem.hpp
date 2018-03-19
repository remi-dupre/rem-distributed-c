/**
 * Implementation of REM's algorithm to calculate spaning forests.
 */
#pragma once

#include <tuple>
#include <vector>

#include "../graph.hpp"


/**
 * Run REM algorithm on a graph.
 * Return resulting forest and disjoint-set structure.
 */
std::pair<Graph, std::vector<size_t>> rem(const Graph& graph);

/**
 * Return components of the graph using disjoint-set datastructure.
 */
std::vector<size_t> rem_components(const Graph& graph);

/**
 * Return a spaning tree given a graph.
 */
Graph rem_spaning(const Graph& graph);
