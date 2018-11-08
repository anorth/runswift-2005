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


/* rlog2bfl.cc
 * Parses rlog files and creates BFL files for each YUV image present.
 * Ring correction is applied.
 * This also serves as an example program for parsing robolink rlog streams
 */

#include <iostream>
#include <fstream>
#include <sstream>

#include "../../robot/share/robolink.h"
#include "../../robot/share/VisionDef.h"
#include "../../robot/vision/CorrectedImage.h"

using namespace std;

static const int SKIP_WIDTH = CPLANE_WIDTH * 6;

// default (Blue 1)
DogColourDistortion dcd = {1.0, 0.0, 1.0, 0.0, 1.0, 0.0};

int main(int argc, char* argv[]) {

    /* Parse command line args */
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " filename.rlog" << endl;
        cerr << "Extracts BFL files from YUV in robolink log files" << endl;
        exit(1);
    }

    /* Open the rlog file */
    ifstream file(argv[1]);
    if (! file.is_open()) {
        perror("error opening rlog file");
        exit(1);
    }

    /* And prepare a file for the BFLs */
    ofstream bfl;

    robolink_header header;

    CorrectedImage::initDCD(dcd);

    /* Read each chunk in the file */
    while (true) {
        // read the header
        file.read((char*)&header, sizeof(header));
        if (! file.good()) { // stop on EOF
            break;
        }

        // skip over non-yuv data
        if (header.data_type != RLNK_YUVPLANE) {
            file.seekg(header.data_len, ios_base::cur);
            continue;
        }

        // open the bfl output file
        ostringstream str;
        str << "YUV" << header.frame_num << ".BFL";
        bfl.open(str.str().c_str(), ifstream::out);
        if (! bfl.is_open()) {
            perror("error opening BFL");
            exit(1);
        }

        uchar raw[CPLANE_WIDTH * CPLANE_HEIGHT * 6];
        uchar corrected[CPLANE_WIDTH * CPLANE_HEIGHT * 6];
        uchar *cy = corrected;
        uchar *cu = corrected + CPLANE_WIDTH;
        uchar *cv = corrected + 2*CPLANE_WIDTH;
        uchar *cy2 = corrected + 3*CPLANE_WIDTH;
        uchar *cy3 = corrected + 4*CPLANE_WIDTH;
        uchar *cy4 = corrected + 5*CPLANE_WIDTH;
        // copy the YUV infomation from the log file to the BFL file
        file.read((char*)raw, sizeof(raw));
        CorrectedImage::setYUV(raw);
        for (int row = 0; row < CPLANE_HEIGHT; ++row) {
            // copy 6 lines of Y, U, V, Y, Y, Y
            for (int col = 0; col < CPLANE_WIDTH; ++col) {
                //cerr << "raw y = " << (int)raw[row * SKIP_WIDTH + col] << " ";
                //cerr << "CI::y = " << (int)CorrectedImage::y(col, row) << endl;
                cy[row * SKIP_WIDTH + col] = CorrectedImage::y(col, row);
                cu[row * SKIP_WIDTH + col] = CorrectedImage::u(col, row);
                cv[row * SKIP_WIDTH + col] = CorrectedImage::v(col, row);
                cy2[row * SKIP_WIDTH + col] = CorrectedImage::y(col, row);
                cy3[row * SKIP_WIDTH + col] = CorrectedImage::y(col, row);
                cy4[row * SKIP_WIDTH + col] = CorrectedImage::y(col, row);
            }
            bfl.write((char*)cy + row * SKIP_WIDTH, CPLANE_WIDTH);
            bfl.write((char*)cu + row * SKIP_WIDTH, CPLANE_WIDTH);
            bfl.write((char*)cv + row * SKIP_WIDTH, CPLANE_WIDTH);
            bfl.write((char*)cy2 + row * SKIP_WIDTH, CPLANE_WIDTH);
            bfl.write((char*)cy3 + row * SKIP_WIDTH, CPLANE_WIDTH);
            bfl.write((char*)cy4 + row * SKIP_WIDTH, CPLANE_WIDTH);
            // add in another line of unclassified colour
            memset(corrected, 127, CPLANE_WIDTH);
            bfl.write((char*)corrected, CPLANE_WIDTH);
        }
        bfl.close();
    }
    file.close();
    return 0;
}
