/*

   Copyright 2005 The University of New South Wales (UNSW) and National  
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
 *
 * Last modification background information
 * $Id: client.cc weiming $
 *
 * Copyright (c) 2005 UNSW
 * All Rights Reserved.
 *
 **/

#include "client.h"
#include "share/SharedMemoryDef.h"
#include "share/ActuatorWirelessData.h"

#include <fstream>

#define SIZE_DEBUG 0 

int outputMsg                    = 1; // default it's 1, so verbose mode is on
static const int debugMsg                     = 0;
static const int commandDebugMsg              = 0;
static const int optimizeDebugMsg             = 0;
static const int delayDebugMsg                = 0;
static const int reduceSendingRateDebugMsg    = 0;
static const int modeZeroCrashDebugMsg        = 0;
static const int yuvDebugMsg                  = 0;
static const int headerAlignmentDebugMsg      = 0;
static const int newOplaneDebugMsg            = 0;
static const int compressedCPlaneDebugMsg     = 0;
static const int oplaneMissingDebugMsg        = 0;
static const int oneCrashWirelessDownDebugMsg = 0;
static long oneCrashWirelessDownCounter       = 0;
static const int maximumCountDebugMsg         = 0;
static const int objectCoordDebugMsg          = 0;
static const int sensorValuesDebugMsg         = 0;
static const int sanityValuesDebugMsg         = 0;
static const int logIndicatorDebugMsg         = 0;

static int socket_humanControl  = 0;
static int socket_roboCommander = 0;
static int socket_frameRate     = 0;
static int socket_cplane        = 0;
static int socket_yuvPlane      = 0;
static int socket_grapher       = 0;
static int socket_debug         = 0;
#define NUM_SOCKETS 8

static const int INVALID_COORD = 100000000;

int clientStart(int port, char* host) {
    struct sockaddr_in their_addr; // connector's address information
    struct hostent *he;
    int sockfd;

    // get socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("Error on port %d " , port);
        perror("socket");
        exit(1);
    }

    // get the host info
    if ((he=gethostbyname(host)) == NULL) {
        perror("gethostbyname");
        exit(1);
    }

    memset(&(their_addr.sin_zero), '\0', 8);  // zero the rest of the struct
    //memcpy(&their_addr.sin_addr, he->h_addr, he->h_length);
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);

    their_addr.sin_family = AF_INET;    // host byte order
    their_addr.sin_port = htons(port);  // short, network byte order

    // connect to address
    if (connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1) {
        perror("connect");
        printf("Could not connect to host %s port %d\n", host, port);
        exit(1);
    }

    return sockfd;

}

