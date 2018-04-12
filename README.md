REM Distributed [![Build Status](https://travis-ci.org/remi-dupre/rem-distributed-c.svg?branch=master)](https://travis-ci.org/remi-dupre/rem-distributed-c)
===============
C implementation of a distributed version of REM's algorithm to process spanning forests.

This work was initiated by Fredrik Manne, and implemented by Rémi Dupré. It is based on some previous work:
 - [Experiments on Union-Find Algorithms for the Disjoint-Set Data Structure](http://www.ii.uib.no/~fredrikm/fredrik/papers/SEA2010.pdf), *Md. Mostofa Ali Patwary, Jean Blair, and Fredrik Manne*
 - [A Scalable Parallel Union-Find Algorithm for Distributed Memory Computers](http://www.ii.uib.no/~fredrikm/fredrik/papers/PPAM2009.pdf), *Fredrik Manne and Md. Mostofa Ali Patwary*


Building
--------

### Dependancies
This project is developped under **C 11**. The only extra dependancy is [**openmpi**](https://www.open-mpi.org/), that is available in most distribution's repositories.

### Build package
Running **make** will create all binary files.
```shell
git clone https://github.com/remi-dupre/rem-distributed.git
cd rem-distributed
make
```


Usage
-----
The program can be run using mpi, you must specify a binary graph input file as parameter.
```shell
mpirun -np 4 ./mpitest graph.bin
```

### Tools
Some tools are also included to manipulate graphs.
```shell
./generate random 100 250 > graph       # generate a random graph of 100 nodes and 250 edges
cat graph | ./spanning_forest > forest  # process a spanning forest of the graph
cat graph forest | ./compare            # check that two graph have the same components
./graph_to_bin graph graph.bin          # converts the graph to a binary representation
```


Graph representations
---------------------

#### Ascii formating
A first line specifies the number of nodes of the graph. It implies that the nodes are identified by indexes from `0` to `number_of_nodes-1`.

An arbitrary number of lines follows, each of them contains two integers separated by a space, representing an edge between thoose two indexes.

```
4                        1 -- 2
0 1      represents      |    
1 2                      0    3
```

#### Binary formating
The binary representation of a graph contains a sequence of 32 bits integers, the first one is the number of nodes, and the others are pairs of integers representing edges.
