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

/*
 * UNSW 2003 Robocup (Ricky)
 *
 * Last modification background information
 * $Id: OfflineVision.cc 1211 2003-05-14 15:37:33Z echung $
 *
 * This program loads vision modules under "robot" directory to analyse 
 * C-Planes under "offvision/frames".
 *
 * Analysis will be written in *.object files in plain text under
 * "offvision/objects" directory, and browsable under OffVision java 
 * program. 
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 * 
**/

#ifndef _Offvision_h_DEFINED
#define _Vision_h_DEFINED

#include <cstdio>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <list>


#include "../../robot/share/intTypes.h"
#include "../../robot/vision/gps.h"
#include "../../robot/vision/VisualCortex.h"
#include "../../robot/vision/Blob.h"
#include "../../robot/vision/CommonSense.h"
#include "../../robot/vision/PyEmbed.h"
#include "../../robot/behaviours/Behaviours.h"
#include "../../robot/share/SwitchBoard.h"
#include "../../robot/share/BallDef.h"

#endif

#ifdef VALGRIND
#include <valgrind/memcheck.h>
#endif // VALGRIND

using namespace std;

VisualCortex *vision;

typedef map<int, list<int> > Table;

static const bool testBallSanity = false;

static const int P_RED_DOG = 1;
static const int P_BLUE_DOG = -1;

static string FRAME_DIRECTORY           = "frames";
static string FRAME_EXTENSION           = "";
static string BLOB_DIRECTORY            = "blobs";
static string BLOB_EXTENSION            = "blob";
static string OBJECT_DIRECTORY          = "objects";
static string OBJECT_EXTENSION          = "object";
static const string GPS_DIRECTORY       = "gps";
static const string GPS_EXTENSION       = "gps";

static string GENERATED_FOLDER     = "sanityLearning";
static string BALL_NAME            = "ball";
static string BLUE_GOAL_NAME       = "blueGoal";
static string YELLOW_GOAL_NAME     = "yellowGoal";
static string BLUE_TOP_PINK_NAME   = "blueTopPink";
static string PINK_TOP_BLUE_NAME   = "pinkTopBlue";
static string GREEN_TOP_PINK_NAME  = "greenTopPink";
static string PINK_TOP_GREEN_NAME  = "pinkTopGreen";
static string YELLOW_TOP_PINK_NAME = "yellowTopPink";
static string PINK_TOP_YELLOW_NAME = "pinkTopYellow";
static string RED_DOG_NAME         = "redDog";
static string BLUE_DOG_NAME        = "blueDog";

static const string C4_5_NAMES_EXTENSION = "names";
static const string GPS_TAG = "GPS";
static const int INT_SIZE = 4;

static const char *CALIBRATION_FILE = "../../robot/cfg/nnmc.cal";

// read in the calibration file and store in a map 
// (colour -> list of position in which the colour is stored)
void readCalibrationFile(Table& table) {

    /* File name hardcoded inside "robot" directory. */
    ifstream calibrationFile(CALIBRATION_FILE);
    if (!calibrationFile) {
        cout << "Error: cannot open calibration file" << CALIBRATION_FILE << endl;
        exit(1);
    }
    char c;
    int colour;
    int i=0;
    Table::iterator it;
    while (calibrationFile.get(c)) {
        colour = (int) c;
        table[colour].push_back(i);
        i++;
    }
    calibrationFile.close();
}

// print the table, storing the colour, in which it is stores in this format
// colour -> list of coordinate in which the colour occurs in the calibration file
void printCalibrationTable(const Table& table) {
    for (Table::const_iterator it=table.begin(); it!=table.end(); it++) {
        cout << it->first << ": ";
        for (list<int>::const_iterator it2=(it->second).begin(); it2!=(it->second).end(); it2++) {
            cout << *it2 << ", ";
        }
        cout << endl;
    }
}

// print the first entry for each cplane classification
// colour -> first yuv num
void printOneEntry(const Table& table) {
    for (Table::const_iterator it=table.begin(); it!=table.end(); it++) {
        cout << it->first << ": ";
		cout << *((it->second).begin()) << endl;
    }
}

