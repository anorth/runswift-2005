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

#include "RInfoParser.h"
#include "Stats.h"
#include <cstring>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>

#define DEFAULT_OBJECT 0
/******
 * RStat uses Stat.cc to calculate median values for Distance samples
 * Opens all rstat files in the directory. PArses the sample data
 * and then produces an output file that summarises the median 
 * data that is specified in the last few lines of the code.
 * Todo:  Produce commnd line itnerface to access each of the RStat fields
 *****/

/****
 * ADD OBJECTINFOTYPE
 * This is the guts of the type of RINFO this code
 * intends to parse.  Add a new OBJECTInfoType if 
 * you are parsing a different sort of RINFO file.
 * The fields must be specified in order
 * Note: A Changes here will probably mean that you may have to add a new
 * case to the marked statements below
 *****/
typedef enum {
  FLAT_DIS,
  PERP_DIS,
  CAMERA_DIS,
  CAMERA_CENTROID_DIS,
  TARGET_PERP_DIS,
  TARGET_FLAT_DIS,
  TARGET_CAMERA_DIS,
  ELEVATION,
  HEADING,
  PAN,
  TILT,
  IMG_ELEVATION,
  IMG_HEADING,
  RADIUS
} BallInfoType;

void usage() {
  cout << "Correct Usage: "
       << "rstat <dirname>" << endl;
}

int main(int argc, char* args[]) {
  char* dirname;
  struct dirent* ent;
  char* filename;
  ofstream out("output");
  int curObjNum=DEFAULT_OBJECT;

  // parse the command line arguments 
  if (argc==1) {
    dirname=".";
  }
  else  if (argc==2) 
    dirname=args[1];
  DIR* dir=opendir(dirname);
  if (chdir(dirname)==-1)
    return 0;

  // Set Object Number
  cout << "Enter Object Number [" << curObjNum <<"]: " << endl;
  char line[1024];
  cin.getline(line, 1024);
  if (!strcmp(line, ""))
    curObjNum=atoi(line);

  // Process all files in the directory
  cout << "Processing:" << endl;
  while ((ent=readdir(dir))!=NULL) {
    filename=ent->d_name;
    if (strcmp(filename,".")==0 || strcmp(filename, "..")==0)
      continue;
     //Create data set from current file
    RInfoParser parser(filename);
    int dim=parser.parseFile();
    DataSet& dataSet=parser.getDataSet();
    //Simple validation Checks
    if (dim<1) 
      continue;
    if (dim!=14)
      continue;

    /***
     * CHANGE THESE STATEMENTS
     * if you want to create a different sort of output file
     * Note: if you change the output, then you must also change the 
     * "calibrate" script
     ***/
    // Now Creates an entry in the output file
    Stats calc(dataSet, dim);
    if (curObjNum) {
      if (calc.median(PAN)>10)
	out << "L: ";
      else if (calc.median(PAN)<-10)
	out << "R: ";
      else 
	out << "C: ";
      out << "perpendicular distance: " << calc.median(PERP_DIS) << endl
	  << "radius:                 " << calc.median(RADIUS) << endl
	  << "height:                 " << calc.median(RADIUS)*2 << endl
	  << "current divisor:         " << calc.median(RADIUS)*calc.median(CAMERA_DIS) << endl
	  << "current divisor*2:       " << calc.median(RADIUS)*calc.median(CAMERA_DIS)*2 << endl
	  << "target divisor:         " << calc.median(RADIUS)*calc.median(TARGET_CAMERA_DIS) << endl
	  << "target divisor*2:       " << calc.median(RADIUS)*calc.median(TARGET_CAMERA_DIS)*2 << endl <<endl;
    }
  }
}
    

