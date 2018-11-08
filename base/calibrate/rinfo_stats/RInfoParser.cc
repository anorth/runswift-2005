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
#include <cstring>
#include <cstdlib>
RInfoParser::RInfoParser(char* filename) {
  in.open(filename);
  fn=filename;
  if (!in.is_open())
    exit(0);
  //  out = new LinkedList();
}

/*
int RInfoParser::parseFile(ObjectInfoType type) {
  char line [1024];
  int  index=-2;
  int j=0;
  while(in.getline(line,1024)) {
    //    cerr << k++ << ": " << line << endl;

    if (strcmp(line, "<RINFO>")==0)
      index=0;
    else if (strcmp(line,"</RINFO>")==0)
      index=-2;
    else if (index == type) {
      double* val=new double[1];
      val[0]=atof(line);
      out.add(new double*(val));
      index++;
      j++;
    }
    else
      index++;
    //    cerr << j << endl;
  }
  //  cerr << j;
  outLength=j;
  return j;
}
*/

int RInfoParser::parseFile() {
  char line [1024];

  int size=-1;
  // find size of vector by counting number of samples
  while(in.getline(line,1024)) {
    if (strcmp(line, "<RINFO>")==0)
      size=0;
    else if (strcmp(line,"</RINFO>")==0)
      break;
    else if (size>=0)
      size++;
  }

  // Detect file errors
  if (size<=0) {
#ifdef DEBUG
    if (size==0)
      cout << "Empty Rinfo" << endl;
    else if (size==-1) 
      cout << "Error Parsing file because there are no RINFO tags" << endl;
#endif
    return -1;
  }

  cout << fn << endl;
  in.seekg(0, ios::beg);
  int j=0;
  double *val;
  int  index=-2;
  while(in.getline(line,1024)) {
    if (strcmp(line, "<RINFO>")==0) {
      index=0;
      val=new double[size];
    }
    else if (strcmp(line,"</RINFO>")==0) {
      out.add(new double*(val));
      j++;
      index=-2;
    }
    else if(index>=0)
      val[index++]=atof(line);
  }
  outLength=j;
  return size;
}
