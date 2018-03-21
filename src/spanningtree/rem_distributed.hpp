/**
 * Implementation of distibuted memory rem algorithm.
 */
#pragma once

#include <algorithm>
#include <cassert>
#include <iostream>
#include <mpi.h>
#include <queue>
#include <string>
#include <sstream>
#include <vector>

#include "../communication/onetomany.hpp"
#include "../communication/manytomany.hpp"
#include "../communication/messages.hpp"
#include "../graph.hpp"
#include "../tools.hpp"
#include "rem.hpp"


/**
 * Class processing the distributed algorithm.
 * It will store states the sorting reaches.
 */
class RemDistributed
{
public:
    /**
     * Default constructor. Specify which process owns the initial graph.
     * This class excepts that the communicator will own all ids from 0 to max.
     */
    RemDistributed(int source, MPI_Comm communicator);

    /**
     * Load a graph from an input stream and share it with other processes.
     * The graph must be ascii-formated for debugging purpose.
     */
    void sendGraph(std::istream& input);

    /**
     * Load a graph sent by the main process. Override any loaded graph.
     */
    void loadGraph();

    /**
     * Run the initial local processing:
     *  - create first state of the disjoint set structure owned by this process
     *  - create the initial queue of tasks with local vertices
     */
    void initTasks();

    /**
     * Work on a chunk of tasks that are enqueued.
     * If the `task_count` parameter is negative, process all the queue.
     */
    void processTasks(int task_count = -1);

    /**
     * Run a task, try to decide if r_x and r_y are linked.
     * If the task must continue on another process, add it to todo[other].
     */
    void runTask(Task& task);

    /**
     * Returns process id of the owner of a vertex.
     */
    int owner(size_t vertex) const;

    /**
     * Get the local root of a vertex among owned nodes.
     */
    size_t local_root(size_t vertex);

    /**
     * Display some general informations on stdout.
     */
    void debug() const;

private:
    // current process
    int process;

    // total number of process
    int nb_process;

    // source process that owns the sorting and initial graph
    int source;

    // edges we received
    Graph internal_graph;
    Graph border_graph;

    // channels to exchange with other processes
    OneToMany otm_channel;
    ManyToMany mtm_channel;

    // local links between nodes we own
    // this is represented with a classical disjoint set datastructure
    std::vector<size_t> uf_parent;

    // list of tasks to send to processes indexed by process id
    // the queue corresponding to this process is used here
    std::vector<std::queue<Task>> todo;
};
