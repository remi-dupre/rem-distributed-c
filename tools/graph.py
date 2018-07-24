#!/usr/bin/python3
"""
Tool to generate a graph from outputed csv file.
Default file to be read is `mpitest.csv`.
"""
import csv
import matplotlib.pyplot as plt
import os

from numpy import mean


# File to load
FILE_NAME = 'mpitest.csv'
FILE_CLAS = 'rem.csv'     # file containing classical algortihm's timers
FILE_SHAR = 'shared.csv'  # file containing shared memory's algorithm's timers

# Directory where to save figure
SAVE_DIR = 'graph'

# Indexes of values we want to extract
COL_CMD = 1
COL_NP = 2
COL_THREADS = 3
COL_LOC = 6
COL_DST = 7


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
        infile = row[COL_CMD]
        np  = int(row[COL_NP])
        threads = int(row[COL_THREADS])
        loc = int(row[COL_LOC])
        dst = int(row[COL_DST])

        if infile not in datas:
            datas[infile] = {
                'np': [],
                'threads': [],
                'loc': [],
                'dst': [],
                'sum': [],
                'cst': None,
                'shr': None
            }

        datas[infile]['np'].append(np)
        datas[infile]['threads'].append(threads)
        datas[infile]['loc'].append(loc)
        datas[infile]['dst'].append(dst)
        datas[infile]['sum'].append(loc + dst)

# Load classical's timers
with open(FILE_CLAS) as file:
    reader = csv.reader(file, delimiter=';')
    next(reader)  # title row
    values = dict()

    for row in reader:
        infile = row[1]
        time = int(row[2])

        if infile not in values:
            values[infile] = []

        values[infile].append(time)

    for infile in values:
        if infile in datas:
            datas[infile]['cst'] = sum(values[infile]) / len(values[infile])

# Load shared memory's timers
with open(FILE_SHAR) as file:
    reader = csv.reader(file, delimiter=';')
    next(reader)  # title row
    values = dict()

    for row in reader:
        infile = row[1]
        time = int(row[2])

        if infile not in values:
            values[infile] = []

        values[infile].append(time)

    for infile in values:
        if infile in datas:
            datas[infile]['shr'] = sum(values[infile]) / len(values[infile])

        datas[infile]['cst'] = [datas[infile]['cst']] * len(datas[infile]['np'])
        datas[infile]['shr'] = [datas[infile]['shr']] * len(datas[infile]['np'])


# Choose axis to plot
x_axis = input('Choose x-axis (%s) [np]: ' % ', '.join(list(datas[list(datas)[0]])[:-2]))
y_axis = input('Choose y-axis (%s) [cst,shr,loc,dst,sum]: ' % ', '.join(list(datas[list(datas)[0]])[:-2]))
filter = input('Choose a filter on datas to keep [True]: ')

if not x_axis: x_axis = 'np'
if not y_axis: y_axis = 'cst,shr,loc,dst,sum'
if not filter: filter = 'True'

y_axis = y_axis.split(',')


# Draw graphs
plot_setup = {
    'loc': {
        'label': 'local steps',
        'style': ':'
    },
    'dst': {
        'label': 'distributed steps',
        'style': ':'
    },
    'sum': {
        'label': 'total',
        'style': '.-'
    },
    'cst': {
        'label': 'classical algorithm',
        'style': 'k-'
    },
    'shr': {
        'label': 'shared memory\'s algorithm (24 threads)',
        'style': 'r-'
    },
    'np': {
        'label': 'number of process (distributed memory)'
    },
    'threads': {
        'label': 'number of threads (shared memory) per process'
    }
}

for infile in datas:
    name = infile.split('/')[-1].split('.')[0]

    # Select axis datas
    print(infile)
    file_datas = datas[infile]

    xy_values = [
        (file_datas[x_axis][i], [
            file_datas[curve][i]
            for curve in y_axis
            if eval(filter, {c: file_datas[c][i] for c in file_datas.keys()})
        ]) for i in range(len(file_datas[y_axis[0]]))
    ]
    x_values = sorted(list(set([x for (x, y) in xy_values])))
    y_values = [[] for i in range(len(y_axis))]
    y_pre_values = [[y for (x, y) in xy_values if x == x_values[i]] for i in range(len(x_values))]


    for Y in y_pre_values:
        dim = len(y_axis)
        val = [[] for i in range(dim)]

        for y in Y:
            for i in range(dim):
                if y:
                    val[i].append(y[i])

        for i in range(dim):
            y_values[i].append(mean(val[i]))

    figure = plt.figure(infile)
    plt.title('REM : ' + name.title())
    plt.xlabel(plot_setup[x_axis]['label'])
    plt.ylabel('Time spent (ms)')

    for curve in range(len(y_values)):
        setup = plot_setup[y_axis[curve]]
        plt.plot(x_values, y_values[curve], setup['style'], label=setup['label'])

    # plt.plot(datas[infile]['np'], datas[infile]['loc'], ':', label='local steps')
    # plt.plot(datas[infile]['np'], datas[infile]['dst'], ':', label='distributed steps')
    # plt.plot(datas[infile]['np'], datas[infile]['sum'], '.-', label='total')
    # plt.plot(datas[infile]['np'], [datas[infile]['cst']] * len(datas[infile]['np']), 'k-', label='classical algorithm')
    # plt.plot(datas[infile]['np'], [datas[infile]['shr']] * len(datas[infile]['np']), 'r-', label='shared memory\'s algorithm (24 threads)')

    plt.legend()

    # Save the graph
    plt.savefig('%s/png/%s.png' % (SAVE_DIR, name))
    plt.savefig('%s/pgf/%s.pgf' % (SAVE_DIR, name))
    plt.savefig('%s/pdf/%s.pdf' % (SAVE_DIR, name))
