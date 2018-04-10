#ifndef rem_distributed
#define rem_distributed

#include <assert.h>
#include <stdbool.h>
#include <mpi.h>
#include <stdio.h>

#include "graph.h"


// Maximum size of sent buffers
#define MAX_COM_SIZE 8192

// Function giving the owner of a process
#define owner(node) ((node) % context->nb_process)


typedef struct RemContext
{
    MPI_Comm communicator;
    int process;
    int nb_process;

    // Structure of the owned nodes
    int nb_vertices;
    uint32_t* uf_parent;
    Graph* border_graph;  // graph containing border edges, if not flushed
} RemContext;

/**
 * Create an initial context for the execution.
 */
RemContext* new_context();

/**
 * Change the number of vertices of the full graph.
 */
void change_nb_vertices(RemContext* context, int nb_vertices);

/**
 * Spread graph among process.
 */
void send_graph(FILE* file, RemContext* context);

/**
 * Receive a graph part sent by main process.
 */
void recv_graph(RemContext* context);

/**
 * Register an edge in the context.
 * If this edge indicates that we won't receive anymore edges, return false.
 */
bool register_edge(Edge edge, RemContext* context);

/**
 * Display some general informations about the context.
 */
void debug_context(const RemContext* context);

#endif
