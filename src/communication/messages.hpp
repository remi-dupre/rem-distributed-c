/**
 * Definition and behaviour of messages moving accross processes.
 */
#pragma once

#include <iostream>

#include "../graph.hpp"




/**
 * Represent a task of the algorithm.
 * This task contains an edge (r_x, r_y) that we need to put in same component.
 * It can also contain p[r_y], if it is not known, we should make it be r_y
 */
struct Task
{
    size_t r_x, r_y, p_r_y;

    Task() = default;

    Task(size_t x, size_t y) :
        r_x(x), r_y(y), p_r_y(y)
    {}

    Task(size_t x, size_t y, size_t p_y) :
        r_x(x), r_y(y), p_r_y(p_y)
    {}
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
GraphPrintBin bin_format(Graph& graph);

/**
 * Read/Write a graph to/from a string as binary.
 */
std::istream& operator>>(std::istream& input, GraphPrintBin graphb);
std::ostream& operator<<(std::ostream& output, const GraphPrintBin& graphb);
