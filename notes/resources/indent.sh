#!/bin/sh

echo "Usage: indent.sh [your file]"
echo "Indenting file : " $1 

vim -c ":normal =G" -c ":wq" $1

