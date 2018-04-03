#include "manytomany.hpp"


ManyToMany::ManyToMany(MPI_Comm communicator) :
    communicator(communicator)
{}

void ManyToMany::send(const std::vector<std::vector<char>>& data)
{
    // Calculate size informations
    std::vector<int> sendcounts;
    std::vector<int> senddispls;

    std::vector<int> recvcounts(data.size());
    std::vector<int> recvdispls;

    int cumul_send_size = 0;
    int cumul_recv_size = 0;

    for (const std::vector<char>& proc_buff: data) {
        senddispls.push_back(cumul_send_size);
        sendcounts.push_back(proc_buff.size());
        cumul_send_size += proc_buff.size();
    }

    // Create sent buffer
    std::vector<char> sendbuf;
    sendbuf.reserve(cumul_send_size);

    for (const std::vector<char>& proc_buff: data)
        sendbuf.insert(sendbuf.end(), proc_buff.begin(), proc_buff.end());

    MPI_Alltoall(
        sendcounts.data(), 1, MPI_INT,
        recvcounts.data(), 1, MPI_INT,
        communicator
    );

    // Gather sizes informations
    for (int proc_size: recvcounts) {
        recvdispls.push_back(cumul_recv_size);
        cumul_recv_size += proc_size;
    }

    // Exchange real data
    cached_buffer.resize(cumul_recv_size, 0);
    MPI_Alltoallv(
        sendbuf.data(), sendcounts.data(), senddispls.data(), MPI_CHAR,
        &cached_buffer[0], recvcounts.data(), recvdispls.data(), MPI_CHAR,
        communicator
    );
}

std::vector<char> ManyToMany::receive_merged()
{
    return std::move(cached_buffer);
}
