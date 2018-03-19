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
