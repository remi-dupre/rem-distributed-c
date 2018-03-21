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

IntPrintBin bin_format(size_t& integer)
{
    return IntPrintBin(integer);
}

EdgePrintBin bin_format(Edge& edge)
{
    return EdgePrintBin(edge);
}

GraphPrintBin bin_format(Graph& graph)
{
    return GraphPrintBin(graph);
}

std::istream& operator>>(std::istream& input, IntPrintBin integerb)
{
    uint32_t integer;

    input.read(reinterpret_cast<char*>(&integer), sizeof(uint32_t));
    integerb.ref = integer;

    return input;
}

std::istream& operator>>(std::istream& input, EdgePrintBin edgeb)
{
    input >> bin_format(edgeb.ref.first) >> bin_format(edgeb.ref.second);
    return input;
}

std::istream& operator>>(std::istream& input, GraphPrintBin graphb)
{
    Graph& graph = graphb.ref;

    input >> bin_format(graph.nb_vertices);
    graph.edges.clear();

    Edge edge;
    while (true) {
        input >> bin_format(edge);

        if (edge.first == graph.nb_vertices || edge.second == graph.nb_vertices)
            break;

        graph.edges.push_back(edge);
    }

    return input;
}

std::ostream& operator<<(std::ostream& output, const IntPrintBin& integerb)
{
    uint32_t integer = integerb.ref;
    output.write(reinterpret_cast<char*>(&integer), sizeof(uint32_t));
    return output;
}

std::ostream& operator<<(std::ostream& output, const EdgePrintBin& edgeb)
{
    Edge edge = edgeb.ref;
    output << bin_format(edge.first) << bin_format(edge.second);
    return output;
}

std::ostream& operator<<(std::ostream& output, const GraphPrintBin& graphb)
{
    const Graph& graph = graphb.ref;
    size_t n = graph.nb_vertices;
    output << bin_format(n);

    for (Edge edge: graph.edges)
        output << bin_format(edge);

    output << bin_format(n) << bin_format(n);
    return output;
}
