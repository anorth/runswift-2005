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
 * @author Nicodemus Sutanto 06-01-02
 *
 * Last modification background information
 * $Id: Wireless.h 7128 2005-07-02 15:09:58Z weiming $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 * 
 * sets configuration file of the robot
 *
 * The Wireless object performs the message passing without any processing on the message 
 * 1. receives messages from the other local objects and pass to the base station
 * 2. receives messages from the base station and pass to the other local objects
 *
 **/


#ifndef Wireless_h_DEFINED
#define Wireless_h_DEFINED

#include <OPENR/OObject.h>
#include <OPENR/OSubject.h>
#include <OPENR/OObserver.h>
#include "TCPConnection.h"
#include "UDPConnection.h"
#include "WirelessConfig.h"
#include "def.h"
#include "../share/PackageDef.h"
#include "../share/SharedMemoryDef.h"
#include "../share/ActuatorWirelessData.h"
#include "../share/Common.h"
#include "../share/WirelessSwitchboard.h"

#include <sstream>

static const int MAX_TEAM_MEMBER = 4;

class Wireless : public OObject {
	public:
		Wireless();
		virtual ~Wireless() {
		}

		OSubject *subject[numOfSubject];
		OObserver *observer[numOfObserver];     


		//  OPENR Methods
		//
		virtual OStatus DoInit(const OSystemEvent &event);
		virtual OStatus DoStart(const OSystemEvent &event);
		virtual OStatus DoStop(const OSystemEvent &event);
		virtual OStatus DoDestroy(const OSystemEvent &event);

#ifdef USE_TCP
		void TCPListenCont(ANTENVMSG msg);
		void TCPSendCont(ANTENVMSG msg);
		void TCPReceiveCont(ANTENVMSG msg);
		void TCPCloseCont(ANTENVMSG msg);
#endif // USE_TCP

#ifdef USE_UDP
		void UDPSendCont(ANTENVMSG msg);
		void UDPReceiveCont(ANTENVMSG msg);
		void UDPCloseCont(ANTENVMSG msg);
#endif // USE_UDP

		//  Inter-Object Communication Methods
		//
		void Ready(const OReadyEvent &event);
		void redirectCompressedCPlane(const ONotifyEvent &event);
		void redirectWorldModel(const ONotifyEvent &event);
		void setPlayerNumber(const ONotifyEvent &event);
		void GameControl(const ONotifyEvent &event);
		void NetInfo(const ONotifyEvent &event);
		void redirectFromActuator(const ONotifyEvent &event);
        void sendTeamTalk(const ONotifyEvent &event);
        void gotGameControlData(const ONotifyEvent &event);

		//helper functions
		void sendWirelessMessage(int channel, int type, int size, const char *data);
		void ForwardNetInfo(const void *data, size_t size);
#ifdef IR_LATENCY_TEST
		// For inter-robot latency testing
		SystemTime latTime;
		int latDog;
#endif // IR_LATENCY_TEST

	private:
		int TCPindexB;
		int TCPindexC;
		int TCPindexD;
		WDataInfo info;

		int UDPSendIndex;       // index for worldmodel/teamtalk
		int UDPRecvIndex;

		/* variables for receiving of data header first, then data */
		bool receivingHeader[TCP_CONNECTION_MAX];
		int nextPacketSize[TCP_CONNECTION_MAX];
		WDataInfo lastPacketHeader[TCP_CONNECTION_MAX];

		antStackRef ipstackRef;

#ifdef USE_TCP
		TCPConnection tcpconnection[TCP_CONNECTION_MAX];
#endif // USE_TCP
#ifdef USE_UDP
		UDPConnection  udpconnection[UDP_CONNECTION_MAX];
#endif // USE_UDP

#ifdef USE_TCP
		OStatus TCPListen(int index);
		OStatus TCPSend(int index);
		OStatus TCPReceive(int index);
		OStatus TCPClose(int index);
		OStatus InitTCPConnection(int index);
#endif // USE_TCP

#ifdef USE_UDP
		OStatus UDPBind   (int index, Port port);
		OStatus UDPSend   (int index);
		OStatus UDPReceive(int index);
		OStatus UDPClose  (int index);
		OStatus InitUDPBuffer(int index);
		OStatus CreateUDPEndpoint(int index);
#endif	// USE_UDP

		// statements to indicate the on/off of the debug
		void printDebugSwitchBoard();

		/*
		 * processing of a wireless data packet
         * it is taken out from the ReceiveCont(void *msg) function to achieve
         * minimal change when TCP is changed to other type of connections
         **/
		void handlePacket(WDataInfo &packetHeader, const void *packetData);

		RCRegion *shared;
		const RC_DEBUG_DATA *sharedDebugMemory;

		Port port[MAX_TEAM_MEMBER];
		Port udpRecPort;
		Port udpSendPort;

		void readUDPPorts();

		// this counter will be sent across to the base to record the delay occured
		int delayTestingCounter;

		int PLAYER_NUMBER;
		int team;
};

#endif // Wireless_h_DEFINED
