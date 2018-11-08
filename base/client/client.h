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
 * UNSW 2003 Robocup (Alex Tang)
 *
 * Last modification background information
 * $Id: client.h 1964 2003-08-26 15:26:27Z ttam186 $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 *
 * intermediate between basestation and the Wireless Aperios object on the dog
 * 
 * The overall communication step starting from the robots
 * 1. robots sends data (eg. cplane, debug, etc) to the Wireless Aperios object
 * 2. Wireless Aperios objects sends data to here
 * 3. This then sends the data to the basestation.
 *
**/

#ifndef _client_h_DEFINED
#define _client_h_DEFINED

#include <stdio.h>
#include <cstdlib>
#include <unistd.h>
#include <errno.h>
#include <string>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <pthread.h>

#include "share/PackageDef.h"
#include "ClientConsts.h"
#include "share/SwitchBoard.h"
#include "ClientSwitchBoard.h"


#define MSGHEADER ""

// the maximum size of the buffer to store the command packet. This is just for the command like (mode, 0), (p, 5), and has nothing to do with robot sharing, YUV paint, etc
static const int COMMAND_SEND_BUFFER_MAX_SIZE = sizeof(PackageDef) + sizeof(WDataInfo);

static const char *PLAYER_CONFIG_FILE = "conf.cfg";

// constants used for the cplane log and versioning
static const int VERSION_SIZE            = 30;
static char CPLANE_VERSION[VERSION_SIZE] = "(empty)";
static const char *CPLANE_VERSION_FILE   = "../RoboCommander/CPLANE_VERSION.txt";
static const char *CPLANE_LOG            = "../RoboCommander/CPLANE_LOG";

/*
 * The roll is multiplied by this factor so that only 4 bytes is used in the transmission, instead of 8.
 * has to agree among the following files: robot/vision/Vision.cc, client.h and RoboConstant.java
**/
static const double SEND_SCALE = 1000.0;

// for joystick control
static const int LATENCY_TEST2 = 0;
static const int LATENCY_TEST3 = 0;

#ifdef JOYSTICK_CONTROL_D
extern void* readJoystick(void *);
#endif

using namespace std;

pthread_t client1;
pthread_t client2;
pthread_t client3; // !! this one doesn't seem to be used, I'm using client4
                   // just in case this is being reserved.. - TT
pthread_t client4; // to read joystick data continuously

pthread_t threads[MAX_PLAYER_NUMBER];


int sockid0, sockid1, sockid2, sockid3, sockid4, sockid5, sockid6;
int crossid[MAX_PLAYER_NUMBER];
int crossid2[MAX_PLAYER_NUMBER];

// sockets for world model display
int sockidWorldModelDisplays[MAX_PLAYER_NUMBER];

// ports for world model display
int WORLD_MODEL_DISPLAY_PORTS[MAX_PLAYER_NUMBER];
static const int WORLD_MODEL_DISPLAY_PORT_BASE = 5019;

#ifdef DELAY_TESTING
int ccplaneSize = INT_BYTES + COMPRESSED_CPLANE_SIZE + SENSOR_SIZE + SANITY_SIZE + OBJ_SIZE + GPS_SIZE + 4;
//int isDelayTesting = 1;
#else
int ccplaneSize = INT_BYTES + COMPRESSED_CPLANE_SIZE + SENSOR_SIZE + SANITY_SIZE + OBJ_SIZE + GPS_SIZE;
//int isDelayTesting = 0;
#endif

//const char DEFAULT_IP_PREFIX[] = "192.168.0."; // default
const char DEFAULT_IP_PREFIX[] = "10.1.2."; // default

int trial2 = 0;
int trial3 = 0;
char robotIp [MAX_PLAYER_NUMBER][16];
int robotPort = 54321;   // default;

int largestPacket = 100;
int regionSize= 500;
char currentBuffer[100];
char totalRegion[500];

int countPlayer = 0;

// this counter will be sent across to the base to record the delay occured
int delayTestingCounter = 0;

// true if processing cplane. Used as a lock to avoid overwriting the array while writing
int processingCPlane = 0;

/* Method */

//Cross Dog 
int clientStart(int port, char *host);

