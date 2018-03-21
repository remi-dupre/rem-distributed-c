#include "onetomany.hpp"


OneToMany::OneToMany(int source, MPI_Comm communicator) :
    source(source),
    communicator(communicator)
{}

void OneToMany::send(const std::vector<std::string>& data)
{
    int process;
    MPI_Comm_rank(MPI_COMM_WORLD, &process);
    assert(process == source);

    // Calculate size informations
    std::vector<int> displs;
    std::vector<int> sendcount;
    std::string sendbuf;

    int cumul_size = 0;
    for (const std::string& proc_buff : data) {
        displs.push_back(cumul_size);
        sendcount.push_back(proc_buff.size());
        sendbuf += proc_buff;
        cumul_size += proc_buff.size();
    }

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

std::string OneToMany::receive()
{
    int process;
    MPI_Comm_rank(MPI_COMM_WORLD, &process);

    if (process == source) {
        std::string ret = std::move(source_buffer);
        return ret;
    }

    // Get size informations
    int my_size;
    MPI_Scatter(
        nullptr, 0, MPI_INT,
        &my_size, 1, MPI_INT,
        source, communicator
    );

    // Get datas
    std::string recvbuf(my_size, 0);
    MPI_Scatterv(
        nullptr, nullptr, nullptr, MPI_CHAR,
        &recvbuf[0], my_size, MPI_CHAR,
        source, communicator
    );
    return recvbuf;
}
