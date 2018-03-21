#include "rem_distributed.hpp"


RemDistributed::RemDistributed(int source, MPI_Comm communicator) :
    source(source),
    otm_channel(source, communicator),
    mtm_channel(communicator)
{
    // Load MPI's general informations
    MPI_Comm_rank(communicator, &process);
    MPI_Comm_size(communicator, &nb_process);

    todo.resize(nb_process);
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
        else
            border_graph.edges.emplace_back(edge.second, edge.first);
    }
}

void RemDistributed::initTasks()
{
    // Calculate initial state of the disjoint-set
    uf_parent = rem_components(internal_graph);

    // Get the edges we want to try to send
    Graph to_send = rem_spanning(border_graph, uf_parent);

    for (const Edge& edge: to_send.edges)
        todo[process].emplace(edge.first, edge.second);
}

void RemDistributed::processTasks(int task_count)
{
    while (task_count != 0 && !todo[process].empty()) {
        task_count--;

        Task task = todo[process].front();
        runTask(task);
        todo[process].pop();
    }
}

void RemDistributed::runTask(Task& task)
{
    std::vector<size_t>& p = uf_parent;

    size_t& r_x = task.r_x;
    size_t& r_y = task.r_y;
    size_t& p_r_y = task.p_r_y;

    // Move to roots and update p[r_y]
    r_x = local_root(r_x);

    if (owner(r_y) == process) {
        r_y = local_root(r_y);
        p_r_y = p[r_y];
    }
    else {
        p[r_y] = p_r_y;
    }

    // Basic cases
    if (p[r_x] == p[r_y]) {
        return;
    }
    if (p[r_x] == r_y) {
        p[r_x] = p[r_y];
        return;
    }
    if (r_x == p[r_y]) {
        p[r_y] = p[r_x];
        return;
    }

    // Try to move on the tree
    if (p[r_x] < p[r_y]) {
        if (r_x == p[r_x]) {
            // AN EDGE CAN BE KEPT
            p[r_x] = p[r_y];
        }
        else {
            size_t z = p[r_x];
            p[r_x] = p[r_y];
            todo[owner(z)].emplace(z, r_y, p[r_y]);
        }
    }
    else {
        todo[owner(p[r_y])].emplace(p[r_y], r_x, p[r_x]);
    }
}

int RemDistributed::owner(size_t vertex) const
{
    return vertex % nb_process;
}

size_t RemDistributed::local_root(size_t vertex)
{
    assert(owner(vertex) == process);

    if (owner(uf_parent[vertex]) != process || uf_parent[vertex] == vertex) {
        return vertex;
    }
    else {
        size_t lroot = local_root(uf_parent[vertex]);
        uf_parent[vertex] = lroot;
        return lroot;
    }
}

void RemDistributed::debug() const
{
    std::cout << "Internal edges: " << internal_graph.edges.size() << std::endl;
    std::cout << "Border edges: " << border_graph.edges.size() << std::endl;
    std::cout << "Remaining tasks: " << todo[process].size() << std::endl;

    size_t to_send_count = 0;

    for (int i = 0 ; i < nb_process ; i++)
        if (i != process)
            to_send_count += todo[i].size();

    std::cout << "Tasks to send: " << to_send_count << std::endl;
}
