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
    constexpr int trigger_send = 32000;

    std::vector<std::vector<char>> buffers(nb_process);
    IStreamBuff ibuff(input);

    for (int i = 0 ; i < nb_process ; i++)
        buffers[i].reserve(trigger_send);

    // Read the size of the graph
    uint32_t n;
    ibuff >> n;

    // Permutation of edges that regroups them
    size_t p = nb_process;
    auto f = [n, p] (size_t x) -> size_t {
        return (p * x) % (p * (n / p)) + (p * x) / n;
    };

    // Send the graph size to everyone
    for (int i = 0 ; i < nb_process ; i++)
        buffers[i].insert(
            buffers[i].end(),
            reinterpret_cast<char*>(&n),
            reinterpret_cast<char*>(&n) + sizeof(uint32_t)
        );

    while (!ibuff.eof()) {
        uint32_t x, y;
        ibuff >> x >> y;

        // Transform edge indexes to regroup contiguous edges
        const Edge edge = std::make_pair(
            std::min(f(x), f(y)),
            std::max(f(x), f(y))
        );

        // This node has already been read
        if (ibuff.eof())
            break;

        // Flush datas
        if (buffers[owner(edge.first)].size() + sizeof(Edge) > trigger_send) {
            otm_channel.send(buffers);

            for (int i = 0 ; i < nb_process ; i++)
                buffers[i].clear();
        }

        // Copy the edge at the right place in the memory
        buffers[owner(edge.first)].resize(buffers[owner(edge.first)].size() + sizeof(Edge));
        *reinterpret_cast<Edge*>(&(*buffers[owner(edge.first)].end()) - sizeof(Edge)) = edge;
    }

    // Signal that this node is done sending data
    for (int i = 0 ; i < nb_process ; i++) {
        const Edge fake(n, n);
        buffers[i].insert(
            buffers[i].end(),
            reinterpret_cast<const char*>(&fake),
            reinterpret_cast<const char*>(&fake) + sizeof(Edge)
        );
    }

    otm_channel.send(buffers);
}

void RemDistributed::loadGraph()
{
    std::vector<char> data = otm_channel.receive();
    size_t data_pos = sizeof(uint32_t);

    size_t nb_vertices = *reinterpret_cast<uint32_t*>(&data[0]);

    internal_graph = Graph(nb_vertices);
    border_graph = Graph(nb_vertices);

    // Loops until we obtained the whole graph
    while (true) {
        while (data_pos < data.size())
        {
            Edge edge = *reinterpret_cast<Edge*>(&data[data_pos]);
            data_pos += sizeof(Edge);

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

        data = otm_channel.receive();
        data_pos = 0;
    }
    std::cout << "leaving loadGraph" << std::endl;
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

    uint32_t& r_x = task.r_x;
    uint32_t& r_y = task.r_y;
    uint32_t& p_r_y = task.p_r_y;

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
    std::vector<std::vector<char>> buffers(nb_process);

    // If there is nothing to do, give the information to everyone by creating
    //   fake tasks
    if (nothingToDo()) {
        std::vector<char> message = Task(
            internal_graph.nb_vertices, internal_graph.nb_vertices
        ).encode();

        for (int i = 0 ; i < nb_process ; i++) {
            buffers[i] = message;
        }
    }
    else {
        for (int i = 0 ; i < nb_process ; i++) {
            if (i == process)
                continue;

            std::vector<char> message;
            message.reserve((1 + todo[i].size()) * 3 * sizeof(uint32_t));

            while (!todo[i].empty()) {
                std::vector<char> task = todo[i].front().encode();
                message.insert(message.end(), task.begin(), task.end());
                todo[i].pop();
            }

            buffers[i] = message;
        }
    }

    mtm_channel.send(buffers);

    // Sequentially read all tasks
    std::vector<char> incoming_data = mtm_channel.receive_merged();
    int nb_fake_tasks = 0;

    for (size_t task = 0 ; task < incoming_data.size() ; task += 3 * sizeof(uint32_t)) {
        Task new_task = *reinterpret_cast<Task*>(&incoming_data[task]);

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
