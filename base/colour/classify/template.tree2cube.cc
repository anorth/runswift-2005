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


#include <stdio.h>
#include <math.h>
#include <string>
#include <fstream>
#include <list>
#include <iostream>
#include "colour_definition.h"

using std::cerr;
using std::endl;
using std::ifstream;
using std::string;

#define CUBE_SIZE 128

int getH(int u, int v) {
    return (int) atan2(u-64, v-64);
}

int getS(int u, int v) {
    return (int) sqrt((u-64)*(u-64) + (v-64)*(v-64));
}    

unsigned char tree(int y, int u, int v) {
  // copy and paste c4.5 output here
    
<insert code here>

}

int main(int argc, char** argv) {
    unsigned char colour;
    //int h, s;
    
    // fill in colour cube
    for (int y = 0; y < CUBE_SIZE; ++y) {
        for (int u = 0; u < CUBE_SIZE; ++u) {
            for (int v = 0; v < CUBE_SIZE; ++v) {

                //h = getH(u, v);
                //s = getS(u, v);

                colour = tree(y, u, v);
                
                printf("%c", colour);
            }
        }
    }
}
