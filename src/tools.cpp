#include "tools.hpp"


bool same_components(const Graph& g1, const Graph& g2)
{
    assert(g1.nb_vertices == g2.nb_vertices);

    std::vector<size_t> comps1 = rem_components(g1);
    std::vector<size_t> comps2 = rem_components(g2);

    std::function<size_t(const std::vector<size_t>&, size_t)> repr = [&repr] (const std::vector<size_t>& p, size_t v) -> size_t
    {
        if (p[v] == v)
            return v;
        else
            return repr(p, p[v]);
    };

    for (size_t i = 0 ; i < g1.nb_vertices ; i++)
        if (repr(comps1, i) != repr(comps2, i)){
            std::cout << i << ':' << repr(comps1, i) << std::endl;
            std::cout << repr(comps2, i) << std::endl;
            return false;
        }

    return true;
}

bool is_tree(const Graph& graph)
{
    return graph.nb_vertices - 1 == graph.edges.size() && is_forest(graph);
}

bool is_forest(const Graph& graph)
{
    auto graph_adj = graph.asAdjacencyList();
    std::vector<bool> visited(graph.nb_vertices, false);

    std::function<bool(size_t, size_t)> run = [&graph_adj, &visited, &run] (size_t v, size_t p) {
        if (visited[v])
            return false;

        visited[v] = true;
        for (size_t t: graph_adj[v]) {
            if (t != p && !run(t, v))
                return false;
        }
        return true;
    };

    for (size_t i = 0 ; i < graph.nb_vertices ; i++) {
        if (!visited[i] && !run(i, -1))
            return false;
    }

    return true;
}

std::vector<Graph> split(const Graph& graph, size_t nb_parts)
{
    std::vector<Graph> result(nb_parts, Graph(graph.nb_vertices));

    for (const Edge& edge: graph.edges) {
        result[edge.first % nb_parts].edges.push_back(edge);

        if (edge.first % nb_parts != edge.second % nb_parts)
            result[edge.second % nb_parts].edges.push_back(edge);
    }

    return result;
}

void redistribute(Graph& graph, int nb_parts)
{
    int n = graph.nb_vertices;
    int p = nb_parts;
    auto f = [n, p] (size_t x) {
        return (((n + 1) * p * x) / n) % n;
    };

    for (Edge& edge: graph.edges)
    {
        edge.first = f(edge.first);
        edge.second = f(edge.second);
    }
}
