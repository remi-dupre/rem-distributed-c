"""
Tool to generate a graph from outputed csv file.
Default file to be read is `mpitest.csv`.
"""
import csv
import matplotlib.pyplot as plt
import os


# File to load
FILE_NAME = 'mpitest.csv'

# Directory where to save figure
SAVE_DIR = 'graph'

# Indexes of values we want to extract
COL_CMD = 1
COL_NP = 2
COL_LOC = 5
COL_DST = 6


# Prepare output directory
try:
    os.mkdir(SAVE_DIR)
except OSError:
    pass

try:
    os.mkdir(SAVE_DIR + '/png')
except OSError:
    pass

try:
    os.mkdir(SAVE_DIR + '/pgf')
except OSError:
    pass



datas = {}

# Load datas
with open(FILE_NAME) as file:
    reader = csv.reader(file, delimiter=';')

    next(reader)  # title column
    for row in reader:
        cmd = row[COL_CMD]
        np  = row[COL_NP]
        loc = int(row[COL_LOC])
        dst = int(row[COL_DST])

        if cmd not in datas:
            datas[cmd] = {
                'np': [],
                'loc': [],
                'dst': [],
                'sum': []
            }

        datas[cmd]['np'].append(np)
        datas[cmd]['loc'].append(loc)
        datas[cmd]['dst'].append(dst)
        datas[cmd]['sum'].append(loc + dst)

# Draw graphs
for cmd in datas:
    name = cmd.split(' ')[-1].split('/')[-1].split('.')[0]

    figure = plt.figure(cmd)
    plt.title('REM : ' + name.title())
    plt.xlabel('Number of processes')
    plt.ylabel('Time spent (ms)')

    plt.plot(datas[cmd]['np'], datas[cmd]['loc'], '--', label='local steps')
    plt.plot(datas[cmd]['np'], datas[cmd]['dst'], '--', label='distributed steps')
    plt.plot(datas[cmd]['np'], datas[cmd]['sum'], '.-', label='total')

    plt.legend()

    # Save the graph
    plt.savefig('%s/png/%s.png' % (SAVE_DIR, name))
    plt.savefig('%s/pgf/%s.pgf' % (SAVE_DIR, name))
