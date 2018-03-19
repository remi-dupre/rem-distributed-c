#include "rem.hpp"


std::pair<Graph, std::vector<size_t>> rem(const Graph& graph)
{
    // Graph containing our solution
    Graph spaning_tree(graph.nb_vertices);

    // Disjoint set structure
    std::vector<size_t> p(graph.nb_vertices);
    for (size_t i = 0 ; i < p.size() ; i++)
        p[i] = i;

    // Check every candidate edge
    for (const Edge& edge: graph.edges) {
        size_t r_x = edge.first;
        size_t r_y = edge.second;

        while (p[r_x] != p[r_y]) {
            if (p[r_x] < p[r_y]) {
                if (r_x == p[r_x]) {
                    p[r_x] = p[r_y];
                    spaning_tree.edges.push_back(edge);
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
                    spaning_tree.edges.push_back(edge);
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

    return std::make_pair(spaning_tree, p);
}

std::vector<size_t> rem_components(const Graph& graph)
{
    return rem(graph).second;
}

Graph rem_spaning(const Graph& graph)
{
    return rem(graph).first;
}
