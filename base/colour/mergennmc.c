/*
   Copyright 2005 The University of New South Wales (UNSW) and National  
   ICT Australia (NICTA).

   This file is part of the 2005 team rUNSWift RoboCup entry.  You may  
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
   should be included in that publication.

   This rUNSWift source is distributed in the hope that it will be useful,  
   but WITHOUT ANY WARRANTY; without even the implied warranty of  
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  
   General Public License for more details.

   You should have received a copy of the GNU General Public License along  
   with this source code; if not, write to the Free Software Foundation,  
   Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 */


/* mergennmc.c
 * Merges the specified loose and tight color classifications into a single
 * lookup table. If the corresponding bytes from the loose and tight files
 * match then that classification is written to the output file, otherwise
 * the classification in the loose file is written but with the maybe bit
 * set. (Usually the tight classification will be cNONE).
 */

#include "stdio.h"

#define MAYBE_BIT 0x10

int main (int argc, char* argv[]) {
    FILE *tight, *loose;
    int i, ret;
    char *loosefile, *tightfile;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s loose.cal tight.cal\n", argv[0]);
        exit(1);
    }

    loosefile = argv[1];
    tightfile = argv[2];

    loose = fopen(loosefile, "r");
    if (loose == NULL) {
        perror("Error opening loose calibration file");
        exit(1);
    }

    tight = fopen(tightfile, "r");
    if (tight == NULL) {
        perror("Error opening tight calibration file");
        fclose(loose);
        exit(1);
    }

    for (i = 0; i < 128*128*128; ++i) { // MAXY*MAXU*MAXV
        unsigned char l, t;
        ret = fread(&l, sizeof(l), 1, loose);
        if (! ret) {
            perror("Error reading loose calibration file");
            break;
        }
        ret = fread(&t, sizeof(t), 1, tight);
        if (! ret) {
            perror("Error reading tight calibration file");
            break;
        }
        putchar((l== t) ? t: l | MAYBE_BIT);
    }
    
    fclose(loose);
    fclose(tight);
    return 0;
}
