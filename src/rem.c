#include "rem.h"


void rem_update(const Edge* edges, int nb_edges, uint32_t* uf_parent)
{
    #define p(x) uf_parent[x]

    for (int i = 0 ; i < nb_edges ; i++) {
        uint32_t r_x = edges[i].x;
        uint32_t r_y = edges[i].y;

        while (r_x != r_y) {
            if (r_x < r_y) {
                if (p(r_y) == r_y) {
                    p(r_y) = r_x;
                }
                else {
                    const int save_pry = p(r_y);
                    p(r_y) = r_x;
                    r_y = save_pry;
                }
            }
            else {
                if (p(r_x) == r_x) {
                    p(r_x) = r_y;
                }
                else {
                    const int save_prx = p(r_x);
                    p(r_x) = r_y;
                    r_x = save_prx;
                }
            }
        }
    }

    #undef p
}

uint32_t repr(uint32_t node, uint32_t* uf_parent)
{
    #define p(x) uf_parent[x]

    if (p(node) == node) {
        return node;
    }
    else {
        const uint32_t root = repr(p(node), uf_parent);
        p(node) = root;
        return root;
    }

    #undef p
}
