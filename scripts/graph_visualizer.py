import sys

import networkx as nx

import matplotlib.pyplot as plt

G = nx.Graph()

G.add_nodes_from([
(7, {'label': '3 | 4', 'degree': 5}),
(4, {'label': '1 | 7', 'degree': 4}),
(2, {'label': '0 | 9', 'degree': 2}),
(3, {'label': '1 | 8', 'degree': 2}),
(6, {'label': '1 | 2', 'degree': 5}),
(1, {'label': '0 | 10', 'degree': 2}),
(0, {'label': '4 | 6', 'degree': 4}),
(5, {'label': '2 | 3', 'degree': 5}),
(9, {'label': '2 | 5', 'degree': 4}),
(8, {'label': '0 | 1', 'degree': 5}),
])

G.add_edges_from([
(8, 3),
(8, 4),
(8, 9),
(8, 6),
(8, 5),
(9, 0),
(4, 2),
(4, 0),
(4, 9),
(6, 7),
(6, 5),
(6, 9),
(6, 0),
(0, 7),
(5, 7),
(5, 3),
(2, 7),
(1, 7),
(1, 5),
])

label_map = {node: G.nodes[node]['label'] for node in G}

fig, ax = plt.subplots()

nx.draw(G, pos=nx.planar_layout(G), ax=ax, node_size=1000)
nx.draw_networkx_labels(G, pos=nx.planar_layout(G), ax=ax, labels=label_map)

plt.show()
