/**
 * Definition of our graph representation.
 */
#pragma once

#include <algorithm>
#include <cassert>
#include <functional>
#include <iostream>
#include <random>
#include <tuple>
#include <vector>

/**
 * An edge is a reference to two vertices, represented by their index.
 */
typedef std::pair<size_t, size_t> Edge;

/**
 * A graph is a number of vertices, and a list of its edges.
 *  - His list of vertices is the range [0, nb_vertices-1]
 *  - His edges are directly represented in a vector
 */
struct Graph {
    size_t nb_vertices;
    std::vector<Edge> edges;

    /**
     * Create a graph containing n vertices and no edges.
     */
    Graph(size_t n = 0);

    /**
     * Return the adjacency list of this graph.
     */
    std::vector<std::vector<size_t>> asAdjacencyList() const;
};

/**
 * Returns a complete graph of n vertices.
 */
Graph complete_graph(size_t n);

/**
 * Return a random graph of n vertices and m edges.
 */
Graph random_graph(size_t n, size_t m);

/**
 * Read a graph from a stream.
 * The graph is formated in following form:
 *   | nb_vertices nb_edges
 *   | x1 y1
 *   | x2 y2
 *   | ...
 */
std::istream& operator>>(std::istream& input, Graph& graph);

/**
 * Write a graph in a stream.
 * The graph is formated in following form:
 *   | nb_vertices nb_edges
 *   | x1 y1
 *   | x2 y2
 *   | ...
 */
std::ostream& operator<<(std::ostream& output, const Graph& graph);
