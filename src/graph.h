#ifndef GRAPH_H
#define GRAPH_H

#include <assert.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>


/**
 * Representation of an edge as concatenation of two integers.
 */
typedef struct Edge
{
    uint32_t x;
    uint32_t y;
} Edge;

static_assert(sizeof(Edge) == 2*sizeof(uint32_t), "Edges shouldn't be padded");

/**
 * A graph is a number of vertices, and a list of its edges.
 *  - its list of vertices is the range [0, nb_vertices-1]
 *  - its edges are directly represented in a vector
 */
typedef struct Graph
{
    int nb_vertices;
    int nb_edges;
    int container_size; // the maximum number of edges we can store
    Edge* edges;
} Graph;

/**
 * Create a new graph containing no edge.
 */
Graph* new_empty_graph(int nb_vertices);

/**
 * Insert a new edge to an existing graph.
 */
void insert_edge(Graph* graph, uint32_t x, uint32_t y);

/**
 * Read a graph from an ascii representation:
 * First line contains its number of nodes.
 * Next lines represent edges.
 */
Graph* read_as_ascii(FILE* file);

#endif
