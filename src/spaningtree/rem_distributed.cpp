#include "rem_distributed.hpp"


RemDistributed::RemDistributed(int source, MPI_Comm communicator) :
    source(source),
    otm_channel(source, communicator),
    mtm_channel(communicator)
{
    // Load MPI's general informations
    MPI_Comm_rank(communicator, &process);
    MPI_Comm_size(communicator, &nb_process);
}

void RemDistributed::sendGraph(std::istream& input)
{
    assert(process == source);

    // Split the graph
    Graph full_graph;
    input >> full_graph;
    redistribute(full_graph, nb_process);

    std::vector<Graph> graphs = split(full_graph, nb_process);

    // Format data
    std::vector<std::string> data;
    for (int i = 0 ; i < nb_process ; i++) {
        std::stringstream ss;
        ss << bin_format(graphs[i]);
        data.push_back(ss.str());
    }

    // Send data
    otm_channel.send(data);
}

void RemDistributed::loadGraph()
{
    Graph full_graph;
    std::stringstream data(otm_channel.receive());
    data >> bin_format(full_graph);

    internal_graph = Graph(full_graph.nb_vertices);
    border_graph = Graph(full_graph.nb_vertices);

    for (const Edge& edge: full_graph.edges)
    {
        if (owner(edge.first) == process && owner(edge.second) == process)
            internal_graph.edges.push_back(edge);
        else if(owner(edge.first) == process)
            border_graph.edges.push_back(edge);
    }
}

void RemDistributed::initTasks()
{
    // Calculate initial state of the disjoint-set
    relations = rem_components(internal_graph);

    // Get the edges we want to try to send
    Graph to_send = rem_spaning(border_graph, relations);

    for (const Edge& edge: to_send.edges)
        todo.emplace(edge.first, edge.second);
}

int RemDistributed::owner(size_t vertex) const
{
    return (vertex % nb_process);
}

void RemDistributed::debug() const
{
    std::cout << "Internal edges: " << internal_graph.edges.size() << std::endl;
    std::cout << "Border edges: " << border_graph.edges.size() << std::endl;
    std::cout << "Remaining tasks: " << todo.size() << std::endl;
}
