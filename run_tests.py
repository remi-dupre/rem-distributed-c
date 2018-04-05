#!/usr/bin/python3
"""
Execute a sequence of tests on graphs listed in inputs/..
"""
import os
import subprocess


# Configuration of different binaries
BIN_GEN = "./generate"
BIN_CONV = "./graph-to-bin"

# Generate some extra random graphs
NB_RANDOM = 5
RANDOM_NB_NODES = 10000
RANDOM_NB_EDGES = 25000


# Generate random graphs
for i in range(NB_RANDOM):
    print('Generated extra random graphs (%d/%d)' % (i+1, NB_RANDOM), end='\r')
    command = [BIN_GEN, 'random', str(RANDOM_NB_NODES), str(RANDOM_NB_EDGES)]

    with open('inputs/random_%d' % i, 'w') as file:
        subprocess.call(command, stdout=file)

print()


# List of input files
inputs = sorted([file for file in os.listdir('inputs') if file[-4:] not in ['.bin', '.out']])
for i in range(len(inputs)):
    print('Convert graphs to binary (%d/%d)' % (i+1, len(inputs)), end='\r')
    input = inputs[i]

    command = [BIN_CONV, 'inputs/' + input]
    subprocess.call(command)

print()


# Execute the algorithm
for i in range(len(inputs)):
    input = inputs[i]
    print('Testing %s (%d/%d) ... ' % (input, i+1, len(inputs)), end='')

    command = ['mpirun', '-np', '4', './mpitest', 'inputs/' + input + '.bin']
    with open('inputs/' + input + '.out', 'w') as file:
        rv = subprocess.call(command, stdout=file, stderr=subprocess.PIPE)

    if rv != 0:
        print('error %d on computation' % rv)
        continue

    # Add number of vertices at begining of file
    command = 'head -n 1 inputs/%s | cat - inputs/%s.out > inputs/%s.out.temp' % (input, input, input)
    subprocess.call(command, shell=True)
    subprocess.call('mv inputs/%s.out.temp inputs/%s.out' % (input, input), shell=True)

    command = ['./compare', 'inputs/' + input, 'inputs/' + input + '.out']
    rv = subprocess.call(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    if rv == 1:
        print('wrong')
    elif rv != 0:
        print('error %d on comparison' % rv)
    else:
        print('ok')
