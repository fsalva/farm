#!/bin/bash
#
# generafile genera i file file100.dat file150.dat file19.dat file116.dat...
# in modo deterministico
#
j=1
for i in 100 150 19 116 2 1 117 3 5 17 4 16 19 8 10 111 12 13 14 15 18 20; do
    ./generafile file$i.dat $(($i*11 + $j*117)) > /dev/null
    j=$(($j+3))
done
mkdir -p generati 
mkdir -p generati/testdir
mkdir -p generati/testdir/testdir2 
mv file111.dat file150.dat generati/testdir/testdir2
mv file19.dat file8.dat generati/testdir
mv file* generati
