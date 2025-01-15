#!/bin/bash

# quickly find dataset with expected vertices count and bad chromatic number from dsatur

for i in `seq 1 10000`; do 
    echo Running $i
    ./generator/generator -N $1 -c -p 0.5 > ./logs/log$i
    ./solver/solver -i ./logs/log$i -c dsatur_fibonacci_heap > ./logs/out

    result=`tail -n 1 ./logs/out`
    if [[ "$result" == "Found coloring N=5" ]]; then
        break
    fi
done
