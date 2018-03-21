#!/usr/bin/python3
"""
Sequentialy simulate the distributed algorithm.
Usage:
    `python3 test.py`       ouputs a spanning tree of input graph
    `python3 test.py debug` give some general informations about the calculation
"""
from sys import argv


nprocs = 4

# Read number of nodes and edges from stdin
n, m = map(int, input().split(' '))

# Initialisation of the union find
p_process = [[i for i in range(n)] * 2 for j in range(nprocs)]
owner = [i % nprocs for i in range(n)] * 2

# Gives the edges to corresponding process
edges = [set() for i in range(nprocs)]
surounding_edges = [set() for i in range(nprocs)]

# Edges that are part of the spanning tree
keep = [[] for i in range(nprocs)]
keep_surounding = [[] for i in range(nprocs)]

# Read edges from stdin
for i in range(m):
    x, y = map(int, input().split(' '))
    x, y = min(x, y), max(x, y)

    if owner[x] == owner[y]:
        edges[owner[x]].add((x, y))
    else:
        surounding_edges[owner[x]].add((y + n, x))
        # surounding_edges[owner[y]].add((x + n, y))  # no need to add twice a gost edge

def REM(process, x, y):
    """
    Simulate classical REM, inside process.
    """
    p = p_process[process]
    r_x, r_y = x, y

    while p[r_x] != p[r_y]:
        if p[r_x] < p[r_y]:
            if r_x == p[r_x]:
                p[r_x] = p[r_y]
                return True
            else:
                p[r_x], r_x = p[r_y], p[r_x]
        else:
            if r_y == p[r_y]:
                p[r_y] = p[r_x]
                return True
            else:
                p[r_y], r_y = p[r_x], p[r_y]

    return False

# Run REM algorithm on each proc
for process in range(nprocs):
    # Run the algorithm on the edges inside the subgraph
    for x, y in edges[process]:
        if REM(process, x, y):
            keep[process].append((x, y))

    save_uf = p_process[process][:n]

    # Run the algorithm on extra edges
    for x, y in surounding_edges[process]:
        if REM(process, x, y):
            keep_surounding[process].append((x, y))

    p_process[process] = save_uf

def local_root(process, x):
    p = p_process[process]

    if p[x] == x or owner[p[x]] != process:
        return x
    else:
        z = local_root(process, p[x])
        p[x] = z
        return z

NCOMS = 0
def distributed_REM(process, r_x, r_y, x, y, pr_y=None):
    global NCOMS
    NCOMS += 1

    # print('%d: %d, %d (%d, %d)' % (process, r_x, r_y, x, y))
    p = p_process[process]

    if owner[r_y] == process:
        pr_y = p[r_y]
    else:
        p[r_y] = pr_y

    r_x = local_root(process, r_x)

    if owner[r_y] == process:
        r_y = local_root(process, r_y)

    if p[r_x] == p[r_y]:
        return
    if p[r_x] == r_y:
        p[r_x] = p[r_y]
        return
    if p[r_y] == r_x:
        p[r_y] = p[r_x]
        return

    if p[r_x] < p[r_y]:
        if r_x == p[r_x]:
            p[r_x] = p[r_y]
            keep[process].append((x, y))
        else:
            z = p[r_x]
            p[r_x] = p[r_y]
            distributed_REM(owner[z], z, r_y, x, y, p[r_y]) # COM
    else:
        if r_y == p[r_y]:
            distributed_REM(owner[r_y], r_y, r_x, x, y, p[r_x]) # COM
        else:
            z = p[r_y]
            NCOMS += 1
            p_process[owner[r_y]][r_y] = p[r_x] # COM
            distributed_REM(owner[z], z, r_x, x, y, p[r_x]) # COM

for process in range(nprocs):
    # Simulate the distributed calls
    for x, y in surounding_edges[process]:
        x -= n  # This is a gost vertex
        NCOMS -= 1  # The first call isn't a COM
        distributed_REM(owner[x], x, y, x, y, y)


if len(argv) > 1 and argv[1] == 'debug':
    # Debug some general informations
    for process in range(nprocs):
        print('--- Process', process)
        # print(' - nodes:', [x for x in range(n) if owner[x] == process])
        # print(' - keeps:', keep[process])
        # print(' - surounding:', surounding_edges[process])
        print(' - nodes:', len([x for x in range(n) if owner[x] == process]))
        print(' - edges:', len(edges[process]))
        print(' - keeps:', len(keep[process]))
        print(' - surounding:', len(surounding_edges[process]))


    print('--- A total of', sum((len(l) for l in keep)), 'edges')
    print('--- Communications: ', NCOMS)
else:
    # Print the resulting tree
    print(n, sum((len(l) for l in keep)))

    for bucket in keep:
        for (x, y) in bucket:
            print(x, y)
