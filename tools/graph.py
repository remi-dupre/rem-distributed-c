#!/usr/bin/python3
"""
Tool to generate a graph from outputed csv file.
Default file to be read is `mpitest.csv`.
"""
import csv
import matplotlib.pyplot as plt
import os


# File to load
FILE_NAME = 'mpitest.csv'
FILE_CLAS = 'rem.csv'     # file containing classical algortihm's timers
FILE_SHAR = 'shared.csv'  # file containing shared memory's algorithm's timers

# Directory where to save figure
SAVE_DIR = 'graph'

# Indexes of values we want to extract
COL_CMD = 1
COL_NP = 2
COL_LOC = 5
COL_DST = 6


# Prepare output directory
def mkdir(dir):
    try:
        os.mkdir(dir)
    except OSError:
        pass

mkdir(SAVE_DIR)
mkdir(SAVE_DIR + '/png')
mkdir(SAVE_DIR + '/pgf')
mkdir(SAVE_DIR + '/pdf')


datas = {}

# Load datas
with open(FILE_NAME) as file:
    reader = csv.reader(file, delimiter=';')
    next(reader)  # title row

    for row in reader:
        input = row[COL_CMD]
        np  = row[COL_NP]
        loc = int(row[COL_LOC])
        dst = int(row[COL_DST])

        if input not in datas:
            datas[input] = {
                'np': [],
                'loc': [],
                'dst': [],
                'sum': [],
                'cst': None,
                'shr': None
            }

        datas[input]['np'].append(np)
        datas[input]['loc'].append(loc)
        datas[input]['dst'].append(dst)
        datas[input]['sum'].append(loc + dst)

# Load classical's timers
with open(FILE_CLAS) as file:
    reader = csv.reader(file, delimiter=';')
    next(reader)  # title row

    for row in reader:
        input = row[1]
        time = int(row[2])

        if input in datas:
            datas[input]['cst'] = time

# Load shared memory's timers
with open(FILE_SHAR) as file:
    reader = csv.reader(file, delimiter=';')
    next(reader)  # title row

    for row in reader:
        input = row[1]
        time = int(row[2])

        if input in datas:
            datas[input]['shr'] = time


# Draw graphs
for input in datas:
    name = input.split('/')[-1].split('.')[0]

    figure = plt.figure(input)
    plt.title('REM : ' + name.title())
    plt.xlabel('Number of processes')
    plt.ylabel('Time spent (ms)')

    plt.plot(datas[input]['np'], datas[input]['loc'], ':', label='local steps')
    plt.plot(datas[input]['np'], datas[input]['dst'], ':', label='distributed steps')
    plt.plot(datas[input]['np'], datas[input]['sum'], '.-', label='total')
    plt.plot(datas[input]['np'], [datas[input]['cst']] * len(datas[input]['np']), 'k-', label='classical algorithm')
    plt.plot(datas[input]['np'], [datas[input]['shr']] * len(datas[input]['np']), 'r-', label='shared memory\'s algorithm (24 threads)')

    plt.legend()

    # Save the graph
    plt.savefig('%s/png/%s.png' % (SAVE_DIR, name))
    plt.savefig('%s/pgf/%s.pgf' % (SAVE_DIR, name))
    plt.savefig('%s/pdf/%s.pdf' % (SAVE_DIR, name))