// return yuv coordinate for given cplane colour
void getYUV(const Table& table, int cColour, uchar& y, uchar& u, uchar& v) {
	
	// number representing yuv coordinate
	int yuvNum;
	for(Table::const_iterator it=table.begin(); it!=table.end(); it++) {
        if(it->first == cColour) {
			yuvNum = (*(it->second.begin()));
			break;
		}
    }
	
	v = (uchar)((yuvNum%128)*2);
	u = (uchar)(((yuvNum/128)%128)*2);
	y = (uchar)(((yuvNum/(128*128))%128)*2);
	
}

// write a yuv array out to a bfl file to see in mc
// assumes yuv is of length PIXELS*3
void writeYUV(uchar *yuv, const char *file) {

	ofstream outfile;
	outfile.open(file, ios::out | ios::binary);
	for(int i=0 ; i<HEIGHT ; i++) {
		outfile.write((const char*)yuv+3*i*WIDTH,3*WIDTH);
		uchar fill = (uchar)8;
		for(int j=0 ; j<WIDTH ; j++)
			outfile.write((const char*)(&fill),sizeof(fill));
	}
	outfile.close();

}

// writes a single colour from cplane to a yuv trio and
// then to a file in bfl format
void writeSingleColour(Table& table, int col, const char *file) {

	uchar y, u, v;
	getYUV(table, col, y, u, v);
	ofstream outfile;
	outfile.open(file, ios::out | ios::binary);
	for(int i=0 ; i<HEIGHT ; i++) {
		for(int j=0 ; j<WIDTH ; j++)
			outfile.write((const char*)(&y),sizeof(y));
		for(int j=0 ; j<WIDTH ; j++)
			outfile.write((const char*)(&u),sizeof(u));
		for(int j=0 ; j<WIDTH ; j++)
			outfile.write((const char*)(&v),sizeof(v));
		uchar fill = (uchar)8;
		for(int j=0 ; j<WIDTH ; j++)
			outfile.write((const char*)(&fill),sizeof(fill));
	}
	outfile.close();

}

void rotate(int ox, int oy, int oh, int ow, int &xRmin, int &xRmax, int &yRmin, int &yRmax) {
    int xOrigin = WIDTH/2;
    int yOrigin = HEIGHT/2;

    int nX, nY, eX, eY, sX, sY, wX, wY;
    int rnX, rnY, reX, reY, rsX, rsY, rwX, rwY;

    nX = xOrigin - (int)(ox + ow/2);
    nY = yOrigin - (int)(oy);

    rnX = (int) (vision->ucos_eroll * nX - vision->usin_eroll * nY);
    rnY = (int) (vision->usin_eroll * nX + vision->ucos_eroll * nY);

    sX = xOrigin - (int)(ox + ow/2);
    sY = yOrigin - (int)(oy + oh);

    rsX = (int) (vision->ucos_eroll * sX - vision->usin_eroll * sY);
    rsY = (int) (vision->usin_eroll * sX + vision->ucos_eroll * sY);

    eX = xOrigin - (int)(ox);
    eY = yOrigin - (int)(oy + oh/2);

    reX = (int) (vision->ucos_eroll * eX - vision->usin_eroll * eY);
    reY = (int) (vision->usin_eroll * eX + vision->ucos_eroll * eY);

    wX = xOrigin - (int)(ox + ow);
    wY = yOrigin - (int)(oy + oh/2);

    rwX = (int) (vision->ucos_eroll * wX - vision->usin_eroll * wY);
    rwY = (int) (vision->usin_eroll * wX + vision->ucos_eroll * wY);

    xRmin = xOrigin - MAX (MAX (MAX (rnX, rsX), reX), rwX);
    xRmax = xOrigin - MIN (MIN (MIN (rnX, rsX), reX), rwX);
    yRmin = yOrigin - MAX (MAX (MAX (rnY, rsY), reY), rwY);
    yRmax = yOrigin - MIN (MIN (MIN (rnY, rsY), reY), rwY);
}

