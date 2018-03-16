"""
Simulate the distributed algorithm.
"""
nprocs = 4

# Read number of nodes and edges from stdin
n, m = map(int, input().split(' '))

# Initialisation of the union find
p_process = [[i for i in range(n)] for j in range(nprocs)]
owner = [i % nprocs for i in range(n)]

# Gives the edges to corresponding process
edges = [set() for i in range(nprocs)]
surounding_edges = [set() for i in range(nprocs)]

# Edges that are part of the spaning tree
keep = [[] for i in range(nprocs)]
keep_surounding = [[] for i in range(nprocs)]

gosts = 0
gost_repr = dict()
gost_from = dict()

gost_owner = []
gost_p = []

# Read edges from stdin
for i in range(m):
    x, y = map(int, input().split(' '))
    x, y = min(x, y), max(x, y)

    if owner[x] == owner[y]:
        edges[owner[x]].add((x, y))
    else:
        if x not in gost_repr.keys():
            gost_repr[x] = gosts - 1
            gost_from[gosts - 1] = x
            gost_owner.append(owner[x])
            gost_p.append(x)
            gosts -= 1
        if y not in gost_repr.keys():
            gost_repr[y] = gosts - 1
            gost_from[gosts - 1] = y
            gost_owner.append(owner[y])
            gost_p.append(y)
            gosts -= 1

        surounding_edges[owner[x]].add((gost_repr[y], x))
        surounding_edges[owner[y]].add((gost_repr[x], y))

gost_owner.reverse()
gost_p.reverse()

owner = owner + gost_owner

for process in range(nprocs):
    p_process[process] += gost_p
    p_process[process] += gost_p

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

def distributed_REM(process, r_x, r_y, x, y, pr_y=None):
    # print('%d: %d, %d (%d, %d)' % (process, r_x, r_y, x, y))
    p = p_process[process]

    if pr_y is None:
        assert(owner[r_y] == process)
        pr_y = p[r_y]
    else:
        p[r_y] = pr_y

    r_x = local_root(process, r_x)

    if owner[r_y] == process:
        r_y = local_root(process, r_y)

    if r_x == r_y or p[r_x] == p[r_y]:
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
            p_process[owner[r_y]][r_y] = p[r_x] # COM
            keep[process].append((x, y))
        else:
            z = p[r_y]
            p_process[owner[r_y]][r_y] = p[r_x] # COM
            distributed_REM(owner[z], z, r_x, x, y, p[r_x]) # COM

# Simulate the distributed calls
for process in range(nprocs):
    for x, y in surounding_edges[process]:
        x = gost_from[x]
        distributed_REM(owner[y], y, x, x, y, p_process[owner[x]][x]) # COM


# Debug some results
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
