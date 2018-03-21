/**
 * Implementation of distibuted memory rem algorithm.
 */
#pragma once

#include <cassert>
#include <iostream>
#include <mpi.h>
#include <string>
#include <sstream>
#include <vector>

#include "../communication/onetomany.hpp"
#include "../communication/manytomany.hpp"
#include "../communication/messages.hpp"
#include "../graph.hpp"
#include "../tools.hpp"


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
     * Load a graph sent by the main process.
     */
    void loadGraph();

    /**
     * Returns process id of the owner of a vertex.
     */
    int owner(size_t vertex) const;

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
};
