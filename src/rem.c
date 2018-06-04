#include "rem.h"


void rem_update(const Edge* edges, size_t nb_edges, Node* uf_parent)
{
    for (size_t i = 0 ; i < nb_edges ; i++)
        rem_insert(edges[i], uf_parent);
}

void rem_shared_update(Edge* edges, size_t nb_edges, Node* uf_parent, int nb_threads, bool skip_insert)
{
    int nb_working = nb_threads;


    #pragma omp parallel num_threads(nb_threads)
    {
        bool done = false;

        int id = omp_get_thread_num();

        size_t begin = (id * nb_edges) / nb_threads;
        size_t end = ((id + 1) * nb_edges) / nb_threads;

        while (nb_working > 0) {
            size_t new_end = begin;

            if (!skip_insert) {
                for (size_t i = begin ; i < end ; i++) {
                    Edge edge = edges[i];

                    #define p(x) (uf_parent[x])
                    while (p(edge.x) != p(edge.y)) {
                        if (p(edge.x) > p(edge.y)) {
                            if(edge.x == p(edge.x)) {
                                p(edge.x) = p(edge.y);
                                edges[new_end] = edge;
                                new_end++;
                            }
                            else {
                                const Node z = p(edge.x);
                                p(edge.x) = p(edge.y);
                                edge.x = z;
                            }
                        }
                        else {
                            if (edge.y == p(edge.y)) {
                                p(edge.y) = p(edge.x);
                                edges[new_end] = edge;
                                new_end++;
                            }
                            else {
                                const Node z = p(edge.y);
                                p(edge.y) = p(edge.x);
                                edge.y = z;
                            }
                        }
                    }
                    #undef p
                }

                end = new_end;
                new_end = begin;
            }

            skip_insert = false;

            #pragma omp barrier
            for (size_t i = begin ; i < end ; i++) {
                if (!rem_compare(edges[i], uf_parent)) {
                    edges[new_end] = edges[i];
                    new_end++;
                }
            }

            if (begin == new_end && !done) {
                #pragma omp atomic
                nb_working--;
                done = true;
            }
            #pragma omp barrier

            end = new_end;
        }
    }
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