void writeGPS(GPS& gps, const char *gpsFile) {

	static const int TEAMMEMBERS_BEING_SENT = (NUM_TEAM_MEMBER-1);
    static const int NUM_ATTR_SENT_ROB = 9;
	static const int NUM_ATTR_VIS_PULL = 3;
    static const int NUM_ATTR_SENT_TEAMMATE = 5;
    static const int NUM_ATTR_SENT_BALL = 9;    //for now x y vx vy varx vary varvx varvy
    static const int NUM_ATTR_SENT_WBALL = 3;	// source robot num, x & y of wireless ball position
	static const int NUM_ATTR_SENT_OPPONENTS = 5;

    const int  arraySize =
        1 +
		
		2 +                     // for the player number and the team number

        1 +                     // for the view variable describing things we can see

        3 +                     // for the last direct update values

        1 +                     // for the size of the remaining vision model data
        NUM_ATTR_SENT_ROB +     // the new world model data (x y h varx vary varh varxy varxh varyh)
        NUM_ATTR_VIS_PULL + 	// the exponentially decaying visual pull
		NUM_ATTR_SENT_BALL +
		NUM_ATTR_SENT_WBALL +   // for the wireless ball
        TEAMMEMBERS_BEING_SENT * NUM_ATTR_SENT_TEAMMATE +

       // 1 +                   // for the size of the remaining world model data
       // NUM_ATTR_SENT_ROB +   // the vision data (x y h varx vary varh varxy varxh varyh)
       // NUM_ATTR_SENT_BALL +

        1 +                     // for the size of the remaining wireless model data
        NUM_ATTR_SENT_ROB +     // the old world model data (x y h varx vary varh varxy varxh varyh)

        1 +                     // for size of opponents data
        NUM_OPPONENTS*NUM_ATTR_SENT_OPPONENTS; // for opponents


    float data[arraySize];
    int ptr = 0;
	
	data[ptr] = arraySize;
	ptr++;

    //set player number
    data[ptr] = Behaviours::PLAYER_NUMBER;
    ptr++;

    // set direction team color
    if (gps.targetGoal == vobBlueGoal) {
        data[ptr] = P_RED_DOG;
    } else {
      data[ptr] = P_BLUE_DOG;
    }
    ptr++;

    // what we can see
    data[ptr++] = (double)gps.view;
        
    data[ptr++] = gps.directx;
    data[ptr++] = gps.directy;
    data[ptr++] = gps.directh;

    /////////////////////
    // NEW WORLD MODEL
    /////////////////////
    //cout << "*************** got to world model sending *********" << endl;


    data[ptr] = NUM_ATTR_SENT_ROB + NUM_ATTR_VIS_PULL + NUM_ATTR_SENT_BALL + NUM_ATTR_SENT_WBALL +
			(TEAMMEMBERS_BEING_SENT * NUM_ATTR_SENT_TEAMMATE);
    ptr++;

	const WMObj& self = gps.self();
	const MMatrix3 scov = gps.selfCov();
	
	data[ptr]     = self.pos.x;
    data[ptr + 1] = self.pos.y;
    data[ptr + 2] = self.h;
    data[ptr + 3] = scov(0,0); //varx
    data[ptr + 4] = scov(1,1); //vary
    data[ptr + 5] = scov(2,2); //varh
    data[ptr + 6] = scov(0,1); //varxy
    data[ptr + 7] = scov(0,2); // varxh
    data[ptr + 8] = scov(1,2); // varyh
    ptr += NUM_ATTR_SENT_ROB;

	const MVec3& vp = gps.getVisualPull();
	data[ptr]     = vp(0,0);
	data[ptr + 1] = vp(1,0);
	data[ptr + 2] = vp(2,0);
	ptr += NUM_ATTR_VIS_PULL;

	//WMObj& ball = gps.ball('g');
	const Vector& ball = gps.getBall(GLOBAL);
	//WMObj& vball = gps.vBall();9c
	const Vector& vball = gps.getVBall(GLOBAL);
	const MMatrix2& bcov = gps.getBallCov();
	const MMatrix2& bvcov = gps.getVelCov();
	//const MMatrix4& bcov = gps.bCov();
    data[ptr]     = ball.x; //ball.pos.x;
    data[ptr + 1] = ball.y; //ball.pos.y;
    data[ptr + 2] = vball.x; //vball.pos.x;
    data[ptr + 3] = vball.y; //vball.pos.y;
    data[ptr + 4] = bcov(0,0); //varx
    data[ptr + 5] = bcov(1,1); //vary
    data[ptr + 6] = bcov(0,1); //varxy
    data[ptr + 7] = bvcov(0,0); //varvx
    data[ptr + 8] = bvcov(1,1); //varvy
    ptr += NUM_ATTR_SENT_BALL;

	if(gps.shareBallvar < 10000000) { // LARGE_VAL
		// wireless ball data is valid
		data[ptr] = gps.sbRobotNum;
		data[ptr+1] = gps.shareBallx;
		data[ptr+2] = gps.shareBally;
	} else {
		// wireless ball data is invalid
		data[ptr] = 0;
		data[ptr+1] = 0;
		data[ptr+2] = 0;
	}
	ptr += NUM_ATTR_SENT_WBALL;

    for (int i=0; i<NUM_TEAM_MEMBER; i++) {
        if (i!=(Behaviours::PLAYER_NUMBER-1)) {
			const WMObj& tm = gps.tmObj(i);
            data[ptr]     = tm.pos.x;
            data[ptr + 1] = tm.pos.y;
            data[ptr + 2] = tm.h;
            data[ptr + 3] = tm.posVar;
            data[ptr + 4] = tm.hVar;
            ptr += NUM_ATTR_SENT_TEAMMATE;
        }
    }

    /////////////////////
    // OLD WORLD MODEL
    /////////////////////

    data[ptr] = NUM_ATTR_SENT_ROB; //number of attributes being sent over
    ptr++;

    data[ptr]     = gps.oldx;
    data[ptr + 1] = gps.oldy;
    data[ptr + 2] = gps.oldh;
    data[ptr + 3] = gps.oldvx; //varx
    data[ptr + 4] = gps.oldvy; //vary
    data[ptr + 5] = gps.oldvh; //varh
    data[ptr + 6] = gps.oldcxy; //varxy
    data[ptr + 7] = gps.oldcxh; //varxh
    data[ptr + 8] = gps.oldcyh; //varyh
    ptr += NUM_ATTR_SENT_ROB;


    /////////////////////
    // OPPONENTS
    /////////////////////
    data[ptr] = NUM_OPPONENTS*NUM_ATTR_SENT_OPPONENTS;
    ptr++;
    for(int i=0 ; i<NUM_OPPONENTS ; i++) {
        const Vector v = gps.getOppPos(i,GLOBAL);
        const MMatrix2& m = gps.getOppCov(i);
        data[ptr] = v.x;
        data[ptr+1] = v.y;
        data[ptr+2] = m(0,0);
        data[ptr+3] = m(1,1);
        data[ptr+4] = m(0,1);
        ptr += NUM_ATTR_SENT_OPPONENTS;
    }
	
	ofstream outfile;
    /*outfile.open(gpsFile, ios::out | ios::binary);
    if (!outfile) {
        cout << "OfflineVision.cc: cannot write to file " << gpsFile << endl;
        exit(1);
    }
	outfile.write((const char*)data,sizeof(data));*/
	outfile.open(gpsFile, ios::out);
	for(int i=0 ; i<arraySize ; i++)
		outfile << data[i] << " ";
	outfile << endl;
	outfile.close();
	
	/*ifstream infile;
	infile.open(gpsFile, ios::in | ios::binary);
	float arr2[arraySize];
	infile.read((char*)arr2,sizeof(arr2));
	infile.close();
	cout << "NOW IS " << arr2[0] << endl;*/

}


