#include "rem.h"


void rem_update(const Edge* edges, size_t nb_edges, Node* uf_parent)
{
    for (size_t i = 0 ; i < nb_edges ; i++)
        rem_insert(edges[i], uf_parent);
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
