#include "graph.hpp"


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
    std::default_random_engine generator;
    std::uniform_int_distribution<int> distribution(0, n);
    auto rand_edge = std::bind(distribution, generator);

    Graph graph(n);

    for (size_t i = 0 ; i < m ; i++)
        graph.edges.emplace_back(rand_edge(), rand_edge());

    return graph;
}

std::istream& operator>>(std::istream& input, Graph& graph)
{
    int n, m;
    input >> n >> m;

    assert(n >= 0);
    assert(m >= 0);

    graph.nb_vertices = n;
    graph.edges.clear();

    size_t x, y;
    for (int i = 0 ; i < m ; i++) {
        input >> x >> y;
        graph.edges.emplace_back(x, y);
    }

    return input;
}

std::ostream& operator<<(std::ostream& output, const Graph& graph)
{
    output << graph.nb_vertices << ' ' << graph.edges.size() << std::endl;

    for (const Edge& edge: graph.edges) {
        output << edge.first << ' ' << edge.second << std::endl;
    }

    return output;
}
