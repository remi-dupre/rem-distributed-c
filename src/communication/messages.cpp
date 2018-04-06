#include "messages.hpp"


std::vector<char> Task::encode() const
{
    std::vector<char> encoded;
    encoded.reserve(sizeof(uint32_t) * 3);

    encoded.insert(
        encoded.end(),
        reinterpret_cast<const char*>(&r_x),
        reinterpret_cast<const char*>(&r_x) + sizeof(uint32_t)
    );
    encoded.insert(
        encoded.end(),
        reinterpret_cast<const char*>(&r_y),
        reinterpret_cast<const char*>(&r_y) + sizeof(uint32_t)
    );

    return encoded;
}

std::istream& operator>>(std::istream& input, Task& task)
{
    input.read(reinterpret_cast<char*>(&task.r_x), sizeof(uint32_t));
    input.read(reinterpret_cast<char*>(&task.r_y), sizeof(uint32_t));

    return input;
}

std::ostream& operator<<(std::ostream& output, const Task& task)
{
    output.write(reinterpret_cast<const char*>(&task.r_x), sizeof(uint32_t));
    output.write(reinterpret_cast<const char*>(&task.r_y), sizeof(uint32_t));

    return output;
}

void bin_write(const Graph& graph, std::ostream& output)
{
    bin_write(graph.nb_vertices, output);

    for (const Edge& edge: graph.edges)
        bin_write(edge, output);
}

void bin_write(const Edge& edge, std::ostream& output)
{
    bin_write(edge.first, output);
    bin_write(edge.second, output);
}

void bin_write(size_t node, std::ostream& output)
{
    const uint32_t node32 = node;
    output.write(
        reinterpret_cast<const char*>(&node32),
        sizeof(uint32_t)
    );
}

Graph bin_readg(std::istream& input)
{
    Graph graph;
    graph.nb_vertices = bin_readn(input);

    while (!input.eof())
    {
        graph.edges.push_back(bin_reade(input));

        if (input.eof())
            graph.edges.pop_back();
    }

    return graph;
}

Edge bin_reade(std::istream& input)
{
    size_t x = bin_readn(input);
    size_t y = bin_readn(input);
    return std::make_pair(x, y);
}

size_t bin_readn(std::istream& input)
{
    uint32_t node32;
    input.read(reinterpret_cast<char*>(&node32), sizeof(uint32_t));
    return node32;
}
