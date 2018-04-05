#!/usr/bin/python3
"""
Execute a sequence of tests on graphs listed in inputs/..
"""
import os, sys

import subprocess


# Configuration of different binaries
BIN_GEN = "./generate"
BIN_CONV = "./graph-to-bin"

# Generate some extra random graphs
NB_RANDOM = 100
RANDOM_NB_NODES = 10000
RANDOM_NB_EDGES = 25000


# Generate random graphs
for i in range(NB_RANDOM):
    print('Generated extra random graphs (%d/%d)' % (i+1, NB_RANDOM), end='\r')
    command = [BIN_GEN, 'random', str(RANDOM_NB_NODES), str(RANDOM_NB_EDGES)]

    with open('inputs/random_%03d' % i, 'w') as file:
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

errors_count = 0

# Execute the algorithm
for i in range(len(inputs)):
    input = inputs[i]
    print('Testing %s (%d/%d) ... ' % (input, i+1, len(inputs)), end='', flush=True)

    command = ['mpirun', '-np', '4', './mpitest', 'inputs/' + input + '.bin']
    with open('inputs/' + input + '.out', 'w') as file:
        rv = subprocess.call(command, stdout=file, stderr=subprocess.PIPE)

    if rv != 0:
        errors_count += 1
        print('error %d on computation' % rv)
        continue

    command = ['./compare', 'inputs/' + input, 'inputs/' + input + '.out']
    rv = subprocess.call(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    if rv == 1:
        errors_count += 1
        print('wrong')
    elif rv != 0:
        errors_count += 1
        print('error %d on comparison' % rv)
    else:
        print('ok')


print('Ended with %d errors' % errors_count)

if errors_count != 0:
    sys.exit(1)
