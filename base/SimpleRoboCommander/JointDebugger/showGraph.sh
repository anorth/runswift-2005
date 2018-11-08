#!/bin/sh
echo "Showing " $1 " in " $2
cp $2/$1.log plot.dat
gnuplot < plotJoints.plt
kuickshow joints.png
