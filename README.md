# cuddly-goggles

## Generator
Planar graph generator based on Voronoi diagram with random edge removal. To clarify, it should be said that the algorithm generates a set of random points on a plane and builds a Voronoi Diagram on them. After that, the centers of the resulting Voronoi cells become the vertices of the graph. And edges are drawn between adjacent cells. To form arbitrary faces, random edges are removed from the graph.

### Features
- Can efficienlty generate huge planar graph with `4e5` verties in under a minute.
- Advantage of controlling graph `density` and `connectivity`.
- Calculates important statistics for graph analysis, such `Average vertex degree`, `Edge density`, `Connected components` and the `Face vertex count distribution`.

### Usage
Output of `help` message
```bash
Usage: ./generator/generator [options]
Options:
  -h [ --help ]                   Produce help message
  -N [ --num-vertices ] arg       Number of vertices in graph
  -p [ --remove-prop ] arg (=0.5) Propability of removing edge from graph
  -c [ --connectivity ]           Make graph connected (1 connected component)
  --export-svg arg                Path to SVG result

```

Example of generating of `connected` planar graph with `10` vertices. Generator prints graph in `DIMACS` format for coloring problem.
```bash
$ ./generator -N 10 -c -p 0.75
c SOURCE: Dmitriy Shustrov (shustrov38@gmail.com)
c DESCRIPTION: Planar graph based on Voronoi Diagram with random edges removed.
c 
c STATS: Average vertex degree = 2.80
c STATS: Edge density = 0.3111
c STATS: Face vertex count distribution:
c           3 verts:       3 faces
c           5 verts:       1 face
c           6 verts:       1 face
c           8 verts:       1 face
c STATS: Connected components = 1
c 
p edge 10 14
e 6 3
e 6 4
e 4 7
e 4 3
e 2 3
e 2 5
e 3 9
e 5 8
e 5 1
e 1 8
e 1 9
e 7 10
e 7 9
e 9 10
```