#!/bin/sh

#Usage:
#     license fileName licenseHeaderFile
#

file=$1
header=$2
echo "Licensing file " $file
if grep "Copyright .* The University of New South Wales" $file; then
echo "Already licensed " $file
else
mv $file $file.tmp
cat $header $file.tmp > $file
rm $file.tmp
fi


