/**
 * Definition of a class handling many to one communication with MPI.
 *
 * More documentation about MPI_Gatherv:
 *   https://www.open-mpi.org/doc/current/man3/MPI_Gatherv.3.php
 */
#pragma once

#include <mpi.h>

#include <cassert>
#include <vector>


class ManyToOne
{
public:
    /**
     * Default constructor.
     * The source will be the receiving process.
     */
    ManyToOne(int source, MPI_Comm communicator);

    /**
     * Send datas to source process.
     */
    void send(const std::vector<char>& data);

    /**
     * Receive data for source process as a string.
     * Source process can only call it once.
     */
    std::vector<char> receive_merged();

private:
    // process sending datas
    int source;

    // MPI communicator carrying datas
    MPI_Comm communicator;

    // Buffer so that sending process can call `receive`
    std::vector<char> source_buffer;
};
