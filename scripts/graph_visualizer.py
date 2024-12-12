import sys
from dataclasses import dataclass

import matplotlib.pyplot as plt


@dataclass
class Point:
    x: float
    y: float


@dataclass
class Vertex:
    i: int
    p: Point


arr_x = []
arr_y = []

fig, ax = plt.subplots()

verts = []

for line in sys.stdin:
    if line.startswith('v'):
        _, v, x, y = line.split()
        pt = Point(float(x), float(y))
        vert = Vertex(int(v), pt)
        verts.append(vert)
        arr_x.append(pt.x)
        arr_y.append(pt.y)
    elif line.startswith('e'):
        _, v, u = line.split()
        v = int(v)
        u = int(u)

        x1, y1 = [verts[v].p.x, verts[u].p.x], [verts[v].p.y, verts[u].p.y]
        plt.plot(x1, y1)


ax.scatter(arr_x, arr_y)

marg = plt.margins()[1]
ax.set_ylim(0 - marg, 1 + marg)
ax.set_xlim(0 - marg, 1 + marg)
ax.set_aspect('equal')
ax.axis("off")

plt.show()