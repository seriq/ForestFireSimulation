#!/bin/bash
g++ -o forest forestfire.cpp
g++ -o clustersumme clustersumme.cpp

theta=30
for(($theta;$theta<50;theta+=10))
do
    for((i=0;i<3;i++))
    do
	./forest $theta 200 500 150000 200000 7
    done
    ./clustersumme $theta 200 500 400 200000 7
done