//Local - Java
int rConnect(short port, const char* host);

int clientReceive(int sockfd, void* b, int s);

int clientReceive(int sockfd, char* buffer);

int clientReceive(int sockfd, char* name, char* value);

int clientSendOriginal(int sockfd, const void* data, int size);

int clientSend(int sockfd, const void* data, int size);

void clientEnd(int sockfd);

void* latency(void *);

void* receiveCommand(void *);

// send command (name, value) to the robot
void sendRobotCommand(char name[], char value[]);

void sendOPlane(const unsigned char* data, int size);

void sendCompressedCPlane(const unsigned char* data, int size, ofstream& output);

void sendDebugPlane(const unsigned char* data, int size);

void sendRobotEnvironment(int robot, const float* data, int size);

void TakePhoto(const unsigned char *data, int size);

void* worldModel(void *);

void* debug_listen(void * robot);

// check the correctness of the primitive size
void checkPrimitiveSize() {
    if (INT_BYTES!=sizeof(int)) {
        cout << "CLIENT: Invalid INT_SIZE " << INT_BYTES << ". Should be " << sizeof(int) << endl;
        exit(1);
    }
    if (LONG_BYTES!=sizeof(long)) {
        cout << "CLIENT: Invalid LONG_SIZE " << LONG_BYTES << ". Should be " << sizeof(long) << endl;
        exit(1);
    }
}

static inline int32_t swapl(const int32_t val) {
	return ((((val) & 0xff000000) >> 24) | (((val) & 0x00ff0000) >> 8) |
			(((val) & 0x0000ff00) << 8) | (((val) & 0x000000ff) << 24));
}

// convert 4 consecutive data elements in a given byte array to an integer
static inline int bytesToInt(const unsigned char *data, int offset) {
	int32_t oldInt = ((int)(data[offset] & 0xff) + 
            (int)((data[offset+1] & 0xff) << 8) + 
            (int)((data[offset+2] & 0xff) << 16) + 
            (int)((data[offset+3] & 0xff) << 24));
	return oldInt;
}

#ifndef LITTLE_ENDIAN
// note: this is the opposite of network byte order

static inline void hostToDog(WDataInfo &wdata) {
	wdata.robot = swapl(wdata.robot);
	wdata.type = swapl(wdata.type);
	wdata.size = swapl(wdata.size);
}

static inline void dogToHost(WDataInfo &wdata) {
	wdata.robot = swapl(wdata.robot);
	wdata.type = swapl(wdata.type);
	wdata.size = swapl(wdata.size);
}

static inline void hostToDog(int32_t &data) {
	data = swapl(data);
}

static inline void dogToHost(int32_t &data) {
	data = swapl(data);
}

static inline void hostToDog(float &data) {
	int *idata = (int *)&data;
	*idata = swapl(*idata);
}

static inline void dogToHost(float &data) {
	int *idata = (int *)&data;
	*idata = swapl(*idata);
}

#else //is LITTLE_ENDIAN

static inline void hostToDog(WDataInfo &) {
}

static inline void dogToHost(WDataInfo &) {
}

static inline void hostToDog(int32_t &) {
}

static inline void dogToHost(int32_t &) {
}

static inline void hostToDog(float &) {
}

static inline void dogToHost(float &) {
}

#endif //LITTLE_ENDIAN



// print whether the switch has been defined or not
static void printSwitchBoard() {
    #ifdef DELAY_TESTING
        cout << "CLIENT: ---------- DELAY_TESTING turned on            ----------" << endl;
    #endif

    #ifdef HEADER_ALIGNMENT
        cout << "CLIENT: ---------- HEADER_ALIGNMENT turned on         ----------" << endl;
    #endif

    #ifdef REDUCE_COMMAND_DATA_SIZE
        cout << "CLIENT: ---------- REDUCE_COMMAND_DATA_SIZE turned on ----------" << endl;
    #endif
    
    #ifdef LOG_CPLANE
        cout << "CLIENT: ---------- LOG_CPLANE turned on               ----------" << endl;
    #endif
}

int main(int argc, char **argv);

#endif // _client_h_DEFINED
