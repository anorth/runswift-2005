#!/bin/sh

#Usage:
#     licenseAll.sh [source directory] [file extension] 
# for current dir, type "licenseAll.sh . py"
#

directory=$1
extension=$2

echo "Licensing all files in " $directory " dir"

for file in $directory/*.$extension ; do
    echo "Doing " $file;
    license.sh $file /tmp/license.$extension
done

