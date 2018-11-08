#!/bin/sh

#Usage:
#     indentAll.sh [source directory]
# for current dir, type "indentAll.sh ."
#

echo "Indenting all files in " $1 " dir"

for file in $1/*.cc $1/*.h; do
    echo "Doing " $file
    vim -c ":normal =G" -c ":wq" $file
done

