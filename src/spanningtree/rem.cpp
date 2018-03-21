#include "rem.hpp"


Graph rem_spanning_inplace(const Graph& graph, std::vector<size_t>& initial)
{
    // Graph containing our solution
    Graph spanning_tree(graph.nb_vertices);

    // Disjoint set structure
    std::vector<size_t>& p = initial;

    // Check every candidate edge
    for (const Edge& edge: graph.edges) {
        size_t r_x = edge.first;
        size_t r_y = edge.second;

        while (p[r_x] != p[r_y]) {
            if (p[r_x] < p[r_y]) {
                if (r_x == p[r_x]) {
                    p[r_x] = p[r_y];
                    spanning_tree.edges.push_back(edge);
                    break;
                }
                else {
                    size_t z = p[r_x];
                    p[r_x] = p[r_y];
                    r_x = z;
                }
            }
            else {
                if (r_y == p[r_y]) {
                    p[r_y] = p[r_x];
                    spanning_tree.edges.push_back(edge);
                    break;
                }
                else {
                    size_t z = p[r_y];
                    p[r_y] = p[r_x];
                    r_y = z;
                }
            }
        }
    }

    return spanning_tree;
}

Graph rem_spanning(const Graph& graph, const std::vector<size_t>& initial)
{
    std::vector<size_t> uf(initial);
    return rem_spanning_inplace(graph, uf);
}

Graph rem_spanning(const Graph& graph)
{
    std::vector<size_t> uf(graph.nb_vertices);

    for (size_t i = 0 ; i < graph.nb_vertices ; i++)
        uf[i] = i;

    return rem_spanning_inplace(graph, uf);
}

std::vector<size_t> rem_components(const Graph& graph, const std::vector<size_t>& initial)
{
    std::vector<size_t> uf(initial);
    rem_spanning_inplace(graph, uf);
    return uf;
}

std::vector<size_t> rem_components(const Graph& graph)
{
    std::vector<size_t> uf(graph.nb_vertices);

    for (size_t i = 0 ; i < graph.nb_vertices ; i++)
        uf[i] = i;

    rem_spanning_inplace(graph, uf);
    return uf;
}
