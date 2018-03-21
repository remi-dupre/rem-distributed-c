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

    // Send edges by chunks of this size
    constexpr int trigger_launch = 32000;

    std::vector<std::stringstream> ss(nb_process);

    size_t n;
    input >> n;

    // Permutation of edges that regroups them
    size_t p = nb_process;
    auto f = [n, p] (size_t x) -> size_t {
        return (p * x) % (p * (n / p)) + (p * x) / n;
    };

    // Flush datas
    auto flush = [&ss, this] () -> void {
        std::vector<std::string> data(nb_process);

        for (int i = 0 ; i < nb_process ; i++)
            data[i] = ss[i].str();

        otm_channel.send(data);
    };

    // Send the graph size to everyone
    for (int i = 0 ; i < nb_process ; i++)
        ss[i] << bin_format(n);

    Edge edge;
    while (input >> edge.first >> edge.second) {
        // Transform edge indexes to regroup contiguous edges
        edge = std::make_pair(
            std::min(f(edge.first), f(edge.second)),
            std::max(f(edge.first), f(edge.second))
        );
        ss[owner(edge.first)] << bin_format(edge);

        // Flush datas
        if (ss[owner(edge.first)].tellg() >= trigger_launch || input.eof()) {
            flush();
        }
    }

    if (input.eof())
        for (int i = 0 ; i < nb_process ; i++)
            ss[i] << bin_format(n) << bin_format(n);

    flush();
}

void RemDistributed::loadGraph()
{
    size_t nb_vertices;
    std::stringstream data(otm_channel.receive());
    data >> bin_format(nb_vertices);

    internal_graph = Graph(nb_vertices);
    border_graph = Graph(nb_vertices);

    // Loops until we obtained the whole graph
    while (true) {
        Edge edge;

        while (data >> bin_format(edge))
        {
            if (edge.first == nb_vertices) {
                assert(edge.second == nb_vertices);
                return;
            }

            if (owner(edge.first) == process && owner(edge.second) == process)
                internal_graph.edges.push_back(edge);
            else if(owner(edge.first) == process)
                border_graph.edges.push_back(edge);
            else
                border_graph.edges.emplace_back(edge.second, edge.first);
        }

        data.str(otm_channel.receive());
    }
}

bool RemDistributed::nothingToDo() const
{
    for (const std::queue<Task>& task_list : todo)
        if (!task_list.empty())
            return false;

    return true;
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

void RemDistributed::dequeueTasks(int task_count)
{
    while (task_count != 0 && !todo[process].empty()) {
        task_count--;

        Task task = todo[process].front();
        runTask(task);
        todo[process].pop();
    }
}

bool RemDistributed::spreadTasks()
{
    std::vector<std::string> buffers(nb_process);

    // If there is nothing to do, give the information to everyone by creating
    //   fake tasks
    if (nothingToDo()) {
        std::stringstream ss;
        ss << Task(
            internal_graph.nb_vertices,
            internal_graph.nb_vertices
        );

        for (int i = 0 ; i < nb_process ; i++) {
            buffers[i] = ss.str();
        }
    }
    else {
        for (int i = 0 ; i < nb_process ; i++) {
            if (i == process)
                continue;

            std::stringstream ss;

            while (!todo[i].empty()) {
                ss << todo[i].front();
                todo[i].pop();
            }

            buffers[i] = ss.str();
        }
    }

    mtm_channel.send(buffers);

    Task new_task;
    std::stringstream incoming_data(mtm_channel.receive_merged());
    int nb_fake_tasks = 0;

    while (incoming_data >> new_task) {
        // Check for fake tasks
        if (new_task.r_x == internal_graph.nb_vertices) {
            assert(new_task.r_y == internal_graph.nb_vertices);
            assert(new_task.p_r_y == internal_graph.nb_vertices);

            nb_fake_tasks++;
            continue;
        }

        todo[process].push(new_task);
    }

    return nb_fake_tasks < nb_process;
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

    // std::cout << "Relations:" << std::endl;
    // for (size_t i = process ; i < internal_graph.nb_vertices ; i += nb_process) {
    //     std::cout << " - " << i << " -> " << uf_parent[i] << std::endl;
    // }
}
