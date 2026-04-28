#!/bin/bash

OUT="results_tests.txt"
echo "Tests SDA Projet 2" > $OUT
echo "=================" >> $OUT
echo "" >> $OUT

make clean
make all

echo "EXPÉRIENCE 1 : rayon fixe r=0.01, recherches=10000" >> $OUT
echo "--------------------------------------------------" >> $OUT

for N in 10000 100000 1000000
do
    echo "" >> $OUT
    echo "N = $N, Q = 10000, r = 0.01" >> $OUT
    echo "LIST:" >> $OUT
    ./testpdlist $N 10000 0.01 >> $OUT

    echo "" >> $OUT
    echo "BST:" >> $OUT
    ./testpdbst $N 10000 0.01 >> $OUT

    echo "" >> $OUT
    echo "BST2D:" >> $OUT
    ./testpdbst2d $N 10000 0.01 >> $OUT
done

echo "" >> $OUT
echo "EXPÉRIENCE 2 : N fixe=100000, recherches=10000, rayon variable" >> $OUT
echo "--------------------------------------------------------------" >> $OUT

for R in 0.01 0.05 0.1
do
    echo "" >> $OUT
    echo "N = 100000, Q = 10000, r = $R" >> $OUT
    echo "LIST:" >> $OUT
    ./testpdlist 100000 10000 $R >> $OUT

    echo "" >> $OUT
    echo "BST:" >> $OUT
    ./testpdbst 100000 10000 $R >> $OUT

    echo "" >> $OUT
    echo "BST2D:" >> $OUT
    ./testpdbst2d 100000 10000 $R >> $OUT
done

echo "" >> $OUT
echo "Tests terminés." >> $OUT