/* Load objects from robot directory to analyse the offline c-planes.
 * Results will be written in text files (*.object) and be browsed using Java
 * program OffVision.
 */
int main(int argc, char **argv) {

    /* This flag is set by the Makefile. */
    #ifdef PRE_SANITY_BLOB
        if (argc!=10) {
            cout << "Usage: OfflineVision nameListingFile generatedFolder ";
            cout << "frameDirectory frameExtension blobDirectory blobExtension objectDirectory objectExtension ";
            cout << "ballName" << endl;
            exit(1);
        }
        // generatedFolder is the directory which stores all other generated files and directories
        GENERATED_FOLDER = string(argv[2]);
        FRAME_DIRECTORY  = GENERATED_FOLDER + string("/") + string(argv[3]);
        FRAME_EXTENSION  = string(argv[4]);
        OBJECT_DIRECTORY = GENERATED_FOLDER + string("/") + string(argv[5]);
        OBJECT_EXTENSION = string(argv[6]);
        BLOB_DIRECTORY   = GENERATED_FOLDER + string("/") + string(argv[7]);
        BLOB_EXTENSION   = string(argv[8]);
        BALL_NAME        = string(argv[9]);
    #endif

    // create colour calibration information
	Table calTable;
	readCalibrationFile(calTable);
	
    // construct the dummy vision object
    vision = new VisualCortex();

    GPS gps;
    //gps.SetGoals(true); // red robot
	gps.SetGoals(false); // blue robot
	
	CommonSense sensors;
	PWalkInfoStruct PWalkInfo;

	// Do work that Vision constructor does
	Behaviours::InitBehaviours();
	Behaviours::sensors      = &sensors;
	Behaviours::vision       = vision;
    Behaviours::gps          = &gps;

    PyBehaviours::sensors = &sensors;
    PyBehaviours::vision = vision;
    PyBehaviours::gps = &gps;
    PyBehaviours::PWalkInfo = &PWalkInfo;

	/*ofstream outfile;
    outfile.open("temp");
    if (!outfile) {
        cout << "OfflineVision.cc: cannot write to file objects/filename.object" << endl;
        return 1;
    }
	vision->outfile = &outfile;

	uchar y, u, v;
	getYUV(calTable, 0, y, u, v);
	uchar yuv[3*PIXELS];
	for(int i=0 ; i<PIXELS ; i++) {
		int ind = i%PIXELS;
		int row = i/PIXELS;
		yuv[3*row+ind]=y;
		yuv[3*row+PIXELS+ind]=u;
		yuv[3*row+2*PIXELS+ind]=v;
	}

	long s0 = 0, s1 =0, s2 =0;
	vision->ProcessImage(yuv, s0, s1, s2, 0, gps.self());
		
	gps.GPSMotionUpdate(0,0,0);

	gps.GPSVisionUpdate(vision->vob, vision->edges, RAD2DEG(vision->effective_pan));

	Behaviours::NextAction();
	return 0;*/
    
	string string_total;
    string filename;

    cout << "====================== Finished initialising vision! " << endl;
    
    #ifdef PRE_SANITY_BLOB
        ofstream ballName;

        // if testing is true, then the coordinate of the recognised ball, using the new sanity 
        // checks from c4.5, will be written to blobOutput,
        // no c4.5 names data will be generated
        // otherwise, the normal sanity checks in VisualCortex.cc will be used
        vision->testBallSanity = testBallSanity;
        
        if (!testBallSanity) {
            // the c4.5 names file for the machine learning
            // ball
            string ballFileName = GENERATED_FOLDER + string("/") + BALL_NAME + string("/") + 
                                    BALL_NAME + string(".") + C4_5_NAMES_EXTENSION;
            ballName.open(ballFileName.c_str());
            if (!ballName) {
                cout << "OfflineVision.cc: cannot write to file " << ballFileName << endl;
                return 1;
            }
            vision->ballName = &ballName;
        }
    
    #endif



    #ifdef PRE_SANITY_BLOB
        // read filename from file instead of from standard input
        ifstream nameListFile(argv[1]);
        getline(nameListFile, filename);
    #else

        cin >> filename;
    #endif



    /* Go through every file in the name list.
     * The last line of the file name list will have "quit" on it. 
     * */
    while (filename.compare(string("quit"))) {

        /* Read in cplane. */
        ifstream infile;
        string frameFileName;
        if (FRAME_EXTENSION.length()>0) {
            frameFileName = FRAME_DIRECTORY + string("/") + filename + string(".") + FRAME_EXTENSION;
        } else {
            frameFileName = FRAME_DIRECTORY + string("/") + filename;
        }
        infile.open(frameFileName.c_str());
        if (!infile) {
            cout << "OfflineVision.cc: Error opening file " << frameFileName << endl;
            return 1;
        } 

        /* Where all the analysis of C-plane will be stored. */
        string objectFileName = OBJECT_DIRECTORY + string("/") + filename + string(".") + OBJECT_EXTENSION;



        #ifdef PRE_SANITY_BLOB
            ofstream blobOutput, ballObject;
            string blobFileName = BLOB_DIRECTORY + string("/") + filename + string(".") + BLOB_EXTENSION;
            if (testBallSanity) {
                ballObject.open(objectFileName.c_str());
                if (!ballObject) {
                    cout << "OfflineVision.cc: cannot write to file " << objectFileName << endl;
                    return 1;
                }
            } else {
                blobOutput.open(blobFileName.c_str());
                if (!blobOutput) {
                    cout << "OfflineVision.cc: cannot write to file " << blobFileName << endl;
                    return 1;
                }
            }
        #else
            ofstream outfile;
            outfile.open(objectFileName.c_str());
            if (!outfile) {
                cout << "OfflineVision.cc: cannot write to file " << objectFileName << endl;
                return 1;
            }
        #endif

        outfile << "-----------------------------------------------------" << endl;
        outfile << "--- WRITING FILE OF " << objectFileName << endl;
        outfile << "-----------------------------------------------------" << endl;


	    getline(infile, string_total);
            infile.close(); // close the input file as early as possible
	    istringstream stream_total(string_total);    

	    int total;
	    stream_total >> total;
        
	    uchar yuv[3*PIXELS];

        /* Construct YUV plane from C-Plane, so that we can feed it to vision
         * module. */
	    uchar y, u, v;
	    int k = 0;
        for (int i = 0; i < total/2; i++) {
            int runlength, color;
            stream_total >> runlength;
            stream_total >> color;
            getYUV(calTable, color, y, u, v);
            for (int j = 0; j < (runlength & 0xff); j++) {
			    int line = k/WIDTH;
			    int num = k%WIDTH;
			    yuv[3*line*WIDTH + num] = y;
			    yuv[3*line*WIDTH + WIDTH + num] = u;
			    yuv[3*line*WIDTH + 2*WIDTH + num] = v;
			    k++;
            }
        }

        /* Checking to see if the cplane is complete. */
	    if(k < PIXELS) {
		    cout << "Error: CPlane in file " << frameFileName << " not correct size. Read " << k << "/" << PIXELS << endl;
            #ifdef PRE_SANITY_BLOB
                if (testBallSanity) {
    	            ballObject.close();
                } else {
                    blobOutput.close();
                }
                getline(nameListFile, filename);
            #else
                exit(1);
            #endif
	    }

	    long s0, s1, s2;
	    stream_total >> s0;
	    stream_total >> s1;
	    stream_total >> s2;
        
        #ifdef PRE_SANITY_BLOB
	        vision->blobOutput = &blobOutput;
        #else
            /* Most of the printed information is from vision module. */
            vision->outfile = &outfile;
        #endif

        /* Feed our YUV-plane (built from C-plane) to vision module. */ 
	    vision->ProcessImage(yuv, s0, s1, s2, 0);
        
        // skip to GPS section
        string tag = "";
        while(!stream_total.eof() && tag != GPS_TAG) {
            stream_total >> tag;
        }
        // read in GPS data
        unsigned char gpsinfo[GPS_SIZE];
        for (int i = 0; i < (GPS_SIZE/INT_SIZE); i++) {
            int entry;
            stream_total >> entry;
            /*gpsinfo[i * INT_SIZE] = (entry >> 24) & 0xff;
            gpsinfo[i * INT_SIZE + 1] = (entry >> 16) & 0xff;
            gpsinfo[i * INT_SIZE + 2] = (entry >> 8) & 0xff;
            gpsinfo[i * INT_SIZE + 3] = entry & 0xff;*/
			gpsinfo[i * INT_SIZE] = entry & 0xff;
            gpsinfo[i * INT_SIZE + 1] = (entry >> 8) & 0xff;
            gpsinfo[i * INT_SIZE + 2] = (entry >> 16) & 0xff;
            gpsinfo[i * INT_SIZE + 3] = (entry >> 24) & 0xff;
        }
        gps.restoreGps(gpsinfo, 0);
		
		writeGPS(gps, (GPS_DIRECTORY + "/" + filename + "." + GPS_EXTENSION).c_str());

		Behaviours::NextAction();

        #ifdef PRE_SANITY_BLOB

            if (testBallSanity) {
	            // recognised objects
	            for (int i = vobBall; i < vobRedDog; i++) {
                    //cout << "doing " << i << endl;
        	        VisualObject &obj = vision->vob[i];
        	        if (obj.cf > 0) {
                        if (i == vobBall) {
                	        ballObject << "o " << i << " " << obj.x << " " << obj.y << " " << obj.height << " " 
                            << obj.width << " " << obj.d << " " << obj.h << " " << vision->ballMethod << endl;
        	            }
        	        }
	            }
    	        ballObject.close();
            } else {
                blobOutput.close();
            }
            getline(nameListFile, filename);

        #else

            /* Go through recognised balls, goals and beacons (indices in 
             * VisualCortex.h) */
            for (int i = vobBall; i < vobRedDog; i++) {

                //cout << "doing " << i << endl;
                VisualObject &obj = vision->vob[i];
                if (obj.cf > 0) {

                    /* Ball */
                    if (i == vobBall) {

                        // Added by Ted Wong. Better to draw unmodifed boundary box.
                        outfile << "o " << i << " " << obj.ox << " " << obj.oy << " " << obj.oheight 
                            << " " << obj.owidth << " " << obj.d << " " <<
                            obj.h << " " << vision->ballDistMethod << endl;

                    /* Goals */
                    } else if (i == vobBlueGoal || i == vobYellowGoal) {


                        #define unrotate_goals_about_center_so_assume_given_xy_are_rotated 
                        #ifdef unrotate_goals_about_center_so_assume_given_xy_are_rotated 
                        /*---------------------------------*/

                        int xRmin, yRmin, xRmax, yRmax;
                        /* Unrotate about center using ucos_eroll and usin_eroll. */
                        rotate((int)(obj.x), (int)(obj.y), (int)(obj.height), (int)(obj.width), 
                                     xRmin, xRmax, yRmin, yRmax);

                        outfile << "o " << i << " " << xRmin << " " << yRmin << " " << (yRmax - yRmin) << " " 
                            << (xRmax - xRmin) << " " << obj.d << " " << obj.h << endl;


                        #else
                        /*---------------------------------*/

                        outfile << "o " << i << " " << (int) obj.x << " " << (int) obj.y 
                            << " " << (int) obj.height 
                            << " " << (int) obj.width 
                            << " " << obj.d << " " << obj.h << endl;

                        #endif // unrotate_goals_about_center_so_assume_given_xy_are_rotated 

                    /* Beacons */
                    } else {
                        outfile << "o " << i << " " << obj.x << " " << obj.y << " " << obj.height << " " 
                            << obj.width << " " << obj.dist << " " << obj.h << endl;
                    }
                }
            }

            /* Go through recognised dogs. */
            for (int i = vobRedDog; i <= vobBlueDog4; i++) {
                VisualObject &obj = vision->vob[i];
                if (obj.cf > 0) {
                    int xRmin, yRmin, xRmax, yRmax;

                    /*
                    rotate((int)(obj.x), (int)(obj.y), (int)(obj.height), (int)(obj.width), xRmin, xRmax, yRmin, yRmax);
                    outfile << "o " << i << " " << xRmin << " " << yRmin << " " << (yRmax - yRmin) << " " 
                        << (xRmax - xRmin) << " " << obj.d << " " << obj.h << endl;
                        */

                    outfile << "o " << i << " " << (int) obj.x << " " << (int) obj.y 
                        << " " << (int) obj.height 
                        << " " << (int) obj.width 
                        << " " << obj.d << " " << obj.h << endl;
                }
            }
            outfile.close();
            cerr << "done file " << filename << endl;
            cin >> filename;

        #endif
    }

    #ifdef PRE_SANITY_BLOB
        nameListFile.close();
        ballName.close();
    #endif

    delete vision;
    return 0;
}

