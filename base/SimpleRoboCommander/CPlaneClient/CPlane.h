/*

   Copyright 2004 The University of New South Wales (UNSW) and National  
   ICT Australia (NICTA).

   This file is part of the 2004 team rUNSWift RoboCup entry.  You may  
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

/*
 * Last modification background information
 * $Id$
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 *
**/



#ifndef _CPLANE_H_
#define _CPLANE_H_

#include <qimage.h>
#include <qtextstream.h>

#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>
#include <cassert>

#include "../../../robot/share/VisionDef.h"

using namespace std;

static const int ERS7_FRAME_WIDTH = 208;
static const int ERS7_FRAME_HEIGHT = 160;

static const QColor colorRed("red");
static const QColor colorBlue("blue");
static const QColor colorOrange("orange");
static const QColor colorGreen("green");
static const QColor colorYellow("yellow");
static const QColor colorPink("pink");
static const QColor colorGrey("grey");
static const QColor colorWhite("white");
static const QColor colorBlack("black");

static const QColor colorTable[] = {
    colorOrange,
    colorBlue,
    colorGreen,
    colorYellow,
    colorPink,
    colorBlue,
    colorRed,
    colorGreen,
    colorGrey,
    colorWhite,
    colorBlack,
    colorBlack
};

static const unsigned char MAYBE_BIT = 0x10;

class CPlane{
    public:
        //WARNING: heuristic , in fact 160*2 < minimumCPlaneSize < 160*204*2
        static const int minimumCPlaneSize = 160;
        static const int maximumCPlaneSize = 60000;

        CPlane(int width = ERS7_FRAME_WIDTH, int height = ERS7_FRAME_HEIGHT){
            this->width = width;
            this->height = height;                
            cplaneBlobArray.clear();
            image = new QImage(width, height,32);
        }
        ~CPlane(){
            delete(image);
        }

        bool parseCPlaneFromStream(istream &ss){
            cplaneBlobArray.clear();

            int u = -1 , v = -1; // uchars?
            int size = -1;
            ss >> size;

            int readSize = 0;
            while (!ss.eof() && readSize < size ){
                u = -1; v = -1;
                ss >> u >> v;
                if (u == -1 || v == -1) break;
                readSize += 2;
                cplaneBlobArray.push_back(make_pair(u,v));
            }
            ss.ignore(100000,'\n');
            return readSize == size;
        }

        bool parseCPlaneFromLine(string line){
            istringstream ss(line);
            return parseCPlaneFromStream(ss);
        }

        bool parseCPlaneFromCompressedPChar(char *buf){
            cplaneBlobArray.clear();

            int compressedLength = *(int *)buf;
            //cout << "Compressed Length = " << compressedLength << "... ";
            buf += 4;

            if (compressedLength > minimumCPlaneSize &&
                    compressedLength < maximumCPlaneSize && 
                    compressedLength % 2 == 0){
                for (int i = 0; i < compressedLength; i+= 2){
                    unsigned char count = (unsigned char)buf[i];
                    unsigned char color = (unsigned char)buf[i+1];
                    //cout << "pair(" << (int)count << "," << (int)color << ")" << endl;
                    cplaneBlobArray.push_back(make_pair(count,color));
                }
            }
            else {
                cout << "Discard , wrong size " << compressedLength << endl;
                return false;
            }
            //cout << cplaneBlobArray.size() << " runs." << endl;
            return true;
        }

        QImage *toImage(){
            int row = 0, col = 0;
            int countsum = 0; // temp for debugging
            vector<pair<int, int> >::iterator itr = cplaneBlobArray.begin();
            //cerr << "toImage: " << cplaneBlobArray.size() << " pairs... ";

            while (itr != cplaneBlobArray.end()) {
                countsum += itr->first;
                ++itr;
            }
            itr = cplaneBlobArray.begin();
            while (itr != cplaneBlobArray.end()) {
                int count = itr->first;
                int color = itr->second;

                //countsum += count;
                //cerr << "pair(" << count << ", " << color << ") ";

                // Ignore maybe-colours
                color &= ~MAYBE_BIT;

                if (count > width) {
                    cerr << "Full width run ";
                }
                // If a run is longer than the remainder of this row, fill the
                // row and jump to the next one.
                while (count > 0 && col + count >= width) {
                    int remainder = width - col;
                    //cerr << "Overflow run" << endl;
                    if (row >= height) {
                        cerr << "Warning(1): CPlane had too many rows" << endl;
                    }
                    for (int i = 0; i < remainder; ++i) {
                        image->setPixel(col + i, row, colorTable[color].rgb());
                    }
                    count -= remainder;
                    col = 0;
                    ++row;
                }

                // Draw the remaining partial row of pixels
                if (count > 0) {
                    if (row >= height) {
                        cerr << "Warning(2): CPlane had too many rows" << endl;
                        return image;
                    }
                    for (int i = 0; i < count; ++i) {
                        image->setPixel(col + i, row, colorTable[color].rgb());
                    }
                    col += count;
                    if (col >= width) {
                        cerr << "Error: more pixels drawn than remain in row"
                            << endl;
                    }
                } else if (count < 0) {
                    cerr << "Error: negative pixel count: " << count
                        << ", color: " << color << endl;
                    return image;
                }
                ++itr;
            }
            //cerr << "done " << row << " rows, " << countsum << " pix" << endl;
            return image;
        }

        QImage *getImage(){
            image = toImage();
            return image;
        }

//  Alexn: this has been moved to CPlaneDisplay, which logs the whole
//  data stream
//        void saveToOpenedFile(QTextStream &outStream){
//            //cout << "Saving " << cplaneBlobArray.size() << endl;
//            outStream << 2 * cplaneBlobArray.size() << " "  ;
//            for (unsigned int i=0;i < cplaneBlobArray.size() ; i++){
//                outStream << cplaneBlobArray[i].first << " "
//                    << cplaneBlobArray[i].second << " " ;
//            }
//            //cout << " -- done saving " << endl;
//            outStream << endl;
//        }

        inline int getWidth() { return width; }
        inline int getHeight() { return height; }
       
    private:
        int width, height;
        vector<pair<int,int> > cplaneBlobArray;
        QImage *image;


};


#endif // _CPLANE_H_
