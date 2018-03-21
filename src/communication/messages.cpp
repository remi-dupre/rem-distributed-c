#include "messages.hpp"

std::istream& operator>>(std::istream& input, Task& task)
{
    uint32_t r_x, r_y, p_r_y;

    input.read(reinterpret_cast<char*>(&r_x), sizeof(uint32_t));
    input.read(reinterpret_cast<char*>(&r_y), sizeof(uint32_t));
    input.read(reinterpret_cast<char*>(&p_r_y), sizeof(uint32_t));

    task.r_x = r_x;
    task.r_y = r_y;
    task.p_r_y = p_r_y;

    return input;
}

std::ostream& operator<<(std::ostream& output, const Task& task)
{
    uint32_t r_x = task.r_x;
    uint32_t r_y = task.r_y;
    uint32_t p_r_y = task.p_r_y;

    output.write(reinterpret_cast<char*>(&r_x), sizeof(uint32_t));
    output.write(reinterpret_cast<char*>(&r_y), sizeof(uint32_t));
    output.write(reinterpret_cast<char*>(&p_r_y), sizeof(uint32_t));

    return output;
}

GraphPrintBin bin_format(Graph& graph)
{
    return GraphPrintBin(graph);
}

std::istream& operator>>(std::istream& input, GraphPrintBin graphb)
{
    Graph& graph = graphb.ref;

    uint32_t n, x, y;

    input.read(reinterpret_cast<char*>(&n), sizeof(uint32_t));

    graph.nb_vertices = n;
    graph.edges.clear();

    while (true) {
        input.read(reinterpret_cast<char*>(&x), sizeof(uint32_t));
        input.read(reinterpret_cast<char*>(&y), sizeof(uint32_t));

        if (x == n || y == n)
            break;

        graph.edges.emplace_back(x, y);
    }

    return input;
}

std::ostream& operator<<(std::ostream& output, const GraphPrintBin& graphb)
{
    const Graph& graph = graphb.ref;

    uint32_t n = graph.nb_vertices;

    output.write(reinterpret_cast<char*>(&n), sizeof(uint32_t));

    for (const Edge& edge: graph.edges) {
        uint32_t x = edge.first;
        uint32_t y = edge.second;

        output.write(reinterpret_cast<char*>(&y), sizeof(uint32_t));
        output.write(reinterpret_cast<char*>(&x), sizeof(uint32_t));
    }

    output.write(reinterpret_cast<char*>(&n), sizeof(uint32_t));
    output.write(reinterpret_cast<char*>(&n), sizeof(uint32_t));
    return output;
}
