#ifndef rem_distributed
#define rem_distributed

#include <assert.h>
#include <stdbool.h>
#include <mpi.h>
#include <stdio.h>

#include "graph.h"
#include "task.h"


// Size of chunks to load from files
#define FILE_BUFF_SIZE 8192

// Maximum size of sent buffers
#define MAX_COM_SIZE 8192

// Maximum tasks poped before next communcation phase
#define MAX_LOCAL_ITER 8192

// Function giving the owner of a process
#define owner(node) (((int) node) % context->nb_process)

// Function giving the rank of a node, given its index
// Note that 4294967291 = 2^32-5 is the biggest prime under 2^32
#define f(node) (((uint64_t) 4294967291 * (uint64_t) (node)) % context->nb_vertices)


typedef struct RemContext
{
    MPI_Comm communicator;
    int process;
    int nb_process;

    // Structure of the owned nodes
    unsigned nb_vertices;
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
 * Get the upper edge owned by current process from given edge.
 * Do compression on the path.
 */
uint32_t local_root(uint32_t node, RemContext* context);

/**
 * Remove some edges from the border graph in order to only keep a local covering tree.
 */
void filter_border(RemContext* context);

/**
 * Start the distributed processing of data.
 */
void process_distributed(RemContext* context);

/**
 * Display the disjoint set structure of all process.
 */
void debug_structure(const RemContext* context);

/**
 * Display some general informations about the context.
 */
void debug_context(const RemContext* context);

#endif
