#include "manytoone.hpp"


ManyToOne::ManyToOne(int source, MPI_Comm communicator):
    source(source),
    communicator(communicator)
{}

void ManyToOne::send(const std::vector<char>& data)
{
    int com_size, process;
    MPI_Comm_size(communicator, &com_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &process);

    std::vector<int> data_sizes(com_size);
    int my_size = data.size();
    MPI_Gather(
        &my_size, 1, MPI_INT,
        data_sizes.data(), 1, MPI_INT,
        source, communicator
    );

    int total_size = 0;
    std::vector<int> displs;

    for (int process_size: data_sizes) {
        displs.push_back(total_size);
        total_size += process_size;
    }

    if (process == source)
        source_buffer.resize(total_size);

    MPI_Gatherv(
        const_cast<char*>(data.data()), data.size(), MPI_CHAR,
        source_buffer.data(), data_sizes.data(), displs.data(), MPI_CHAR,
        source, communicator
    );
}

std::vector<char> ManyToOne::receive_merged()
{
    return std::move(source_buffer);
}
