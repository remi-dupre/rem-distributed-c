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
    uint32_t r_x, r_y, p_r_y;

    Task() = default;

    Task(size_t x, size_t y) :
        r_x(x), r_y(y), p_r_y(y)
    {}

    Task(size_t x, size_t y, size_t p_y) :
        r_x(x), r_y(y), p_r_y(p_y)
    {}

    /**
     * Encode a task to its buffer format.
     */
    std::vector<char> encode() const;
};

/**
 * Read/Write a task from/to a stream.
 * The task will be written in non human readable binary.
 */
std::istream& operator>>(std::istream& input, Task& task);
std::ostream& operator<<(std::ostream& output, const Task& task);

/**
 * Write a graph/edge/node to a stream as a binary format.
 */
void bin_write(const Graph& graph, std::ostream& output);
void bin_write(const Edge& edge, std::ostream& output);
void bin_write(size_t node, std::ostream& output);

/**
 * Read a graph/edge/node from a stream as a binary format.
 */
Graph bin_readg(std::istream& input);
Edge bin_reade(std::istream& input);
size_t bin_readn(std::istream& input);
