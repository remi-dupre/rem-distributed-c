/**
 * Definition of a class handling many to many communication in cpp.
 *
 * More documentation aMPI_Alltoallv:
 *   https://www.open-mpi.org/doc/current/man3/MPI_Alltoallv.3.php
 */
#pragma once

#include <mpi.h>

#include <sstream>
#include <vector>


class ManyToMany
{
public:
    /**
     * Create a many to many channel over a communicator.
     */
    ManyToMany(MPI_Comm communicator);

    /**
     * Send datas to every process.
     * The datas are arranged in a vector, indexed by receiver's id.
     * All process must synchronize by this method.
     */
    void send(const std::vector<std::vector<char>>& data);

    /**
     * Get cached datas from last communication, concatenates all incoming datas.
     * This method should only be called once.
     */
    std::vector<char> receive_merged();
private:
    // MPI communicator carrying datas
    MPI_Comm communicator;

    // Buffer to return in next receive
    std::vector<char> cached_buffer;
};
