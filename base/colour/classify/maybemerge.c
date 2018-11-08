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

#define COLOUR_CUBE_SIZE 2097152	// 128 * 128 * 128

char *core;
char *maybe;
char *result;

void init_calib(char *filename1, char *filename2) {
  FILE *fp;
  fp = fopen(filename1, "r");
  if (fp == NULL) {
    printf("Core file not found\n");
    exit(1);
  }
  core = (char*)calloc(COLOUR_CUBE_SIZE, sizeof(char));
  fread(core, sizeof(char), COLOUR_CUBE_SIZE, fp);
  fclose(fp);

  fp = fopen(filename2, "r");
  if (fp == NULL) {
    printf("Maybe file not found\n");
    exit(1);
  }
  maybe = (char*)calloc(COLOUR_CUBE_SIZE, sizeof(char));
  fread(maybe, sizeof(char), COLOUR_CUBE_SIZE, fp);
  fclose(fp);

  result = (char*)calloc(COLOUR_CUBE_SIZE, sizeof(char));
}

int main(int argc, char *argv[]) {
  int i;
  FILE *fp;
  if (argc <= 3) {
    printf("usage: %s <core> <maybe> <output>\n", argv[0]);
    exit(1);
  }

  init_calib(argv[1], argv[2]);

  int BLUE_DOG = 5;
  int RED_DOG = 6;
  int GREEN_FIELD = 7;
  int DBACKGROUND = 8;
  int LBACKGROUND = 9;
  int MAYBE = 10;
  for (i = 0; i < COLOUR_CUBE_SIZE; i++) {
    if (core[i] == DBACKGROUND || core[i] == LBACKGROUND) 
    {
 /*
      if (maybe[i] == GREEN_FIELD)
        result[i] = GREEN_FIELD;
      else if (maybe[i] < BLUE_DOG)
        result[i] = maybe[i] + MAYBE;
      else if (maybe[i] == BLUE_DOG || maybe[i] == RED_DOG)
        result[i] = maybe[i];
      else
        result[i] = BACKGROUND;
        */
      result[i] = maybe[i];
      
    } 
    else 
    {
      result[i] = core[i];
    }   
  }

  fp = fopen(argv[3], "w");
  if (fp == NULL) {
    printf("Cant create output file\n");
    exit(1);
  }
  fwrite(result, sizeof(char), COLOUR_CUBE_SIZE, fp);
  fclose(fp);
}
