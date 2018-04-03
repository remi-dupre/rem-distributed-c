#include "onetomany.hpp"


OneToMany::OneToMany(int source, MPI_Comm communicator) :
    source(source),
    communicator(communicator)
{}

void OneToMany::send(const std::vector<std::vector<char>>& data)
{
    int process;
    MPI_Comm_rank(MPI_COMM_WORLD, &process);
    assert(process == source);

    // Calculate size informations
    std::vector<int> displs;
    std::vector<int> sendcount;

    int cumul_size = 0;
    for (const std::vector<char>& proc_buff : data) {
        displs.push_back(cumul_size);
        sendcount.push_back(proc_buff.size());
        cumul_size += proc_buff.size();
    }

    // Prepare sent buffer
    std::vector<char> sendbuf;
    sendbuf.reserve(cumul_size);

    for (const std::vector<char>& proc_buff : data)
        sendbuf.insert(sendbuf.end(), proc_buff.begin(), proc_buff.end());

    // Send size informations
    int my_size; // pretty pointless var
    MPI_Scatter(
        sendcount.data(), 1, MPI_INT,
        &my_size, 1, MPI_INT,
        source, communicator
    );

    // Send datas
    size_t buff_old_size = source_buffer.size();
    source_buffer.resize(buff_old_size + my_size);
    MPI_Scatterv(
        sendbuf.data(), sendcount.data(), displs.data(), MPI_CHAR,
        &source_buffer[buff_old_size], my_size, MPI_CHAR,
        source, communicator
    );
}

std::vector<char> OneToMany::receive()
{
    int process;
    MPI_Comm_rank(MPI_COMM_WORLD, &process);

    if (process == source) {
        return std::move(source_buffer);
    }

    // Get size informations
    int my_size;
    MPI_Scatter(
        nullptr, 0, MPI_INT,
        &my_size, 1, MPI_INT,
        source, communicator
    );

    // Get datas
    std::vector<char> recvbuf;
    recvbuf.resize(my_size);
    MPI_Scatterv(
        nullptr, nullptr, nullptr, MPI_CHAR,
        recvbuf.data(), my_size, MPI_CHAR,
        source, communicator
    );
    return recvbuf;
}
