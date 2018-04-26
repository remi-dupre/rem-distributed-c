#include "rem.h"


void rem_update(const Edge* edges, size_t nb_edges, Node* uf_parent)
{
    #define p(x) uf_parent[x]

    for (size_t i = 0 ; i < nb_edges ; i++) {
        Node r_x = edges[i].x;
        Node r_y = edges[i].y;

        while (p(r_x) != p(r_y)) {
            if (p(r_x) < p(r_y)) {
                if (p(r_y) == r_y) {
                    p(r_y) = p(r_x);
                }
                else {
                    const Node save_pry = p(r_y);
                    p(r_y) = p(r_x);
                    r_y = save_pry;
                }
            }
            else {
                if (p(r_x) == r_x) {
                    p(r_x) = p(r_y);
                }
                else {
                    const Node save_prx = p(r_x);
                    p(r_x) = p(r_y);
                    r_x = save_prx;
                }
            }
        }
    }

    #undef p
}

Node repr(Node node, Node* uf_parent)
{
    #define p(x) uf_parent[x]

    if (p(node) == node) {
        return node;
    }
    else {
        const Node root = repr(p(node), uf_parent);
        p(node) = root;
        return root;
    }

    #undef p
}
