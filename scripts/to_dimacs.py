import os
import sys


assert len(sys.argv) > 1
filename = sys.argv[1]
assert os.path.exists(filename)

verts = 0
edges = []

with open(filename, 'r') as fin:
    verts = int(fin.readline())

    for line in fin:
        u, v = map(int, line.rstrip()[1: -1].split(','))
        edges.append((u, v))


print(f'p edge {verts} {len(edges)}')
for u, v in edges:
    print(f'e {u + 1} {v + 1}')
