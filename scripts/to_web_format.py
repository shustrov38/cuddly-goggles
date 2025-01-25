import sys


def main(file):
    lines = [line.rstrip() for line in file]
    
    n, m = 0, 0
    edges = set()

    for line in lines:
        if line.startswith('e'):
            u, v = list(map(int, line.split()[1:]))
            if u > v:
                u, v = v, u
            edges.add((u, v))

        if line.startswith('p'):
            n, m = list(map(int, line.split()[2:]))
            continue

    for u, v in edges:
        print(f'{u} -- {v}')

if __name__ == '__main__':

    if len(sys.argv) < 2:
        print(f'usage: python {sys.argv[0]} path_to_dimacs')
        sys.exit(1)

    with open(sys.argv[1], 'r') as fin:
        main(fin)