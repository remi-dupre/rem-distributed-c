static inline bool rem_insert(Edge edge, Node* uf_parent)
{
    #define p(x) uf_parent[x]

    while (p(edge.x) != p(edge.y)) {
        if (p(edge.x) < p(edge.y)) {
            if (p(edge.y) == edge.y) {
                p(edge.y) = p(edge.x);
                return false;
            }

            const Node save_pry = p(edge.y);
            p(edge.y) = p(edge.x);
            edge.y = save_pry;
        }
        else {
            if (p(edge.x) == edge.x) {
                p(edge.x) = p(edge.y);
                return false;
            }

            const Node save_prx = p(edge.x);
            p(edge.x) = p(edge.y);
            edge.x = save_prx;
        }
    }

    return true;
    #undef p
}
