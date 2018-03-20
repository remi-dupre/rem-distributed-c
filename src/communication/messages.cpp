#include "messages.hpp"


Task::Task(size_t r_x, size_t r_y, size_t x, size_t y, size_t p_r_y) :
    original(x, y),
    current(r_x, r_y),
    p_r_y(p_r_y)
{}

std::istream& operator>>(std::istream& input, Task& task)
{
    uint32_t x, y, r_x, r_y, p_r_y;

    input.read(reinterpret_cast<char*>(&x), sizeof(uint32_t));
    input.read(reinterpret_cast<char*>(&y), sizeof(uint32_t));
    input.read(reinterpret_cast<char*>(&r_x), sizeof(uint32_t));
    input.read(reinterpret_cast<char*>(&r_y), sizeof(uint32_t));
    input.read(reinterpret_cast<char*>(&p_r_y), sizeof(uint32_t));

    task.original = std::make_pair(x, y);
    task.current = std::make_pair(x, y);
    task.p_r_y = p_r_y;

    return input;
}

std::ostream& operator<<(std::ostream& output, const Task& task)
{
    uint32_t x = task.original.first;
    uint32_t y = task.original.second;
    uint32_t r_x = task.current.first;
    uint32_t r_y = task.current.second;
    uint32_t p_r_y = task.p_r_y;

    output.write(reinterpret_cast<char*>(&x), sizeof(uint32_t));
    output.write(reinterpret_cast<char*>(&y), sizeof(uint32_t));
    output.write(reinterpret_cast<char*>(&r_x), sizeof(uint32_t));
    output.write(reinterpret_cast<char*>(&r_y), sizeof(uint32_t));
    output.write(reinterpret_cast<char*>(&p_r_y), sizeof(uint32_t));

    return output;
}

std::istream& operator>>(std::istream& input, GraphPrintBin& graphb)
{
    Graph& graph = graphb.ref;

    uint32_t n, m, x, y;

    input.read(reinterpret_cast<char*>(&n), sizeof(uint32_t));
    input.read(reinterpret_cast<char*>(&m), sizeof(uint32_t));

    graph.nb_vertices = n;
    graph.edges.clear();

    for (size_t i = 0 ; i < m ; i++) {
        input.read(reinterpret_cast<char*>(&x), sizeof(uint32_t));
        input.read(reinterpret_cast<char*>(&y), sizeof(uint32_t));
        graph.edges.emplace_back(x, y);
    }

    return input;
}

std::ostream& operator<<(std::ostream& output, const GraphPrintBin& graphb)
{
    const Graph& graph = graphb.ref;

    uint32_t n = graph.nb_vertices;
    uint32_t m = graph.edges.size();

    output.write(reinterpret_cast<char*>(&n), sizeof(uint32_t));
    output.write(reinterpret_cast<char*>(&m), sizeof(uint32_t));

    for (const Edge& edge: graph.edges) {
        uint32_t x = edge.first;
        uint32_t y = edge.second;

        output.write(reinterpret_cast<char*>(&y), sizeof(uint32_t));
        output.write(reinterpret_cast<char*>(&x), sizeof(uint32_t));
    }

    return output;
}
