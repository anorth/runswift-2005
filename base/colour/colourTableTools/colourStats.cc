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


/* colourStats.cc
 * Colour table ("nnmc") statistics
 *
 * Processes a colour lookup table and prints statistics
 */

#include <iostream>
#include <fstream>

#define OFFLINE
#include "../../../robot/vision/CorrectedImage.h"

using namespace std;

void usage(const char* progname);
void process(ifstream& in);
void printColourCounts(unsigned char table[MAXY][MAXU][MAXV]);

int main(int argc, char *argv[]) {
    ifstream in;
    
    /* Process args */
    if (argc != 2) {
        usage(argv[0]);
        exit(1);
    }

    /* Open files */
    in.open(argv[1]);
    if (! in.is_open()) {
        cerr << "Failed to open " << argv[1] << ": ";
        perror(0);
        exit(1);
    }

    process(in);
}

/* Processes the colour table (nnmc) open with 'in' and prints 
 * statistics
 */
void process(ifstream& in) {
    unsigned char table[MAXY][MAXU][MAXV];

    /* Read the file into memory */
    in.read((char*)table, sizeof(table));

    /* Process the table */
    printColourCounts(table);

}

/* Counts the number of occurences of each colour and prints the totals */
void printColourCounts(unsigned char table[MAXY][MAXU][MAXV]) {
    // counters for colours. cNONE at counts[NUM_COLOUR].
    int counts[cNUM_COLOR + 1];

    cout << "--- Classification frequency ---" << endl;
    
    for (int i = 0; i < cNUM_COLOR + 1; ++i) {
        counts[i] = 0;
    }

    for (int y = 0; y < MAXY; ++y) {
        for (int u = 0; u < MAXU; ++u) {
            for (int v = 0; v < MAXV; ++v) {
                int col = table[y][u][v];
                if (col < cNUM_COLOR)
                    counts[col]++;
                else if (col == cNONE)
                    counts[cNUM_COLOR]++;
                else
                    cerr << "Unexpected colour " << col << " at ("
                        << y << ", " << u << ", " << v << ")" << endl;
            }
        }
    }

    for (int i = 0; i < cNUM_COLOR; ++i) {
        cout << ColourNames[i] << ": " << counts[i] << " ("
            << (100 * counts[i]/(double)(MAXY*MAXU*MAXV)) << "%)" << endl;
    }
    cout << "no colour: " << counts[cNUM_COLOR] << " ("
            << (100 * counts[cNUM_COLOR]/(double)(MAXY*MAXU*MAXV))
            << "%)" << endl;
}

void usage(const char* progname) {
    cerr << "usage: " << progname << " colourtable" << endl;
}
