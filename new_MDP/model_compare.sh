#!/bin/bash
for i in 557
do
    #for j in 100 250 500 750 1000 2500 5000 7500 10000 12500 15000 17500 100000
    for j in 100 250 500 750 1000 2500 5000 7500 10000 25000 50000 75000 100000
    #for j in 12500 15000 17500 100000
    do
        ./run_model.exe naive $j $i
        #./run_model.exe root $j 21
        #./run_model.exe infinite $j 21
        #./run_model.exe tree $j 21
    done
done