int rConnect(short port, const char* host) {
    struct sockaddr_in r_their_addr; // connector's address information
    struct hostent *r_he;
    int r_sockfd;

    // get socket
    if ((r_sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("Error on port:%d host:%s" , port, host);
        perror("socket");
        exit(1);
    }

    // get the host info
    if ((r_he=gethostbyname(host)) == NULL) {
        perror("gethostbyname");
        exit(1);
    }

    r_their_addr.sin_family = AF_INET;    // host byte order
    r_their_addr.sin_port = htons(port);  // short, network byte order
    r_their_addr.sin_addr = *((struct in_addr *)r_he->h_addr);
    memset(&(r_their_addr.sin_zero), '\0', 8);  // zero the rest of the struct
    //memcpy(&r_their_addr.sin_addr, r_he->h_addr, r_he->h_length);


    // connect to address
    if (connect(r_sockfd, (struct sockaddr *)&r_their_addr, sizeof(struct sockaddr)) == -1) {
        perror("connect");
        printf("Could not connect to host %s port %d\n", host, port);
        exit(1);
    }

    return r_sockfd;

}

int clientReceive(int sockfd, void* b, int s) {

  //int temp = 0;
  
  int result = 0;
  char *buff = (char *)b;
  while (s > 0) {
    result = recv(sockfd, buff, s,0);
    if (result < 0) {
      perror("=========CLIENT========== Error in Receiving From Server\n");
      return result;
    } else if (result == 0){
      printf("socket %d\n",sockfd);
      perror("=========CLIENT========== Receive zero bytes\n");
      perror("Program exited");
      exit(1);
    }
    
    if (debugMsg) {
      cout << "\nCLIENT receives from WirelessObject :" << result << endl;;
      for (int i=0; i<result; i++)
	cout << (int)(buff[i]) << " ";       
      cout << endl;
    }
    
    buff   += result;
    s      -= result;
    //temp   += result;
    
    result  = 0;
  }
  
  /*cout << "\nCLIENT receives from WirelessObject :" << temp << endl << endl;*/
  
  if (s < 0){
    perror ("---Got too many bytes --- ");
    abort();
  }
  return 0;
}


//Receiving one byte ( size of the packet)
int clientReceive(int sockfd, char* buffer) {
    unsigned char len;
    if (clientReceive(sockfd,&len,1) == 0 &&
            clientReceive(sockfd,buffer,(int) len) == 0) {
        buffer[len] = '\0';
        return 0;
    }
    return 1;
}


//Receiving from RoboCommander
int clientReceive(int sockfd, char* name, char* value) {
    if (clientReceive(sockfd,name) == 0 &&
            clientReceive(sockfd,value) == 0) {
        return 0;
    }
    return 1;
}



int clientSendOriginal(int sockfd, const void* data, int size) {
    if (debugMsg) {
        cout << "Sending to server with Size: " << size << endl;
    }
    if (send(sockfd,data, size, 0) == -1) {
        perror("============CLIENT========= Send to Java Error\n");
        return 1;
    }
    if (debugMsg) {
        cout << "Successfully sent!" << endl;
    }
    return 0;
}

int clientSend(int sockfd, const void* data, int size) {

    int temp = 0;
    int result = 0;
    const char *buff = (const char *)data;

    while (size > 0) {
        result = send(sockfd, buff, size, 0);
        if (result < 0) {
	  //HACK: sockfd == 0 -> send to CPlaneDisplay, 
	  //we allow it to be broken, as it can still log the cplane
	  if (sockfd != 0)
	    perror("============CLIENT========= Send to Java Error , port \n");
	  return result;
        }

        if (delayDebugMsg || headerAlignmentDebugMsg) {
            cout << "CLIENT sends socket " << sockfd << " " << result << " bytes" << endl;
        }
        if (debugMsg) {
            cout << "\nCLIENT sends " << result << " bytes!" << endl;
            for (int i=0; i<result; i++) {
                //cout << (int)(buff[i]) << " ";
            }
            cout << endl;
        }

        buff   += result;
        size   -= result;
        temp   += result;
        result  = 0;
    }

    if (debugMsg) {
        cout << "\nCLIENT sends to Local :" << temp << endl << endl;
    }
    return 0;
}


void clientEnd(int sockfd) {
    close(sockfd);
}


void* latency(void *){
    WDataInfo info;
    static const int dataSize = 160;
    char data[dataSize];
    char send_buffer[dataSize+12];
    char receive_buffer[dataSize+12];
    int ret;

#ifdef HEADER_ALIGNMENT
    strcpy(info.header, WDataHeader);
#endif
    info.robot  = 0;
    info.type   = 500;
    info.size   = 160;

    for (int c = 0; c < dataSize; c++) {
        data[c] = 'a';
    }

    hostToDog(info);
    memcpy(send_buffer, &info, sizeof(WDataInfo));
    memcpy(send_buffer+sizeof(WDataInfo), data, dataSize);

    struct timeval start_tv;
    struct timezone start_tz;
    struct timeval end_tv;
    struct timezone end_tz;
    int ret_time;
    int second;
    double micro;

    int iterator_interval = 20;
    double average_trip_time[iterator_interval];
    int iterator = 0;

    while (true) {
        /*for (int c = 0; c < sizeof(receive_buffer); c++)
          receive_buffer[c] = '\0';*/

        for (int x = 0; x < countPlayer; x++){
            ret_time = gettimeofday(&start_tv, &start_tz);

            ret = clientSend(crossid[x], send_buffer, sizeof(send_buffer));
            if (ret != 0) {
                perror("write on cross_id : " + x);
                close(crossid[x]);
                exit(1);
            }
            for (unsigned int d = 0; d < sizeof(receive_buffer); d++) {
                receive_buffer[d] = '\0';
            }


            if (clientReceive(crossid[x], receive_buffer, sizeof(WDataInfo)) == 0) {
                WDataInfo w = *(WDataInfo *) receive_buffer;
                dogToHost(w);
                int size = w.size;

                switch (w.type) {
                case 500:
                    if (clientReceive(crossid[x], receive_buffer, size) == 0){
                        bool status = true;
                        for (unsigned int j = 0; j < sizeof(receive_buffer) - sizeof(WDataInfo); j++){
                            if (receive_buffer[j] != 'b')
                                status = false;
                        }
                        if (status){
                            if (debugMsg) {
                                cout << "Data received correctly... " << endl;
                            }
                            ret_time = gettimeofday(&end_tv, &end_tz);

                            second = end_tv.tv_sec - start_tv.tv_sec;
                            micro = end_tv.tv_usec - start_tv.tv_usec;
                            if (micro < 0){
                                micro += second*1000000;
                            } else{
                                if (second != 0){
                                    second --;
                                }
                                micro += second * 100000;
                            }

                            micro = micro / 1000;
                            /*printf( "\nElapsed time : %0.3f milliseconds\n", micro );*/

                            average_trip_time[iterator] = micro;

                            iterator++;
                            if (iterator >= iterator_interval){
                                double total;
                                for (int x = 0; x < iterator_interval; x++){
                                    total += average_trip_time[x];
                                }
                                total = total/iterator_interval;
                                printf( "\nElapsed time : %0.3f milliseconds\n", total);
                                iterator = 0;
                            }

                            break;
                        } else {
                            cout << "Wrong data received... " << endl;
                        }
                    } else {
                        cout << "Error after the first 12 bytes!" << endl;
                        break;
                    }
                }
            }

        }
    }
    cout <<" Command out!" << endl;
    pthread_exit(0);
}

void* receiveCommand(void *){
    char name[NAME_SIZE];
    char value[VALUE_SIZE];

    while (true) {
        if (clientReceive(sockid0, name, value) == 0) {
            if (LATENCY_TEST3) {
                struct timeval tp;
                gettimeofday(&tp, NULL);
                long millis = tp.tv_sec*1000 + tp.tv_usec/1000;
                printf("t3end, trial #%d=%ld\n",trial3,millis);
                fflush(stdout);
                trial3++;
            }
            sendRobotCommand(name, value);
        } else {
            cout <<" Command out!" << endl;
            pthread_exit(0);
        }
    }
}

void sendRobotCommand(char name[], char value[]) {
    if (headerAlignmentDebugMsg) {
        cout << "CLIENT: in sendRobotCommand()" << endl;
    }
    WDataInfo info;
    WDataInfo nameInfo, valueInfo;
    int result;
    int totalSent;
    char send_buffer[COMMAND_SEND_BUFFER_MAX_SIZE];

    if (commandDebugMsg) {
        cout << name << "---" << value << endl;
    }

#ifdef HEADER_ALIGNMENT
    strcpy(nameInfo.header, WDataHeader);
#endif
    nameInfo.robot = 0;
    nameInfo.type = 200;
    nameInfo.size = strlen(name);

    hostToDog(nameInfo);

    if (strcmp (name, "ip") == 0){
        for (int x=0; x<countPlayer; x++){
            sprintf(value, "%d", x+1);

#ifdef HEADER_ALIGNMENT
            strcpy(valueInfo.header, WDataHeader);
#endif
            valueInfo.robot  = 0;
            valueInfo.type   = 201;
            valueInfo.size   = strlen(value);
            hostToDog(valueInfo);
            PackageDef pack  = PackageDef(name, nameInfo, value, valueInfo);
#ifdef HEADER_ALIGNMENT
            strcpy(info.header, WDataHeader);
#endif
            info.robot = 0;
            info.type  = 200;
            info.size  = sizeof(pack);
            hostToDog(info);

            memcpy(send_buffer, &info, sizeof(WDataInfo));
            memcpy(send_buffer+sizeof(WDataInfo), &pack, sizeof(pack));

            totalSent = sizeof(pack) + sizeof(WDataInfo);
            if (reduceSendingRateDebugMsg) {
                cout << "CLIENT: totalSent: " << totalSent << "; COMMAND_SEND_BUFFER_MAX_SIZE: " << COMMAND_SEND_BUFFER_MAX_SIZE << "; PackageDef: " << sizeof(PackageDef) << "; WDataInfo: " << sizeof(WDataInfo) << endl;
            }

            cout << "CLIENT assigning " << robotIp[x] << " IP = " << (x+1) << endl;
            result = clientSend(crossid[x], send_buffer, totalSent);

            if (result != 0) {
                perror("write on cross_id : " + x);
                close(crossid[x]);
                exit(1);
            }
        }
    } else {
#ifdef HEADER_ALIGNMENT
        strcpy(valueInfo.header, WDataHeader);
#endif
        valueInfo.robot = 0;
        valueInfo.type  = 201;
        valueInfo.size  = strlen(value);
        hostToDog(valueInfo);
        PackageDef pack = PackageDef(name, nameInfo, value, valueInfo);
#ifdef HEADER_ALIGNMENT
        strcpy(info.header, WDataHeader);
#endif
        info.robot = 0;
        info.type  = 200;
        info.size  = sizeof(pack);
        hostToDog(info);

        memcpy(send_buffer, &info, sizeof(WDataInfo));
        memcpy(send_buffer+sizeof(WDataInfo), &pack, sizeof(pack));

        totalSent = sizeof(pack) + sizeof(WDataInfo);
        if (totalSent!=COMMAND_SEND_BUFFER_MAX_SIZE) {
            cout << "CLIENT: Error totalSent!=COMMAND_SEND_BUFFER_MAX_SIZE, " << totalSent << " != " << COMMAND_SEND_BUFFER_MAX_SIZE << "." << endl;
        }
        if (delayDebugMsg) {
            cout << "CLIENT: receive (" << name << ", " << value << ")" << endl;
        }
        if (modeZeroCrashDebugMsg) {
            dogToHost(nameInfo);
            dogToHost(valueInfo);
            cout << "CLIENT: nameInfo robot: " << nameInfo.robot << "; type: " << nameInfo.type << "; size: " << nameInfo.size << endl;
            cout << "CLIENT: valueInfo robot: " << valueInfo.robot << "; type: " << valueInfo.type << "; size: " << valueInfo.size << endl;
            cout << "CLIENT: info robot: " << info.robot << "; type: " << info.type << "; size: " << info.size << endl;
            hostToDog(nameInfo);
            hostToDog(valueInfo);
        }

        for (int x=0; x <countPlayer; x++){
            result = clientSend(crossid[x], send_buffer, totalSent);
            if (modeZeroCrashDebugMsg) {
                cout << "sending to crossid[x]: " << crossid[x] << endl;
            }

            if (result != 0) {
                perror("write on cross_id : " + x);
                close(crossid[x]);
                exit(1);
            }
        }
    }

    if (headerAlignmentDebugMsg) {
        cout << "CLIENT: Finish sendRobotCommand()" << endl;
    }
}

// passes received oplane data thru to the socket
void sendOPlane(const unsigned char* data, int size) {
    /* send the number of object that will be sent */
    // subtract the pan and tilt that are sent first and get the size of a single object
    int32_t objNum = (size-NUM_ROBOT_DATA*INT_BYTES) / ONE_OBJ_SIZE;

    unsigned char *objNumPtr = (unsigned char *) malloc(sizeof(unsigned char) * 4);
    hostToDog(objNum);
    memcpy(objNumPtr, &objNum, sizeof(int));
    dogToHost(objNum);

    if (debugMsg) {
        cout << "content of objNoPtr: " << (int)(objNumPtr[0] & 0xff) << " ";
        cout << (int)((objNumPtr[1] & 0xff) << 8) << " ";
        cout << (int)((objNumPtr[2] & 0xff) << 16) << " ";
        cout << (int)((objNumPtr[3] & 0xff) << 24) << " = ";
        cout << bytesToInt(objNumPtr, 0) << endl;
    }

    clientSend(sockid6, objNumPtr, sizeof(const unsigned char *));

    /* send the object */
    clientSend(sockid6, data, size);

}

// passes received oplane data thru to the socket
// adds a counter before oplane data, used for latency_test 2
void sendOPlane(const unsigned char* data, int size, int count) {

    /* send the number of object that will be sent */
    // subtract the pan and tilt that are sent first and get the size of a single object
    int32_t objNum = (size-NUM_ROBOT_DATA*INT_BYTES) / ONE_OBJ_SIZE;

    unsigned char *countPtr = (unsigned char *) malloc(sizeof(unsigned char) * 4);

    //cout << "count: " << endl;
    //cout << "count cast: " << ((int32_t) count) << endl;


    hostToDog((int32_t &)count);
    memcpy(countPtr, &count, sizeof(int));
    dogToHost((int32_t &)count);

    unsigned char *objNumPtr = (unsigned char *) malloc(sizeof(unsigned char) * 4);
    hostToDog(objNum);
    memcpy(objNumPtr, &objNum, sizeof(int));
    dogToHost(objNum);

    if (debugMsg) {
        cout << "content of objNoPtr: " << (int)(objNumPtr[0] & 0xff) << " ";
        cout << (int)((objNumPtr[1] & 0xff) << 8) << " ";
        cout << (int)((objNumPtr[2] & 0xff) << 16) << " ";
        cout << (int)((objNumPtr[3] & 0xff) << 24) << " = ";
        cout << bytesToInt(objNumPtr, 0) << endl;
    }

    clientSend(sockid6, countPtr, sizeof(const unsigned char *));
    clientSend(sockid6, objNumPtr, sizeof(const unsigned char *));

    struct timeval tp;
    gettimeofday(&tp, NULL);
    long millis = tp.tv_sec*1000 + tp.tv_usec/1000;
    printf("t3end, trial #%d=%ld\n",count,millis);
    fflush(stdout);

    /* send the object */
    clientSend(sockid6, data, size);
}

void sendCompressedCPlane(const unsigned char *data, int size, ofstream& output) {

  static string header = MSGHEADER "CPLANE: ";

    if (isDelayTesting) {
        delayTestingCounter = (int)(data[size-1] & 0xff) +
                              (int)((data[size-2] & 0xff) << 8) +
                              (int)((data[size-3] & 0xff) << 16) +
                              (int)((data[size-4] & 0xff) << 24) ;
        cout << "CLIENT ---------- delayTestingCounter: " << delayTestingCounter << " ----------" << endl;
    }
    int i;
    int offset = 0;

    // get the size of the compressed cplane
    int compressedSize = bytesToInt(data, 0);
    if (compressedCPlaneDebugMsg) {
        cout << "total: " << size << "; compressedSize: " << compressedSize << "; data: ";
        cout << (int)(data[0] & 0xff) << " ";
        cout << (int)((data[1] & 0xff) << 8) << " ";
        cout << (int)((data[2] & 0xff) << 16) << " ";
        cout << (int)((data[3] & 0xff) << 24) << " = ";
        cout << bytesToInt(data, 0) << endl;
    }

    output << compressedSize << " ";

    // send the size of the compressed cplane
    clientSend(sockid2, data, INT_BYTES);

    // the data is offset by an integer, the compressedCPlaneSize
    data   += INT_BYTES;
    offset += INT_BYTES;

    for (i=0; i<compressedSize; i++) {
        output << (data[i] & 0xff) << " ";
    }

    /* send the cplane */
    if (outputMsg || compressedCPlaneDebugMsg)
      cout << header
	   << "compressedSize: " 
	   << compressedSize << " bytes" << endl;
    clientSend(sockid2, data, compressedSize);

    data   += compressedSize;
    offset += compressedSize;

    if (offset>size) {
        cout << "CLIENT: offset larger than data size " << offset 
	     << " > " << size << endl;
        exit(1);
    }

    if (compressedCPlaneDebugMsg) {
        cout << "CLIENT: after sending an integer for compressedSize (" << INT_BYTES << ") and cplane (" << compressedSize << "), offset = " << offset << endl;
    }

    for (i=0; i<SENSOR_SIZE; i+=LONG_BYTES) {
        output << bytesToInt(data, i) << " ";
    }

#if SIZE_DEBUG
    /* send the sensor readings */
    cout << " ----- sensor size ---- " << SENSOR_SIZE << " bytes" << endl;
#endif

    clientSend(sockid2, data, SENSOR_SIZE);

    data   += SENSOR_SIZE;
    offset += SENSOR_SIZE;

    if (offset>size) {
        cout << "CLIENT: offset larger than data size " << offset << " > " << size << endl;
        exit(1);
    }

    if (compressedCPlaneDebugMsg) {
        cout << "CLIENT: after sending sensor values (" << SENSOR_SIZE << "), offset = " << offset << endl;
    }

    for (i=0; i<SANITY_SIZE; i+=INT_BYTES) {
        output << bytesToInt(data, i) << " ";
    }

    /* send the sanity values */
#if SIZE_DEBUG
    cout << " ----------- sanity size" << SANITY_SIZE << " bytes" << endl;
#endif

    clientSend(sockid2, data, SANITY_SIZE);

    data   += SANITY_SIZE;
    offset += SANITY_SIZE;

    if (offset>size) {
        cout << "CLIENT: offset larger than data size " << offset << " > " << size << endl;
        exit(1);
    }

    if (compressedCPlaneDebugMsg) {
        cout << "CLIENT: after sending sanity values (" << SANITY_SIZE << "), offset = " << offset << endl;
    }

    /* send the number of object that is seen */
    int32_t objNum;
    if (isDelayTesting) {
        // subtract the pan and tilt that are sent first and get the size of a single object
        objNum = (size-GPS_SIZE-offset-INT_BYTES-NUM_ROBOT_DATA*INT_BYTES) / ONE_OBJ_SIZE;
    } else {
        // subtract the pan and tilt that are sent first and get the size of a single object
        objNum = (size-GPS_SIZE-offset-NUM_ROBOT_DATA*INT_BYTES) / ONE_OBJ_SIZE;
        if (newOplaneDebugMsg) {
            cout << "CLIENT: " << size << " - " << offset << " - " << NUM_ROBOT_DATA*INT_BYTES << " - " << GPS_SIZE << " = " << (size-offset-GPS_SIZE - NUM_ROBOT_DATA*INT_BYTES) << " ; objNum: " << objNum << endl;
        }
    }

    unsigned char *objNumPtr = (unsigned char *) malloc(sizeof(unsigned char) * INT_BYTES);
    hostToDog(objNum);
    memcpy(objNumPtr, &objNum, INT_BYTES);
    dogToHost(objNum);

    if (optimizeDebugMsg || oplaneMissingDebugMsg) {
        cout << size << " - " << offset << " = " << (size-offset) << "; objNum: " << objNum << "; objNoPtr: ";
        cout << (int)(objNumPtr[0] & 0xff) << " ";
        cout << (int)((objNumPtr[1] & 0xff) << 8) << " ";
        cout << (int)((objNumPtr[2] & 0xff) << 16) << " ";
        cout << (int)((objNumPtr[3] & 0xff) << 24) << " = ";
        cout << bytesToInt(objNumPtr, 0) << endl;
    }

    // send the number of object
#if SIZE_DEBUG
    cout << "CHAR SIZE: " << sizeof(const unsigned char *) << " bytes" << endl;
#endif
    clientSend(sockid2, objNumPtr, sizeof(const unsigned char *));

    int robotDataSize = NUM_ROBOT_DATA * INT_BYTES;
    int planeDataSize = objNum * ONE_OBJ_SIZE;
    int objectSize    = robotDataSize + planeDataSize;

    if (compressedCPlaneDebugMsg) {
        cout << "planeDataSize = " << planeDataSize << endl;
        cout << "objectSize = " << objectSize << endl;
    }


    for (i=0; i<robotDataSize; i+=INT_BYTES) {
        output << (double) bytesToInt(data, i) / SEND_SCALE << " ";
    }

    for (i=0; i<planeDataSize; i+=INT_BYTES) {
        output << bytesToInt(data, i+robotDataSize) << " ";
    }

    /* send the object */
    /* pan, tilt, horizon stuff + vobs. */
    if (outputMsg || compressedCPlaneDebugMsg) {
      cout << header
	   << "objectsize= " << objectSize
	   << " robotdata= " << robotDataSize 
	   << " planeData= " << planeDataSize
	   << endl; 
      cout << header
	   << "NUM_ROBOT_DATA= " << NUM_ROBOT_DATA 
	   << " objNum= " << objNum 
	   << " ONE_OBJ_SIZE= " << ONE_OBJ_SIZE
	   << endl;
    }
    clientSend(sockid2, data, objectSize);

    data   += objectSize;
    offset += objectSize;

    if (compressedCPlaneDebugMsg) {
        cout << "CLIENT: after sending object values (" << objectSize << "), offset = " << offset << endl;
    }

    if (offset>size) {
        cout << "CLIENT: offset larger than data size " << offset << " > " << size << endl;
        exit(1);
    }

    /* send the gps stuff */
    // addthe GPS tag
    output << "GPS ";
    for (int i = 0; i < GPS_SIZE; i+=INT_BYTES) {
        output << (int) bytesToInt(data, i) << " ";
    }
    
    data   += GPS_SIZE;
    offset += GPS_SIZE;

    if (compressedCPlaneDebugMsg) {
        cout << "CLIENT: after sending gps values (" << GPS_SIZE << "), offset = " << offset << endl;
    }

    if (isDelayTesting) {
        output << bytesToInt(data, 0) << " ";

        // send delay testing counter
        cout << " ---- 1   ------------- " << INT_BYTES << endl;
        clientSend(sockid2, data, INT_BYTES);

        data   += INT_BYTES;
        offset += INT_BYTES;

        if (offset>size) {
            cout << "CLIENT: offset larger than data size " << offset << " > " << size << endl;
            exit(1);
        }

    }

    if (compressedCPlaneDebugMsg) {
        cout << "CLIENT: the end, offset = " << offset << endl;
    }

    if (offset!=size) {
        cout << "CLIENT: Error: size of data get sent is different to the size received. Data sent: " << offset << "; size received: " << size << endl;
        exit(1);
    }

    output << "\n"; // one line per one frame
    
    if (outputMsg)
      cout << endl; // one line per one frame
    
}

void sendDebugPlane(const unsigned char* data, int size) {
    unsigned char type = shmdtEdgeDetectDebug;//!!!!!FIX THIS! -> Fixed
    clientSend(sockid5, &type, sizeof(unsigned char));
    clientSend(sockid5, data, size);
    if (debugMsg) {
        cout << "Debug plane sent of size " << size << endl;
    }
}

void sendRobotEnvironment(int /*robot*/, const float* data, int size) {
    stringstream out;
    int result;
    out << "<RINFO>\n";
    const float* ptr = data;
    for (unsigned int i = 0; i < size / sizeof(float); i++) {
        float d = *ptr;
        dogToHost(d);
        out << d << "\n";
        ptr++;
    }
    out << "</RINFO>\n";

    string sdata = out.str();
    const char* cdata = sdata.c_str();
    int len = sdata.length();

    // decide which port to send to based on player number
    float fPlayerNumber = *data;
    dogToHost(fPlayerNumber);
    int playerNumber = (int)fPlayerNumber;
    if (playerNumber > MAX_PLAYER_NUMBER) {
        cout << "sendRobotEnvironment: player number greater than max expected" << endl;
    } else {
        result = clientSend(sockidWorldModelDisplays[playerNumber-1], cdata, len);
        if (result != 0) {
            cout << "CLIENT: Error in sending robot environment to playerNumber " << playerNumber << endl;
            close(sockidWorldModelDisplays[playerNumber-1]);
            exit(1);
        }
        if (oneCrashWirelessDownDebugMsg) {
            cout << "CLIENT: " << oneCrashWirelessDownCounter << " Sending Robot Environment to playerNumber " << playerNumber << endl;
            oneCrashWirelessDownCounter++;
        }
    }
}


void TakePhoto(const unsigned char *data, int size) {
    unsigned char type;

    if (size >= YUVPLANE_SIZE) {
        // yuv
        if (debugMsg) {
            cout << "CLIENT: In TakePhoto(); Prepare to send YUVPlane " << size << endl;
        }
        type = shmdtYUVPlane ;
        clientSend(sockid3, &type, 1);
        clientSend(sockid3, data, size);

    } else {
        // cplane
        if (debugMsg) {
            cout << "CLIENT: In TakePhoto(); Prepare to send CPlane " << size << endl;
        }
        type = shmdtYUVPlaneCPlane ;
        clientSend(sockid3, &type, 1);
        clientSend(sockid3, data, size);
    }
}

void* worldModel(void *){
    unsigned char buff[ccplaneSize];
    char sharedWorldModel[160];
    int current_robot = 0;

    int interval = INTERVAL;
    int counter=0;
    char send_buffer[160+12];
    int share_target, result;
    WDataInfo info;

    while (true) {
        current_robot = counter/interval;

        if (debugMsg) {
            cout << "World Model Current Robot : " << current_robot << endl;
        }

        if (clientReceive(crossid2[current_robot], buff, sizeof(WDataInfo)) == 0) {
            WDataInfo w = *(WDataInfo *) buff;
            dogToHost(w);
            int size = w.size;

            if (debugMsg) {
                cout << endl << "===================================================" << endl;
                cout << "Basestation received SHARED WORLD MODEL from robot: " << current_robot << endl;
                cout << " with type:  " << w.type << endl;
            }


            // debugging
            switch (w.type) {

            case 300:
                if (clientReceive(crossid2[current_robot], sharedWorldModel, size) == 0) {
#ifdef HEADER_ALIGNMENT
                    strcpy(info.header, WDataHeader);
#endif
                    //info.robot = current_robot+1;
                    info.robot  = w.robot;
                    info.type   = 300;
                    info.size   =  w.size;
                    hostToDog(info);

                    memcpy(send_buffer, &info, sizeof(WDataInfo));
                    memcpy(send_buffer+sizeof(WDataInfo), sharedWorldModel, sizeof(sharedWorldModel));

                    for (share_target = 0; share_target < countPlayer; share_target++){
                        if (share_target!=current_robot){
                            result = clientSend(crossid2[share_target], send_buffer, info.size+sizeof(WDataInfo));
                            if (result != 0) {
                                cout << "CLIENT: Error in sending robot environment to share_target " << share_target << endl;
                            }
                            if (debugMsg) {
                                cout << "Robot : " << current_robot  << " is sending -- WorldModel -- to Robot : " << share_target  << endl;
                            }
                        }

                    }
                } else {
                    cout << "Error after the first 12 bytes!" << endl;
                    continue;
                }
                break;


            default:
                if (clientReceive(crossid2[current_robot], sharedWorldModel, size) == 0){
                    continue;
                } else {
                    cout << "Error after the first 12 bytes!" << endl;
                    continue;
                }
                break;
            }
        } else {
            if (debugMsg) {
                cout << "I am out now!" << endl;
            }
            pthread_exit(0);
        }

        counter++;
        if (counter >= interval*countPlayer) {
            counter = 0;
        }
    }
    return 0;
}


void* debug_listen(void * robot){
    unsigned char buff[ccplaneSize];
    char buffFloat[560];
    unsigned char buffPhoto[CPLANE_WIDTH*6*CPLANE_HEIGHT];
    unsigned char buffDebug[DEBUG_SIZE];
    char buffCase0[1024];

    int current_robot = (int) robot;

    int test_counter=0;

#ifdef LOG_CPLANE
    ofstream output(CPLANE_LOG, ios::out);
    output << CPLANE_VERSION << endl;
#endif
    
    while (true) {
      buff[0] = 0;
      if (clientReceive(crossid[current_robot], buff, sizeof(WDataInfo)) == 0) {
	WDataInfo w = *(WDataInfo *) buff;
	dogToHost(w);
	unsigned int size = w.size;
	
	
	if (outputMsg) {
	  cout << endl;
	  cout << "CLIENT: received information from robot: " << current_robot << endl;
	  cout << " with type:  " << w.type << " size: " << size << endl;
	}
	
	switch (w.type) {
	case 0:
	  
	  if (size > sizeof(buffCase0)) {
	    cout << "received too much data from the dog in case 0" << endl;
	    continue;
	    //abort();
	  }
	  
	  if (clientReceive(crossid[current_robot], buffCase0, size) == 0){
	    clientSend(sockid1, (char *)buffCase0, size);
	    break;
	  } else {
	    cout << "Error after the first 12 bytes!" << endl;
	    continue;
	  }
	  break;
	  
	case shmdtWorldModel:
	  if (clientReceive(crossid[current_robot], buffFloat, size) == 0){
	    if (debugMsg) {
	      cout << "Size : " << size << endl;
	    }
	    sendRobotEnvironment(current_robot, (const float*)buffFloat, size);
	  } else {
	    cout << "Error after the first 12 bytes!" << endl;
	    continue;
	  }
	  break;
	  
	case shmdtCPlane :
	  if (clientReceive(crossid[current_robot], buff, size) == 0) {
#ifdef LOG_CPLANE
	    sendCompressedCPlane(buff, size, output);
#else
	    sendCompressedCPlane(buff, size);
#endif
	  } else {
	    cout << "Error after the first 12 bytes!" << endl;
	    continue;
	  }
	  break;
	  
	case shmdtOPlane :
	  if (clientReceive(crossid[current_robot], buff, size) == 0){
	    if (LATENCY_TEST2) {
	      sendOPlane(buff, size, ++test_counter);
	    } else {
	      sendOPlane(buff, size);
	    }
	  } else {
	    cout << "Error after the first 12 bytes!" << endl;
	    continue;
	  }
	  break;
	  

	case shmdtYUVPlane :
	case shmdtYUVPlaneCPlane :
	  if (yuvDebugMsg) {
	    cout << "CLIENT: YUV type message" << endl;
	  }
	  if (clientReceive(crossid[current_robot], buffPhoto, size) == 0){
	    TakePhoto((const unsigned char*) buffPhoto, size);
	  } else {
	    cout << "Error after the first 12 bytes!" << endl;
	    continue;
	  }
	  break;
	  
	case shmdtEdgeDetectDebug:
	  if (clientReceive(crossid[current_robot], buffDebug, size) == 0) {
	    if (debugMsg) {
	      cout << "Starting debug" << endl;
	    }
	    sendDebugPlane(buffDebug, size);
	  } else {
	    cout << "Error after first 12 bytes!" << endl;
	  }
	  break;
	  
	case shmdtActuatorMessage: {
	  ActuatorWirelessDataEntry awd[20];
	  static bool fileOpen = false;
	  static std::ostream *actFile = NULL;
	  
	  if (!fileOpen) {
	    actFile = new std::ofstream("../offactuator/actuator.dat", ios::out);
	    fileOpen = true;
	  }
	  
	  int count = size / sizeof(ActuatorWirelessDataEntry);
	  if (count >= 20) {
	    cout << "too many data points at once!" << endl;
	    abort();
	  }
	  if (clientReceive(crossid[current_robot], &(awd[0]), size) == 0){
	    for (int i=0; i<count; i++) {
	      awd[i].dtoh();
	      awd[i].simplePrint(*actFile);
	      *actFile << " -1" << endl; // -1 means undefine stuck situation
	    }
	    break;
	  } else {
	    cout << "Error after the first 12 bytes!" << endl;
	    continue;
	  }
	}
	  break;
	  
	  
	default:
	  if (clientReceive(crossid[current_robot], buffPhoto, size) == 0){
	    cout << "Throwing unknown data of size " << size << " type " << w.type << endl;
	    continue;
	  } else {
	    cout << "Error after the first 12 bytes!" << endl;
	    continue;
	  }
	  break;
	}
      } else {
	if (debugMsg) {
	  cout << "I am out now!" << endl;
	}
	pthread_exit(0);
      }
    }
#ifdef LOG_CPLANE
    output.close();
#endif
    return 0;
}



void printUsage() {
  char buffLine[256];
  ifstream inFile("USAGE");
  if (inFile.fail()){
    cout << "CLIENT: Fail opening usage file " << endl;
    exit (1);
  }  
  while (!inFile.eof()) {
    inFile.getline(buffLine, 256, '\n');
    cout << buffLine << endl;
  }
  inFile.close();
}


void readCommand(char *command) {
  for (; *command != '\0'; command++) {
    switch (*command) {
    case 'q':
      outputMsg = 0;
      break;
    case 'h':       
      socket_humanControl = 1;
      break;
    case 'r':
      socket_roboCommander = 1;
      break;
    case 'c':
      socket_cplane = 1;
      break;
    case 'y':
      socket_yuvPlane = 1;
      break;
    case 'g':
      socket_grapher = 1;
      break;
    case 'd':
      socket_debug = 1;
      break;
    case 'f':
      socket_frameRate = 1;
      break;
    default:
      cout << "ERROR: Unknown Command '" << *command << "' !!" << endl;
      cout << endl;
      printUsage();
      exit(1);
    }
  }  
}


void readConfFile() {
  char buffHeaderFile [256];
  /* read the configuration file for the number of players, player IPs and player ports */
  ifstream inFile(PLAYER_CONFIG_FILE);
  
  if (inFile.fail()){
    cout << "CLIENT: Fail opening file " << PLAYER_CONFIG_FILE << endl;
    exit (1);
  }
    
  int ch;
  if ((ch = inFile.peek()) != EOF) {
    inFile.getline(buffHeaderFile, 256, '\n');
  }  
  
  inFile >> countPlayer;
  inFile.getline(robotIp[0], 20, '\n'); // get rid of '\n'
  
  for (int i = 0; i < countPlayer; i++) {
    inFile.getline(robotIp[i], 20, '\n');
  }
  
  inFile >> robotPort;
  
  inFile.close();
}




void readIPs(int num, char *IPs[]) {
  int i, j, prefixLen;
  num = (num <= MAX_PLAYER_NUMBER)? num : MAX_PLAYER_NUMBER;
  prefixLen = strlen(DEFAULT_IP_PREFIX);

  for (i = 0; i < num; i++) {
    // add prefix (eg. 192.168.0. )
    strcpy(robotIp[i], DEFAULT_IP_PREFIX);

    // add postfix (eg. 138)
    for (j =0; j <3; j++) {
      robotIp[i][j+prefixLen] = IPs[i][j];
    }
  }
}


int main(int argc, char *argv[]) {

  if (argc == 1 || (argc == 2 && strcmp(argv[1],"usage")==0) ) {
    printUsage();
    exit(0);
  }
  else if (argc == 2) { // if no IP addresses specified, read the conf file
    readCommand(argv[1]);
    readConfFile();
  }
  else { // read command and IP addresses from argv;
    readCommand(argv[1]);
    countPlayer = argc-2;
    readIPs(countPlayer, argv+2);    
  }

  if (outputMsg) {
    cout << "Number of Players: " << countPlayer << endl;
    for (int y = 0; y < countPlayer; y++) {
      cout << "Robot No: " << y + 1 << 
	"  IP: " << robotIp[y] <<
	"  Port: " << robotPort << endl;      
    }
    cout << endl;

    printSwitchBoard();    
  }

  
  checkPrimitiveSize();

#ifdef LOG_CPLANE
  /* read the cplane log version */
  ifstream cplaneVersionFile(CPLANE_VERSION_FILE);
  if (!cplaneVersionFile || cplaneVersionFile.fail()) {
    cout << "CLIENT: Fail opening file " << CPLANE_VERSION_FILE << endl;
  } else {
    cplaneVersionFile >> CPLANE_VERSION;
  }
  cout << "CPLANE_VERSION: " << CPLANE_VERSION << endl;
#endif
  
  
  if (socket_humanControl) {
    if (outputMsg)
      printf("Socket HumanControl Activated\n");
    sockid6 = rConnect(5102, LOCAL_HOST);   // oplane
  }
  if (socket_roboCommander) {
    if (outputMsg)
      printf("Socket RoboCommander Activated\n");
    sockid0 = rConnect(5005, LOCAL_HOST);   // receive commands RoboCommander
  }
  if (socket_cplane) {
    if (outputMsg)
      printf("Socket CPlane Activated\n");
    sockid2 = rConnect(5010, LOCAL_HOST);   // cplane
  }
  if (socket_yuvPlane) {
    if (outputMsg)
      printf("Socket YUVPlane Activated\n");
    sockid3 = rConnect(5011, LOCAL_HOST);   // port to receive photo
  }
  if (socket_grapher) {
    if (outputMsg)
      printf("Socket Grapher Activated\n");
    sockid4 = rConnect(5014, LOCAL_HOST);
  }
  if (socket_debug) {
    if (outputMsg)
      printf("Socket Debug Activated\n");
    sockid5 = rConnect(5015, LOCAL_HOST);   // Vision debug port
  }
  if (socket_frameRate) {
    if (outputMsg)
      printf("Socket FrameRate Activated\n");
    sockid1 = rConnect(5006, LOCAL_HOST);   // send frame rate data to RoboCommander
  }
  
    // establish world model display ports and sockets
    for (int i = 0; i < MAX_PLAYER_NUMBER; i++) {
        WORLD_MODEL_DISPLAY_PORTS[i] = WORLD_MODEL_DISPLAY_PORT_BASE + i;
        sockidWorldModelDisplays[i] = rConnect(WORLD_MODEL_DISPLAY_PORTS[i], LOCAL_HOST);
    }



    for (int y = 0; y < countPlayer; y++){
        crossid[y]  = clientStart(robotPort,   robotIp[y]); // Dog
        crossid2[y] = clientStart(robotPort+1, robotIp[y]); // Dog
    }

    int ret = 0;
    int postCommand;
    int postYUV;

    /*This is to test the Latency. disable other threads to use it.
      ret = pthread_create(&client1, NULL, latency, NULL);
      if(ret) {
      cout<<"Error creating Thread Client 1!!"<<endl;
      }*/


    ret = pthread_create(&client1, NULL, receiveCommand, NULL);
    if (ret) {
        cout<<"Error creating Thread Client 1!!"<<endl;
    }

    ret = pthread_create(&client2, NULL, worldModel, NULL);
    if (ret) {
        cout<<"Error creating Thread Client 2 !!"<<endl;
    }

#ifdef JOYSTICK_CONTROL_D
    ret = pthread_create(&client4, NULL, readJoystick, NULL);
    if (ret) {
        cout<<"Error creating Thread Client 4!!"<<endl;
    }
#endif

    for (int curr_robot = 0; curr_robot < countPlayer; curr_robot++) {
        ret = pthread_create(&threads[curr_robot], NULL, debug_listen, (void *) curr_robot);
        if (ret) {
            cout<<" Error creating Debug_Listen " << curr_robot << endl;
        }
    }

    pthread_join(client1, NULL);
    pthread_join(client2, NULL);

#ifdef JOYSTICK_CONTROL_D
    pthread_join(client4, NULL);
#endif

    for (int curr_robot = 0; curr_robot < countPlayer; curr_robot++){
        pthread_join(threads[curr_robot], NULL);
    }

    if ((postCommand) && (postYUV)){
        for (int z = 0; z < countPlayer; z++){
            clientEnd(crossid[z]);
            clientEnd(crossid2[z]);
        }
        clientEnd(sockid0);
        clientEnd(sockid1);
        clientEnd(sockid2);
        clientEnd(sockid3);
        clientEnd(sockid4);
        clientEnd(sockid5);
        clientEnd(sockid6);
        return 0;
    }

}

