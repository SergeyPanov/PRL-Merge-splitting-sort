#!/bin/bash

#pocet cisel bud zadam nebo 10 :)
if [ $# -lt 1 ];then
    numbers=16;
else
    numbers=$1;
fi;

#preklad cpp zdrojaku
/Users/sergeypanov/bin/mpi/bin/mpic++ -o mss.o mss.cpp


#vyrobeni souboru s random cisly
dd if=/dev/random bs=1 count=$numbers of=numbers

#spusteni
/Users/sergeypanov/bin/mpi/bin/mpirun -np 3 mss.o

#uklid
rm -f oets numbers
