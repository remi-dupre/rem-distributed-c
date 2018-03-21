#include "graph.hpp"


Graph::Graph(size_t n) :
    nb_vertices(n)
{}

std::vector<std::vector<size_t>> Graph::asAdjacencyList() const
{
    std::vector<std::vector<size_t>> result(nb_vertices);

    for (const Edge& edge: edges) {
        result[edge.first].push_back(edge.second);
        result[edge.second].push_back(edge.first);
    }

    return result;
}

Graph complete_graph(size_t n)
{
    Graph graph(n);

    for (size_t x = 0 ; x < n ; x++)
        for (size_t y = 0 ; y < x ; y++)
            graph.edges.emplace_back(x, y);

    return graph;
}

Graph random_graph(size_t n, size_t m)
{
    // std::random_device()
    std::random_device r;
    std::default_random_engine generator;
    generator.seed(r());

    std::uniform_int_distribution<int> distribution(0, n-1);
    auto rand_edge = std::bind(distribution, generator);

    Graph graph(n);

    for (size_t i = 0 ; i < m ; i++)
        graph.edges.emplace_back(rand_edge(), rand_edge());

    return graph;
}

std::istream& operator>>(std::istream& input, Graph& graph)
{
    size_t n;
    input >> n;

    graph.nb_vertices = n;
    graph.edges.clear();

    size_t x, y;
    while (input >> x >> y)
        graph.edges.emplace_back(std::min(x, y), std::max(x, y));

    return input;
}

std::ostream& operator<<(std::ostream& output, const Graph& graph)
{
    output << graph.nb_vertices << std::endl;

    for (const Edge& edge: graph.edges) {
        output << edge.first << ' ' << edge.second << std::endl;
    }

    return output;
}
