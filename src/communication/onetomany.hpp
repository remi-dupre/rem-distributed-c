/**
 * Definition of a class handling one to many communication with MPI.
 *
 * More documentation about MPI_Scatterv: https://www.open-mpi.org/doc/v1.4/man3/MPI_Scatterv.3.php
 */
#include <mpi.h>

#include <cassert>
#include <string>
#include <vector>


class OneToMany
{
public:
    /**
     * Default constructor.
     * The source will be the sending process.
     */
    OneToMany(int source, MPI_Comm communicator);

    /**
     * Send datas to every process.
     * The datas are arranged in a vector, indexed by receiver's id.
     * All process must synchronize by calling "receive".
     */
    void send(const std::vector<std::string>& data);

    /**
     * Receive data for this process as a string.
     * Each process can only call this once.
     */
    std::string receive();

private:
    // process sending datas
    int source;

    // MPI communicator carrying datas
    MPI_Comm communicator;

    // Buffer so that sending process can call `receive`
    std::string source_buffer;
};
