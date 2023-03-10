#!/bin/bash

if [ ! -e generafile ];
then
    echo "Compilare generafile, eseguibile mancante!";
    exit 1
fi
if [ ! -e farm ];
then
    echo "Compilare farm, eseguibile mancante!"
    exit 1
fi

#
# il file expected.txt contiene i risultati attesi per i file
# generati nel seguito con il programma generafile
#
cat > expected.txt <<EOF
64834211 file100.dat
103453975 file2.dat
153259244 file1.dat
193031985 testdir/testdir2/file150.dat
258119464 file116.dat
293718900 file3.dat
380867448 file5.dat
518290132 file17.dat
532900090 file117.dat
584164283 file4.dat
748176663 file16.dat
890024017 testdir/file19.dat
985077644 testdir/file8.dat
1146505381 file10.dat
1485251680 file12.dat
1674267050 file13.dat
1878386755 file14.dat
1884778221 testdir/testdir2/file111.dat
2086317503 file15.dat
2322416554 file18.dat
2560452408 file20.dat
EOF

#
# generafile genera i file file100.dat file150.dat file19.dat file116.dat...
# in modo deterministico
#
j=1
for i in 100 150 19 116 2 1 117 3 5 17 4 16 19 8 10 111 12 13 14 15 18 20; do
    ./generafile file$i.dat $(($i*11 + $j*117)) > /dev/null
    j=$(($j+3))
done

mkdir -p testdir
mv file19.dat file8.dat testdir
mkdir -p testdir/testdir2
mv file111.dat file150.dat testdir/testdir2

# Eseguo il programma con un tempo più lento, (ho attivato le stampe a video)
# Lo mando in background ed invio prima 20 segnali USR1 di fila, per vedere
# se il programma li gestisce, poi ne mando 5 ad intervalli più ragionevoli.
#

./farm -n 1 -d testdir -q 1 -t 1000 file* 2>&1 > output.txt  &
pid=$!
sleep 2 
i=0
echo Invio 20 USR1...
for i in {1..20}
do
    kill -USR1 $pid &
done

for i in {1..5}
do
    echo Invio USR1 to Farm... [$i/5]
    kill -USR1 $pid
    sleep 1
done

echo Invio SIGQUIT...
kill -SIGQUIT $pid 

wait $pid

grep -Pvc '\S' output.txt