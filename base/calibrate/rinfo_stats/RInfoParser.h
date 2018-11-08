/*

Copyright 2003 The University of New South Wales (UNSW) and National  
ICT Australia (NICTA).

This file is part of the 2003 team rUNSWift RoboCup entry.  You may  
redistribute it and/or modify it under the terms of the GNU General  
Public License as published by the Free Software Foundation; either  
version 2 of the License, or (at your option) any later version as  
modified below.  As the original licensors, we add the following  
conditions to that license:

In paragraph 2.b), the phrase "distribute or publish" should be  
interpreted to include entry into a competition, and hence the source  
of any derived work entered into a competition must be made available  
to all parties involved in that competition under the terms of this  
license.

In addition, if the authors of a derived work publish any conference  
proceedings, journal articles or other academic papers describing that  
derived work, then appropriate academic citations to the original work  
must be included in that publication.

This rUNSWift source is distributed in the hope that it will be useful,  
but WITHOUT ANY WARRANTY; without even the implied warranty of  
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  
General Public License for more details.

You should have received a copy of the GNU General Public License along  
with this source code; if not, write to the Free Software Foundation,  
Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#include <iostream>
#include <fstream>
#include "ll.h"
#include "Stats.h"

/*******************************************
 * RInfoParser.h
 * Name: Andres Olave
 * Date: 16/9/01
 * Parses a RINFO file.
 * An RINFO file is one created by a wireless
 * base station program communicating with a aibo
 * robot dog.  In fact this RInfo parser will ONLY
 * parse a file created by a dog running
 * the BallDistanceCalibrator code.  Please do not use
 * for any other RINFO file unless you intend to modify 
 * this file and ofcourse save it under a differenet filename
 *
 * Class Declaration
 *   Stats
 ******************************************/

class RInfoParser {
  ifstream in;
  DataSet out;
  int outLength;
  char* fn;
 public:
  RInfoParser(char*);
  /*Parses the file for this particular information field*/
  //  int parseFile(ObjectInfoType type);
  /*Parses the file and creates a data set of the rinfo fields*/
  int parseFile();
  // Returns data set equiuvalents of the rinfo file
  DataSet& getDataSet();
};

inline DataSet& RInfoParser::getDataSet() {
  return out;
}
