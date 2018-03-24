#!/bin/bash
#pocet cisel bud zadam nebo 10 :)
if [ $# -lt 1 ];then
    numbers=10;
else
    numbers=$1;
fi;

procs=$2

#preklad cpp zdrojaku
/Users/sergeypanov/bin/mpi/bin/mpic++ -o mss mss.cpp


#vyrobeni souboru s random cisly
dd if=/dev/random bs=1 count=$numbers of=numbers

#spusteni
/Users/sergeypanov/bin/mpi/bin/mpirun -np $procs mss

#uklid
rm -f mss numbers
