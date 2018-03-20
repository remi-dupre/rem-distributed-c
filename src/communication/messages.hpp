/**
 * Definition and behaviour of messages moving accross processes.
 */
#pragma once

#include <iostream>

#include "../graph.hpp"


/**
 * Indicate that a task should be run from this process.
 */
struct Task
{
    /**
     * Quick constructor for a task.
     */
    Task(size_t r_x, size_t r_y, size_t x, size_t y, size_t p_r_y);

    Edge original;  // The original edge (x, y) to insert in the spaning forest
    Edge current;   // The current (r_x, r_y) edge to check
    size_t p_r_y;   // If r_y is not owned by this process, contains p[r_y]
};

/**
 * Read/Write a task from/to a stream.
 * The task will be written in non human readable binary.
 */
std::istream& operator>>(std::istream& input, Task& task);
std::ostream& operator<<(std::ostream& output, const Task& task);

/**
 * Manipulator struct for a graph we want to ouput in binary format.
 */
struct GraphPrintBin
{
    Graph& ref;

    GraphPrintBin(Graph& graph) :
        ref(graph)
    {}
};

/**
 * Manipulator function, a graph called inside this function will be send and received to strings in a binary format.
 * Example: std::cout >> bin_format(graph);
 */
GraphPrintBin bin_format(Graph& graph) { return GraphPrintBin(graph); }

/**
 * Read/Write a graph to/from a string as binary.
 */
std::istream& operator>>(std::istream& input, GraphPrintBin& graphb);
std::ostream& operator<<(std::ostream& output, const GraphPrintBin& graphb);
