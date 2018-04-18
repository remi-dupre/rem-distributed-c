#ifndef GRAPH_H
#define GRAPH_H

#include <assert.h>
#include <memory.h>
#include <mpi.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>


/**
 * Type used to represent a node.
 */
typedef uint32_t Node;

/**
 * Representation of an edge as concatenation of two integers.
 */
typedef struct Edge
{
    Node x;
    Node y;
} Edge;

// Mpi edge's type
MPI_Datatype MPI_EDGE;

/**
 * A graph is a number of vertices, and a list of its edges.
 *  - its list of vertices is the range [0, nb_vertices-1]
 *  - its edges are directly represented in a vector
 */
typedef struct Graph
{
    Node nb_vertices;
    size_t nb_edges;
    size_t container_size; // the maximum number of edges we can store
    Edge* edges;
} Graph;

/**
 * Create a new graph containing no edge.
 */
Graph* new_empty_graph(Node nb_vertices);

/**
 * Delete a graph, free any memory usage.
 */
void delete_graph(Graph* graph);

/**
 * Indicate that a graph will need at least some space.
 */
void reserve(Graph* graph, size_t min_size);

/**
 * Shrink the structure to keep edges from 0 to end_edge-1.
 * Returns false if no space was released.
 */
bool shrink(Graph* graph, size_t end_edge);

/**
 * Insert a new edge to an existing graph.
 */
void insert_edge(Graph* graph, Edge edge);

/**
 * Insert a list of edges to an existing graph.
 */
void insert_edges(Graph* graph, const Edge* edges, size_t nb_edges);

/**
 * Read a graph from an ascii representation:
 * First line contains its number of nodes.
 * Next lines represent edges.
 */
Graph* read_as_ascii(FILE* file);

#endif
