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


#include <cstdio>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "CorrectedImage.h"
#include "Robolink.h"

#ifdef OFFLINE
static char *calFile = "../../robot/cfg/nnmc.cal";
#else // OFFLINE
static char *calFile = "/MS/NNMC.CAL";
#endif // OFFLINE


using namespace std;

// CorrectedImage static members
const uchar* CorrectedImage::yuv;
const uchar* CorrectedImage::yplane,
             * CorrectedImage::uplane,
             * CorrectedImage::vplane;
uchar CorrectedImage::cplane[CPLANE_WIDTH * CPLANE_HEIGHT];
uchar CorrectedImage::nnmc[MAXY*MAXU*MAXV];
uchar CorrectedImage::dcdY[DCD_SIZE];
uchar CorrectedImage::dcdU[DCD_SIZE];
uchar CorrectedImage::dcdV[DCD_SIZE];

CorrectedImage::CorrectedImage() {
    loadColourCalibration(calFile);
}

void CorrectedImage::initDCD(const DogColourDistortion &dcd) {
	for (int i = 0; i < DCD_SIZE; i++) {
		double yd = i * dcd.My + dcd.Cy;
		double ud = i * dcd.Mu + dcd.Cu;
		double vd = i * dcd.Mv + dcd.Cv;

		if (yd < 0) {
			yd = 0;
		} else if (yd > 255) {
			yd = 255;
		}
			
		if (ud < 0) {
			ud = 0;
		} else if (ud > 255) {
			ud = 255;
		}

		if (vd < 0) {
			vd = 0;
		} else if (vd > 255) {
			vd = 255;
		}
		
		dcdY[i] = (uchar)(yd + 0.5);
		dcdU[i] = (uchar)(ud + 0.5);
		dcdV[i] = (uchar)(vd + 0.5);
	}
}

/* Loads the specified calibration file(s).  */
void CorrectedImage::loadColourCalibration(const char *filename) {
	int fd, rr;

    if (! filename) {
        filename = calFile;
    }
    
	fd = open(filename, O_RDONLY);

	if (fd < 0) {
		perror("Error opening calibration file");
	} else {
		rr = read(fd, nnmc, MAXY*MAXU*MAXV);
	}
    close(fd);
    cerr << "CorrectedImage nnmc loaded to " << (const void*) nnmc
        << " from " << filename << endl;
}

void CorrectedImage::fillCPlane(bool tight) {
#ifndef PROFILE_ACCESS
    if (tight) {
        for (int row = 0; row < CPLANE_HEIGHT; ++row) {
            for (int col = 0; col < CPLANE_WIDTH; ++col) {
                cplane[row * CPLANE_WIDTH + col] = classifyTight(col, row);
            }
        }
    } else {
        for (int row = 0; row < CPLANE_HEIGHT; ++row) {
            for (int col = 0; col < CPLANE_WIDTH; ++col) {
                cplane[row * CPLANE_WIDTH + col] = classify(col, row);
            }
        }
    }
#endif
}

void CorrectedImage::sendCPlane() {
#ifndef OFFLINE
	uchar* cp = cplane;
    static uchar compressed[COMPRESSED_CPLANE_SIZE];
	int offset = 0;
	int start;  // start of a run

	//Runlength encode cplane
	for (int row = 0; row < CPLANE_HEIGHT; ++row) {
        start = 0;
        for (int col = 0; col < CPLANE_WIDTH; col++, cp++) {
            if (*cp != *(cp - 1) || col == CPLANE_WIDTH - 1) {
                if (col == CPLANE_WIDTH - 1) col++;
                compressed[offset++] = col - start;
                compressed[offset++] = *(cp-1);
                start = col;  // reset start for segment just started.
            }
        }
	}

    Robolink::sendToBase((byte*) compressed, RLNK_CPLANE, offset);
#endif // OFFLINE
}

void CorrectedImage::fillSubCPlane() {
#ifndef PROFILE_ACCESS
    for (int row = 0; row < CPLANE_HEIGHT; row += SUB_CPLANE_SAMPLE) {
        for (int col = 0; col < CPLANE_WIDTH; col += SUB_CPLANE_SAMPLE) {
            cplane[row * CPLANE_WIDTH + col] = classify(col, row);
        }
    }
#endif
}

void CorrectedImage::sendSubCPlane() {
#ifndef OFFLINE
	uchar* cp = cplane;
    uchar cur = cNONE, prev = cNONE;
    static uchar compressed[SUB_CPLANE_SIZE];
	int offset = 0;
	int start;  // start of a run

	//Runlength encode cplane
	for (int row = 0; row < CPLANE_HEIGHT; row += SUB_CPLANE_SAMPLE) {
        start = 0;
        for (int col = 0; col < CPLANE_WIDTH; col += SUB_CPLANE_SAMPLE) {
//            if (col == 0)
//                continue;
            cur = cplane[row * CPLANE_WIDTH + col];
            if (cur != prev || col >= CPLANE_WIDTH - SUB_CPLANE_SAMPLE) {
                if (col >= CPLANE_WIDTH - SUB_CPLANE_SAMPLE)
                    col+= SUB_CPLANE_SAMPLE; // skip to next line
                compressed[offset++] = (col - start)/SUB_CPLANE_SAMPLE;
                compressed[offset++] = prev;
//                if ((col - start) / SUB_CPLANE_SAMPLE > 30
//                        && prev != cFIELD_GREEN) {
//                    cerr << "Run of " << ((col - start)/SUB_CPLANE_SAMPLE)
//                        << " colour "
//                        << (int) prev 
//                        << endl;
//                }
                start = col;  // reset start for segment just started.
            }
            prev = cur;
        }
	}

    Robolink::sendToBase((byte*) compressed, RLNK_SUBCPLANE, offset);
#endif // OFFLINE
}

