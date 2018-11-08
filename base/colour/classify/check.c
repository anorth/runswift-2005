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
#include <stdlib.h>
#include <string.h>

#define COLOUR_CUBE_SIZE 2097152	// 128 * 128 * 128

char *core;

void init_calib(char *filename1) {
  FILE *fp;
  fp = fopen(filename1, "r");
  if (fp == NULL) {
    printf("Core file not found\n");
    exit(1);
  }
  core = (char*)calloc(COLOUR_CUBE_SIZE, sizeof(char));
  fread(core, sizeof(char), COLOUR_CUBE_SIZE, fp);
  fclose(fp);
}

int main(int argc, char *argv[]) {
  int i;
  FILE *fp;
  int count[10];
  if (argc != 2) {
    printf("usage: %s <file>\n", argv[0]);
    exit(1);
  }

  init_calib(argv[1]);

  // init count
  for (i = 0; i < 10; i++) {
    count[i] = 0;
  }

  for (i = 0; i < COLOUR_CUBE_SIZE; i++) {
    count[core[i]]++;
  }
  
  printf("Ball......... %d\n", count[0]);
  printf("Blue......... %d\n", count[1]);
  printf("Green........ %d\n", count[2]);
  printf("Yellow....... %d\n", count[3]);
  printf("Pink......... %d\n", count[4]);
  printf("Blue Dog..... %d\n", count[5]);
  printf("Red Dog...... %d\n", count[6]);
  printf("Green Field.. %d\n", count[7]);
  printf("Background.D. %d\n", count[8]);
  printf("Background.L. %d\n", count[9]);
}
