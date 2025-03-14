#!/bin/bash
if [ "$2" = "prefetch" ]; then
bunzip2 -kc ../traces/$1.bz2 | ./cache --icache=128:2:128:2 --dcache=64:4:128:2 --l2cache=128:8:128:50 --memspeed=100 --prefetch
else
bunzip2 -kc ../traces/$1.bz2 | ./cache --icache=128:2:128:2 --dcache=64:4:128:2 --l2cache=128:8:128:50 --memspeed=100 
fi
