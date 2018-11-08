#!/bin/sh
#
# Copyright 2003 The University of New South Wales (UNSW) and National  
# ICT Australia (NICTA).
#
# This file is part of the 2003 team rUNSWift RoboCup entry.  You may  
# redistribute it and/or modify it under the terms of the GNU General  
# Public License as published by the Free Software Foundation; either  
# version 2 of the License, or (at your option) any later version as  
# modified below.  As the original licensors, we add the following  
# conditions to that license:
#
# In paragraph 2.b), the phrase "distribute or publish" should be  
# interpreted to include entry into a competition, and hence the source  
# of any derived work entered into a competition must be made available  
# to all parties involved in that competition under the terms of this  
# license.
#
# In addition, if the authors of a derived work publish any conference  
# proceedings, journal articles or other academic papers describing that  
# derived work, then appropriate academic citations to the original work  
# must be included in that publication.
#
# This rUNSWift source is distributed in the hope that it will be useful,  
# but WITHOUT ANY WARRANTY; without even the implied warranty of  
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License along  
# with this source code; if not, write to the Free Software Foundation,  
# Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
#
# Automates the whole calibration process.
#
 
if [ ! -d files ]; then
    mkdir files
fi

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 dirname containing labelled bfl images" >&2
    exit 1
fi

javac Convert.java;
javac DecisionTreeToCodeYUV.java;
g++ -Wall -o dtcompressor dtcompressor.cc;

# Generate the .data file for c4.5
# dtcompressor.c is the same as compressor.c
# except the output format is c4.5-acceptable, 

echo "generating .data file for c4.5 ...";
#java Convert $1 "files/cal.data";
java Convert $1 "files/cal.convert";
./dtcompressor "files/cal.convert" > "files/cal.data";

# Copy across template.names before executing c4.5
cp template.names "files/cal.names";
cd files;
echo "running c4.5 ..."
c4.5 -f "cal" > "cal.dt";
cd ..

# Convert c4.5 output to C++ code
echo "converting c4.5 decision tree to C++ code";
java DecisionTreeToCodeYUV "files/cal.dt" 0 > "files/cal.code";
echo "The C++ code generation is completed.";

echo "Creating lookup table...";
perl insertCode.pl "template.tree2cube.cc" "files/cal.code" tree2cube.cc
g++ -Wall -o tree2cube tree2cube.cc
./tree2cube > files/nnmc.cal
cp -v files/nnmc.cal $1
echo "Lookup table created."
echo " * Copy the colour lookup table across, i.e. cp files/nnmc.cal 
~/trunk/robot/cfg/nnmc.cal";
