#!/bin/bash
#pocet cisel bud zadam nebo 10 :)
if [ $# -lt 1 ];then
    numbers=10;
else
    numbers=$1;
fi;

procs=$2

#preklad cpp zdrojaku
mpic++ -o mss mss.cpp


#vyrobeni souboru s random cisly
dd if=/dev/random bs=1 count=$numbers of=numbers 2> /dev/null

#spusteni
mpirun -np $procs mss
#uklid
rm -f mss numbers
