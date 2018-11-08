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
 * UNSW 2003 Robocup (Nicodemus Sutanto)
 *
 * Last modification background information
 * $Id: Wireless.cc 5378 2005-04-02 08:27:43Z alexn $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 * 
 **/

#include <string.h>
#include <OPENR/OSyslog.h>
#include <OPENR/core_macro.h>
#include <OPENR/OPENRAPI.h>
#include <ant.h>
#include <EndpointTypes.h>
#include <TCPEndpointMsg.h>
#include <UDPEndpointMsg.h>
#include "Wireless.h"
#include "entry.h"

#include "../share/PackageDef.h"
#include "../share/WirelessTypeDef.h"
#include "../share/CommandData.h"
//#include "../share/RoboCupGameControlData.h"

#include "../share/debuggingControl.h"
#include <string>
#include <sstream>
#include <fstream>

using namespace std;

static const bool delayDebugMsg = false;
static const bool yuvDebugMsg = false;
static const bool modeZeroCrashDebugMsg = false;
static const bool bDebugCorruptedWireless = false;
static const bool cplaneDebugMsg = false;

/* Headers for UDP wireless messages. Keep any new one's to length 4 */
static const char worldModelHeader[] = {'N', 'S', 'w', 'm'}; 
static const char teamTalkHeader[] = {'N', 'S', 't', 't'};

Wireless::Wireless() {
	printDebugSwitchBoard();
	shared = NULL;
	sharedDebugMemory = NULL;

#ifdef IR_LATENCY_TEST
	GetSystemTime(&latTime);
	latDog = 0;
#endif // IR_LATENCY_TEST
}

void Wireless::printDebugSwitchBoard() {
	cout << "SENDING_DEBUG turned on" << endl;
	cout << "SENDING_WORLD_MODEL_DEBUG turned on" << endl;
	cout << "SENDING_CPLANE turned on" << endl;
	cout << "SENDING_OPLANE turned on" << endl;
	cout << "SENDING_YUVPLANE turned on" << endl;
	cout << "SENDING_LINE_DEBUG turned on" << endl;
}

OStatus Wireless::DoInit(const OSystemEvent &) {
	OSYSDEBUG(("Wireless::DoInit()\n"));

	NEW_ALL_SUBJECT_AND_OBSERVER;
	REGISTER_ALL_ENTRY;
	SET_ALL_READY_AND_NOTIFY_ENTRY;

	ipstackRef = antStackRef(strdup("IPStack"));

#ifdef USE_TCP
	for (int indexi = 0; indexi < TCP_CONNECTION_MAX; indexi++) {
		OStatus result = InitTCPConnection(indexi);
		if (result != oSUCCESS)
			return oFAIL;
	}
	TCPindexB = 0;
	TCPindexC = 1;
	TCPindexD = 2;
	port[0] = TCP_PORT1;
	port[1] = TCP_PORT2;
	port[2] = TCP_PORT2;
#endif	// USE_TCP

#ifdef USE_UDP
	udpRecPort  = UDP_PORT;
	udpSendPort = UDP_PORT+1;
    
	readUDPPorts();
	for (int indexi = 0; indexi < UDP_CONNECTION_MAX; indexi++) {
		OStatus result = InitUDPBuffer(indexi);
		if (result != oSUCCESS)
			return oFAIL;
	}	
#endif	// USE_UDP

	return oSUCCESS;
}

OStatus Wireless::DoStart(const OSystemEvent &) {
	OSYSDEBUG(("Wireless::DoStart()\n"));

	ENABLE_ALL_SUBJECT;
	ASSERT_READY_TO_ALL_OBSERVER;

#ifdef USE_TCP
	TCPListen(TCPindexB);
	TCPListen(TCPindexC);
#endif // USE_TCP

#ifdef USE_UDP
    for (int indexi = 0; indexi < UDP_CONNECTION_MAX; indexi++) {
        // 
        // Bind and Start receive data
        //
        OStatus result = CreateUDPEndpoint(indexi);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s[%d]",
                  "UDPEchoServer::DoStart()",
                  "CreateUDPEndpoint() fail", indexi));
            continue;
        }
    }
    UDPSendIndex = 1;
    UDPRecvIndex = 0;
	{	// start listening on the one port
	
        OStatus result = UDPBind(UDPRecvIndex, udpRecPort);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s[%d]",
                  "UDPEchoServer::DoStart()",
                  "Bind() fail", UDPRecvIndex));
        }
	}
	{	// start listening on the one port
	
        OStatus result = UDPBind(UDPSendIndex, udpSendPort);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s[%d]",
                  "UDPEchoServer::DoStart()",
                  "Bind() fail", UDPSendIndex));
        }
	}
    
    UDPReceive(UDPRecvIndex);

#endif // USE_UDP

	return oSUCCESS;
}    

OStatus Wireless::DoStop(const OSystemEvent &) {

#ifdef USE_UDP
    for (int index = 0; index < UDP_CONNECTION_MAX; index++) {
        if (udpconnection[index].state != CONNECTION_CLOSED &&
            udpconnection[index].state != CONNECTION_CLOSING) {

            // Connection close
            UDPEndpointCloseMsg closeMsg(udpconnection[index].endpoint);
            closeMsg.Call(ipstackRef, sizeof(closeMsg));
            udpconnection[index].state = CONNECTION_CLOSED;
        }
    }
#endif // USE_UDP

	DISABLE_ALL_SUBJECT;
	DEASSERT_READY_TO_ALL_OBSERVER;

	OSYSDEBUG(("Wireless::DoStop()\n"));
	return oSUCCESS;
}

OStatus Wireless::DoDestroy(const OSystemEvent &) {

#ifdef USE_UDP
    for (int index = 0; index < UDP_CONNECTION_MAX; index++) {
        // UnMap and Destroy SendBuffer
        udpconnection[index].sendBuffer.UnMap();
        antEnvDestroySharedBufferMsg destroySendBufferMsg(udpconnection[index].sendBuffer);
        destroySendBufferMsg.Call(ipstackRef, sizeof(destroySendBufferMsg));

        // UnMap and Destroy RecvBuffer
        udpconnection[index].recvBuffer.UnMap();
        antEnvDestroySharedBufferMsg destroyRecvBufferMsg(udpconnection[index].recvBuffer);
        destroyRecvBufferMsg.Call(ipstackRef, sizeof(destroyRecvBufferMsg));
    }
#endif // USE_UDP

	DELETE_ALL_SUBJECT_AND_OBSERVER;

	return oSUCCESS;
}

void Wireless::Ready(const OReadyEvent &) {
}


#ifdef USE_TCP
OStatus Wireless::InitTCPConnection(int channel) {
	OSYSDEBUG(("Wireless::InitTCPConnection()\n"));

	tcpconnection[channel].state = CONNECTION_CLOSED;

	/* Allocate send buffer */
	antEnvCreateSharedBufferMsg sendBufferMsg(TCP_BUFFER_SIZE_SENDER);

	sendBufferMsg.Call(ipstackRef, sizeof(sendBufferMsg));
	if (sendBufferMsg.error != ANT_SUCCESS) {
		OSYSLOG1((osyslogERROR,
					"Wireless::InitTCPConnection() : Can't allocate send buffer[%d] antError %d",
					channel,
					sendBufferMsg.error));
		return oFAIL;
	}

	tcpconnection[channel].sendBuffer = sendBufferMsg.buffer;
	tcpconnection[channel].sendBuffer.Map();
	tcpconnection[channel].sendData = (byte *)
		(tcpconnection[channel].sendBuffer.GetAddress());

	/* Allocate receive buffer */
	antEnvCreateSharedBufferMsg receiveBufferMsg(TCP_BUFFER_SIZE_RECEIVER);

	receiveBufferMsg.Call(ipstackRef, sizeof(receiveBufferMsg));
	if (receiveBufferMsg.error != ANT_SUCCESS) {
		OSYSLOG1((osyslogERROR,
					"Wireless::InitTCPConnection() : Can't allocate receive buffer[%d] antError %d",
					channel,
					receiveBufferMsg.error));
		return oFAIL;
	}

	tcpconnection[channel].receiveBuffer = receiveBufferMsg.buffer;
	tcpconnection[channel].receiveBuffer.Map();
	tcpconnection[channel].receiveData = (byte *)
		(tcpconnection[channel].receiveBuffer.GetAddress());

	return oSUCCESS;
}


OStatus Wireless::TCPListen(int channel) {
	OSYSDEBUG(("Wireless::Listen()\n"));

	if (tcpconnection[channel].state != CONNECTION_CLOSED) {
		return oFAIL;
	}

	// Create endpoint
	antEnvCreateEndpointMsg tcpCreateMsg(EndpointType_TCP,
			TCP_BUFFER_SIZE * 2);

	tcpCreateMsg.Call(ipstackRef, sizeof(tcpCreateMsg));
	if (tcpCreateMsg.error != ANT_SUCCESS) {
		OSYSLOG1((osyslogERROR,
					"Wireless::Listen() : Can't create endpoint[%d] antError %d",
					channel,
					tcpCreateMsg.error));
		return oFAIL;
	}
	tcpconnection[channel].endpoint = tcpCreateMsg.moduleRef;

	// Listen
	TCPEndpointListenMsg listenMsg(tcpconnection[channel].endpoint,
			IP_ADDR_ANY,
			port[channel]);
	listenMsg.continuation = (void *) channel;

	listenMsg.Send(ipstackRef,
			myOID_,
			Extra_Entry[entryTCPListenCont],
			sizeof(listenMsg));

	tcpconnection[channel].state = CONNECTION_LISTENING;

	return oSUCCESS;
}

void Wireless::TCPListenCont(void *msg) {
	OSYSDEBUG(("Wireless::TCPListenCont()\n"));

	TCPEndpointListenMsg *listenMsg = (TCPEndpointListenMsg *) msg;
	int channel = (int) listenMsg->continuation;

	if (listenMsg->error != TCP_SUCCESS) {
		OSYSLOG1((osyslogERROR,
					"Wireless::TCPListenCont() : FAILED. listenMsg->error %d",
					listenMsg->error));
		TCPClose(channel);
		return;
	}

	tcpconnection[channel].state = CONNECTION_CONNECTED;

	// so that it will receive the header first
	receivingHeader[channel] = true;
	TCPReceive(channel);
}

OStatus Wireless::TCPSend(int channel) {
	OSYSDEBUG(("Wireless::Send()\n"));
	if (channel == 0 && yuvDebugMsg) {
		cout << "Wireless.cc: 3. In Send()" << endl;
	}

	if (tcpconnection[channel].sendSize == 0
			|| tcpconnection[channel].state != CONNECTION_CONNECTED) {
		return oFAIL;
	}
	if (TCP_BUFFER_SIZE_SENDER < tcpconnection[channel].sendSize) {
		cout
			<< "Wireless.cc: Error: send buffer size smaller than the actual data size; send buffer size: "
			<< TCP_BUFFER_SIZE_RECEIVER
			<< "; send data size: "
			<< tcpconnection[channel].sendSize
			<< endl;
		return oFAIL;
	}

	tcpconnection[channel].state = CONNECTION_SENDING;

	TCPEndpointSendMsg sendMsg(tcpconnection[channel].endpoint,
			tcpconnection[channel].sendData,
			tcpconnection[channel].sendSize);
	if (channel == 0 && yuvDebugMsg) {
		cout << "Wireless.cc: 4. sendSize = " << tcpconnection[channel].sendSize
			<< endl;
	}

	sendMsg.continuation = (void *) channel;

	sendMsg.Send(ipstackRef, myOID_, Extra_Entry[entryTCPSendCont], sizeof(sendMsg));

	tcpconnection[channel].sendSize = 0;

	if (channel == 0 && yuvDebugMsg) {
		cout << "Wireless.cc: 5. after sendMsg.Send()" << endl;
	}

	return oSUCCESS;
}

void Wireless::TCPSendCont(void *msg) {
	OSYSDEBUG(("Wireless::TCPSendCont()\n"));

	TCPEndpointSendMsg *sendMsg = (TCPEndpointSendMsg *) msg;
	int channel = (int) (sendMsg->continuation);

	if (channel == 0 && yuvDebugMsg) {
		cout << "Wireless.cc: 6. In TCPSendCont(): sendMsg->error: "
			<< (sendMsg->error) << endl;
	}

	if (sendMsg->error != TCP_SUCCESS) {
		OSYSLOG1((osyslogERROR,
					"Wireless::TCPSendCont() : FAILED. sendMsg->error %d",
					sendMsg->error));
		TCPClose(channel);
		return;
	}

	// empty connection.sendData buffer
	tcpconnection[channel].sendData[0] = '\0';
	tcpconnection[channel].state = CONNECTION_CONNECTED;
}

OStatus Wireless::TCPReceive(int channel) {
	OSYSDEBUG(("Wireless::Receive()\n"));
	if (modeZeroCrashDebugMsg) {
		cout << "In Receive()" << endl;
	}

	if (tcpconnection[channel].state != CONNECTION_CONNECTED
			&& tcpconnection[channel].state != CONNECTION_SENDING) {
		cout << "connection idle" << endl;
		return oFAIL;
	}

	int currentReceiveSize;
	if (receivingHeader[channel]) {
		// receiving the header
		currentReceiveSize = sizeof(WDataInfo);
	}
	else {
		// receiving the data
		currentReceiveSize = nextPacketSize[channel];
	}
	if (TCP_BUFFER_SIZE_RECEIVER < currentReceiveSize) {
		cout
			<< "Wireless.cc: Error: receive buffer size smaller than the actual data size; receive buffer size: "
			<< TCP_BUFFER_SIZE_RECEIVER
			<< "; receive data size: "
			<< currentReceiveSize
			<< endl;
		return oFAIL;
	}
	// create a message with the amount of data to be received
	TCPEndpointReceiveMsg receiveMsg(tcpconnection[channel].endpoint,
			tcpconnection[channel].receiveData,
			currentReceiveSize,
			currentReceiveSize);
	receiveMsg.continuation = (void *) channel;
	receiveMsg.Send(ipstackRef,
			myOID_,
			Extra_Entry[entryTCPReceiveCont],
			sizeof(receiveMsg));
	if (cplaneDebugMsg) {
		cout << "Wireless.cc: currentReceiveSize: " << currentReceiveSize << endl;
	}
	if (modeZeroCrashDebugMsg) {
		cout << "Wireless.cc: Finish Receive()" << endl;
		cout << endl;
	}
	return oSUCCESS;
}

void Wireless::TCPReceiveCont(void *msg) {
	OSYSDEBUG(("Wireless::TCPReceiveCont()\n"));
	if (modeZeroCrashDebugMsg) {
		cout << "In TCPReceiveCont()" << endl;
	}

	TCPEndpointReceiveMsg *receiveMsg = (TCPEndpointReceiveMsg *) msg;
	int channel = (int) (receiveMsg->continuation);

	if (receiveMsg->error != TCP_SUCCESS) {
		OSYSLOG1((osyslogERROR,
					"Wireless::TCPReceiveCont() : FAILED. receiveMsg->error %d",
					receiveMsg->error));
		TCPClose(channel);
		return;
	}

	// alternating the receiving of header and data
	if (receivingHeader[channel]) {
		WDataInfo packetHeader = *(WDataInfo *) tcpconnection[channel].receiveData;

#ifdef HEADER_ALIGNMENT
		if (sizeof(WDataHeader) != sizeof(packetHeader.header)) {
			cout << "Wireless.cc: Incorrect packet magic header size" << endl;
			return;
		}

		for (unsigned int i = 0; i < sizeof(WDataHeader); i++) {
			if (WDataHeader[i] != packetHeader.header[i]) {
				cout << "Wireless.cc: packet headers misaligned. i = " << i << "; ("
					<< WDataHeader[i] << " : " << (packetHeader.header[i]) << ")"
					<< endl;
				return;
			}
		}
#endif

		nextPacketSize[channel] = packetHeader.size;
		receivingHeader[channel] = false;
		lastPacketHeader[channel] = packetHeader;
	}
	else {
		handlePacket(lastPacketHeader[channel], tcpconnection[channel].receiveData);
		receivingHeader[channel] = true;
	}

	TCPReceive(channel);

	if (modeZeroCrashDebugMsg) {
		cout << "Finish TCPReceiveCont()" << endl;
		cout << endl;
	}
}
#endif // USE_TCP

void Wireless::handlePacket(WDataInfo &packetHeader, const void *packetData) {
	//int msgType = packetHeader.type;
	if (modeZeroCrashDebugMsg) {
		cout << "In handlePacket()" << endl;
	}
	if (delayDebugMsg || bDebugCorruptedWireless) {
		cout << "Wireless.cc: packetHeader.type = " << packetHeader.type
			<< "; packetHeader.size = " << packetHeader.size << endl;
	}

	if (packetHeader.type == 200) {
		PackageDef pack = *(const PackageDef *) packetData;
        if (bDebugCorruptedWireless){
            cout << "Wireless.cc: name=" << pack.name << " value=" << pack.value << endl;
        }

		CommandData command(pack);

		subject[sbjCommandReceived]->ClearBuffer();
		subject[sbjCommandReceived]->SetData(&command, sizeof(command));

		if (delayDebugMsg) {
			cout << "Wireless.cc: (" << pack.name << ", " << pack.value << ")"
				<< endl;

			/*PackageDef *unpack = (PackageDef *) ((int)(shared->Base()+REGION_G));
			  cout << "Wireless.cc unpack: (" << unpack->name << ", " << unpack->value << ")" << endl;
			  cout << "Wireless.cc Name robot: " << unpack->nameInfo.robot << "; type: " << unpack->nameInfo.type << "; size: " << unpack->nameInfo.size << "; name: " << unpack->name << endl;
			  cout << "Wireless.cc Value robot: " << unpack->valueInfo.robot << "; type: " << unpack->valueInfo.type << "; size: " << unpack->valueInfo.size << "; value: " << unpack->value << endl;*/
		}

		subject[sbjCommandReceived]->NotifyObservers();
	} 

	if (modeZeroCrashDebugMsg) {
		cout << "Finish handlePacket()" << endl;
	}
}

#ifdef USE_TCP
OStatus Wireless::TCPClose(int channel) {
	OSYSDEBUG(("Wireless::Close()\n"));

	if (tcpconnection[channel].state == CONNECTION_CLOSED
			|| tcpconnection[channel].state == CONNECTION_CLOSING) {
		return oFAIL;
	}

	TCPEndpointCloseMsg closeMsg(tcpconnection[channel].endpoint);
	closeMsg.continuation = (void *) channel;

	closeMsg.Send(ipstackRef,
			myOID_,
			Extra_Entry[entryTCPCloseCont],
			sizeof(closeMsg));

	tcpconnection[channel].state = CONNECTION_CLOSING;

	return oSUCCESS;
}

void Wireless::TCPCloseCont(void *msg) {
	OSYSDEBUG(("Wireless::TCPCloseCont()\n"));

	TCPEndpointCloseMsg *closeMsg = (TCPEndpointCloseMsg *) msg;
	int channel = (int) (closeMsg->continuation);

	tcpconnection[channel].state = CONNECTION_CLOSED;
	TCPListen(channel);
}
#endif // USE_TCP

#ifdef USE_UDP

OStatus
Wireless::UDPSend(int index)
{
    OSYSDEBUG(("Wireless::Send()\n"));

    if (udpconnection[index].sendSize == 0 ||
        udpconnection[index].state != CONNECTION_CONNECTED) return oFAIL;


	udpconnection[index].sendAddress = IP_ADDR_BROADCAST;
	udpconnection[index].sendPort = udpRecPort; // broadcast to everyone's receiving port

	// cout << "sending to port: " << udpRecPort << endl;

    UDPEndpointSendMsg sendMsg(udpconnection[index].endpoint,
                               udpconnection[index].sendAddress,
                               udpconnection[index].sendPort,
                               udpconnection[index].sendData,
                               udpconnection[index].sendSize);
    sendMsg.continuation = (void*)index;
    sendMsg.Send(ipstackRef, myOID_,
                 Extra_Entry[entryUDPSendCont], sizeof(UDPEndpointSendMsg));
                 
    udpconnection[index].state = CONNECTION_SENDING;
    return oSUCCESS;
}

void
Wireless::UDPSendCont(ANTENVMSG msg)
{
    OSYSDEBUG(("Wireless::SendCont()\n"));

    UDPEndpointSendMsg* sendMsg = (UDPEndpointSendMsg*)antEnvMsg::Receive(msg);
    int index = (int)(sendMsg->continuation);
    if (udpconnection[index].state == CONNECTION_CLOSED)
        return;

    if (sendMsg->error != UDP_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "Wireless::SendCont()",
                  "FAILED. sendMsg->error", sendMsg->error));
        UDPClose(index);
        return;
    }
    
    udpconnection[index].state = CONNECTION_CONNECTED;
}

/* Waits for a UDP message */
OStatus
Wireless::UDPReceive(int index)
{
    OSYSDEBUG(("Wireless::Receive()\n"));    

    if (udpconnection[index].state != CONNECTION_CONNECTED &&
        udpconnection[index].state != CONNECTION_SENDING) return oFAIL;

	udpconnection[index].recvSize = UDP_BUFFER_SIZE;

    UDPEndpointReceiveMsg receiveMsg(udpconnection[index].endpoint,
                                     udpconnection[index].recvData,
                                     udpconnection[index].recvSize);
    receiveMsg.continuation = (void*)index;

    receiveMsg.Send(ipstackRef, myOID_,
                    Extra_Entry[entryUDPReceiveCont], sizeof(receiveMsg));

    return oSUCCESS;
}

/* The callback from UDPReceive, invoked when a message is received. */
void
Wireless::UDPReceiveCont(ANTENVMSG msg)
{
    OSYSDEBUG(("Wireless::ReceiveCont()\n"));
    
    UDPEndpointReceiveMsg* receiveMsg
        = (UDPEndpointReceiveMsg*)antEnvMsg::Receive(msg);

    int index = (int)(receiveMsg->continuation);
    if (udpconnection[index].state == CONNECTION_CLOSED) return;

    if (receiveMsg->error != UDP_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "Wireless::ReceiveCont()",
                  "FAILED. receiveMsg->error", receiveMsg->error));
        UDPClose(index);
        return;
    }

/*
	// this is the original ping code
	
    OSYSPRINT(("recvData : %s", receiveMsg->buffer));

    //
    // Send back the message
    //
    udpconnection[index].sendAddress = receiveMsg->address;
    udpconnection[index].sendPort    = receiveMsg->port;
    udpconnection[index].sendSize    = receiveMsg->size;
    memcpy(udpconnection[index].sendData, receiveMsg->buffer, receiveMsg->size);

    UDPSend(index);
*/

	ForwardNetInfo(receiveMsg->buffer, receiveMsg->size);

	udpconnection[index].state = CONNECTION_CONNECTED;
	udpconnection[index].recvSize = UDP_BUFFER_SIZE;
	UDPReceive(index);
}

OStatus
Wireless::UDPClose(int index)
{
    OSYSDEBUG(("Wireless::Close()\n"));

    if (udpconnection[index].state == CONNECTION_CLOSED ||
        udpconnection[index].state == CONNECTION_CLOSING) return oFAIL;

    UDPEndpointCloseMsg closeMsg(udpconnection[index].endpoint);
    closeMsg.continuation = (void*)index;

    closeMsg.Send(ipstackRef, myOID_,
                  Extra_Entry[entryUDPCloseCont], sizeof(closeMsg));

    udpconnection[index].state = CONNECTION_CLOSING;

    return oSUCCESS;
}

void
Wireless::UDPCloseCont(ANTENVMSG msg)
{
    OSYSDEBUG(("Wireless::CloseCont()\n"));
    
    UDPEndpointCloseMsg* closeMsg = (UDPEndpointCloseMsg*)antEnvMsg::Receive(msg);
    int index = (int)(closeMsg->continuation);
    if (udpconnection[index].state == CONNECTION_CLOSED)
        return;

    udpconnection[index].state = CONNECTION_CLOSED;

/*
    OStatus result;
    result = CreateUDPEndpoint(index);
    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s[%d]",
                  "Wireless::CloseCont()",
                  "CreateUDPEndpoint() fail", index));
        return;
    }
    result = UDPBind(index);
    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s[%d]",
                  "Wireless::CloseCont()",
                  "Bind() fail", index));
    }
*/
}

OStatus
Wireless::InitUDPBuffer(int index)
{
    OSYSDEBUG(("Wireless::InitUDPBuffer()\n"));

    udpconnection[index].state = CONNECTION_CLOSED;

    // 
    // Allocate send buffer
    //
    antEnvCreateSharedBufferMsg sendBufferMsg(UDP_BUFFER_SIZE);

    sendBufferMsg.Call(ipstackRef, sizeof(sendBufferMsg));
    if (sendBufferMsg.error != ANT_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s[%d] antError %d",
                  "Wireless::InitUDPBuffer()",
                  "Can't allocate send buffer",
                  index, sendBufferMsg.error));
        return oFAIL;
    }

    udpconnection[index].sendBuffer = sendBufferMsg.buffer;
    udpconnection[index].sendBuffer.Map();
    udpconnection[index].sendData
        = (byte*)(udpconnection[index].sendBuffer.GetAddress());

    //
    // Allocate receive buffer
    //
    antEnvCreateSharedBufferMsg recvBufferMsg(UDP_BUFFER_SIZE);

    recvBufferMsg.Call(ipstackRef, sizeof(recvBufferMsg));
    if (recvBufferMsg.error != ANT_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s[%d] antError %d",
                  "Wireless::InitUDPBuffer()",
                  "Can't allocate receive buffer",
                  index, recvBufferMsg.error));
        return oFAIL;
    }

    udpconnection[index].recvBuffer = recvBufferMsg.buffer;
    udpconnection[index].recvBuffer.Map();
    udpconnection[index].recvData
        = (byte*)(udpconnection[index].recvBuffer.GetAddress());
    udpconnection[index].recvSize = UDP_BUFFER_SIZE;

    return oSUCCESS;
}

OStatus
Wireless::CreateUDPEndpoint(int index)
{
    OSYSDEBUG(("Wireless::CreateUDPEndpoint()\n"));

    if (udpconnection[index].state != CONNECTION_CLOSED) return oFAIL;

    //
    // Create UDP endpoint
    //
    antEnvCreateEndpointMsg udpCreateMsg(EndpointType_UDP,
                                         UDP_BUFFER_SIZE * 2);
    udpCreateMsg.Call(ipstackRef, sizeof(udpCreateMsg));
    if (udpCreateMsg.error != ANT_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s[%d] antError %d",
                  "Wireless::CreateUDPEndpoint()",
                  "Can't create endpoint",
                  index, udpCreateMsg.error));
        return oFAIL;
    }
    udpconnection[index].endpoint = udpCreateMsg.moduleRef;

    return oSUCCESS;
}


OStatus
Wireless::UDPBind(int index, Port bport)
{
    OSYSDEBUG(("Wireless::Bind()\n"));

    if (udpconnection[index].state != CONNECTION_CLOSED) return oFAIL;

    // 
    // Bind
    //
    cout << "Binding to UDP Port: " << bport << endl;
    UDPEndpointBindMsg bindMsg(udpconnection[index].endpoint, 
                               IP_ADDR_ANY, bport);
    bindMsg.Call(ipstackRef,sizeof(antEnvCreateEndpointMsg));
    if (bindMsg.error != UDP_SUCCESS) {
        return oFAIL;
    }

    udpconnection[index].state = CONNECTION_CONNECTED;
    udpconnection[index].recvSize = UDP_BUFFER_SIZE;

    return oSUCCESS;
}

#endif	// USE_UDP

/* Sends this dog's world model (in event.Data) out over UDP broadcast. */
void Wireless::redirectWorldModel(const ONotifyEvent &event) {

	const WMShareObj &wm = *(const WMShareObj *) event.Data(0);
	WMShareInfo wi;
	wi.obj = wm;
	wi.playerNum = PLAYER_NUMBER;
	wi.team = team;
    memmove(wi.header, worldModelHeader, sizeof(worldModelHeader));
//	wi.header[0] = 'N';
//	wi.header[1] = 'S';
//	wi.header[2] = 'w';
//	wi.header[3] = 'm';

#ifdef IR_LATENCY_TEST
	if (wm.latMode == INIT_LAT_WM && latDog == 0) {
		GetSystemTime(&latTime);
		//latDog = -1;
		cout << "reset latTime - " << latTime.seconds << "s " << latTime.useconds
			<< "us" << endl;
	}
	else if (latDog > 0) {
		cout << "returning ping to " << latDog << endl;
		wi.obj.latMode = latDog;
		latDog = 0;
		SystemTime temp;
		GetSystemTime(&temp);
		wi.obj.waited = temp - latTime;
	}
#endif // IR_LATENCY_TEST

#ifdef USE_UDP
	if (udpconnection[UDPSendIndex].state == CONNECTION_CONNECTED) {		
		udpconnection[UDPSendIndex].sendSize    = sizeof(wi);
		memcpy(udpconnection[UDPSendIndex].sendData, &wi, sizeof(wi));
	
		UDPSend(UDPSendIndex);
	} else {
		cout << "Unable to send UDP world model info - connection not connected" << endl;
	}
#else
	subject[sbjNetSendInfo]->ClearBuffer();
	subject[sbjNetSendInfo]->SetData(&wi, sizeof(wi));
	subject[sbjNetSendInfo]->NotifyObservers();
#endif

	observer[obsWorldModelSend]->AssertReady();
}

/* Sends this dog's CPlane over the TCP connection to base station */
void Wireless::redirectCompressedCPlane(const ONotifyEvent &event) {
	//cout << "redirectCompressedCPlane" << endl;
	int msgType = *(const int *) event.Data(0);
	if (cplaneDebugMsg) {
		cout << "Wireless.cc: in redirectCompressedCPlane" << endl;
	}

	if (msgType == msgTypeESTABLISHING) {
		const MemoryRegionID mem = *(const MemoryRegionID *) event.Data(1);
		const size_t offset = *(const size_t *) event.Data(2);
		const voidPtr base = *(const voidPtr *) event.Data(3);
		const size_t size = *(const size_t *) event.Data(4);

		if (cplaneDebugMsg) {
			cout << "Wireless.cc: msgType=0" << endl;
		}

		shared = new RCRegion(mem, offset, base, size);
		sharedDebugMemory = (const RC_DEBUG_DATA *) shared->Base();
		if (sharedDebugMemory != base) {
			cout << "Wireless: SharedMemory assert failed" << endl;
			sharedDebugMemory = NULL;
			shared = NULL;
		}
	}
	else if (sharedDebugMemory == NULL) {
		cout << "Wireless: received debug message before shared memory pointer"
			<< endl;
	}
	else {
		if (cplaneDebugMsg) {
			cout << "Wireless.cc: msgType!=0" << endl;
		}

		//int type      = *(const int *) shared->Base();
		//const int size = *(const int *) (shared->Base()+sizeof(int));
		int type = *(const int *) event.Data(1);
		const int size = *(const int *) event.Data(2);
		//cout << "type=" << type << " size = " << size << " msgType = " << msgType << endl;

		const char *data = 0;
		int channel = 0;

		if (type == shmdtMessages) {
			data = sharedDebugMemory->message;
			channel = TCPindexB;
		}
		else if (type == shmdtWorldModel) {
			data = sharedDebugMemory->world_model_debug_data;
			channel = TCPindexB;
		}
		else if (type == shmdtCPlane) {
			data = sharedDebugMemory->cplane_data;
			channel = TCPindexB;
		}
		else if (type == shmdtOPlane) {
			data = sharedDebugMemory->oplane_data;
			channel = TCPindexB;
		}
		else if (type == shmdtYUVPlaneCPlane) {
			data = sharedDebugMemory->yuvCplane_data;
			channel = TCPindexB;
		}
		else if (type == shmdtYUVPlane) {
			data = sharedDebugMemory->yuvplane_data;
			channel = TCPindexB;
		}
		else if (type == shmdtEdgeDetectDebug) {
			//EDGE_DETECT
			data = sharedDebugMemory->line_debug_data;    //EDGE_DETECT
			channel = TCPindexB;
		} 
#ifdef USE_TCP
		if (tcpconnection[channel].state != CONNECTION_SENDING) {
			if (type == shmdtCPlane && isDelayTesting) {
				delayTestingCounter = (int) (data[size - 1] & 0xff)
					+ (int) ((data[size - 2] & 0xff) << 8)
					+ (int) ((data[size - 3] & 0xff) << 16)
					+ (int) ((data[size - 4] & 0xff) << 24) ;
				cout << "Wireless.cc ---------- delayTestingCounter: "
					<< delayTestingCounter << " ----------" << endl;
			}

			sendWirelessMessage(channel, type,size , data);

			if ((type == shmdtYUVPlaneCPlane || type == shmdtYUVPlane)
					&& yuvDebugMsg) {
				cout << endl;
				cout << "Wireless.cc: 1. type = " << type << "; sizeof(WDataInfo) = "
					<< sizeof(WDataInfo) << "; totalSize = "
					<< (sizeof(WDataInfo) + info.size) << endl;
				cout << "Wireless.cc: 2. sending to channel " << channel << endl;
			}
		}
#endif // USE_TCP
	}

	observer[obsLISTENER]->AssertReady();
}

void Wireless::redirectFromActuator(const ONotifyEvent &event){
	const int *count = (const int*) event.Data(0);
	const char *awd = (const char*) event.Data(1);
	/*
	   cout << "Received from actuator control " << endl;
	   cout << " Type = " << awd->type << endl;
	   cout << " Size = " << awd->size << endl;
	   cout << " Data = " << awd->data << endl;
	   */

	//hacked to send it as a messages type
	//TODO: make it own type (shmdtActuatorMessage) and channel ( socket, maybe)
	sendWirelessMessage(TCPindexB, shmdtActuatorMessage, (*count) * sizeof(ActuatorWirelessDataEntry), awd );
	observer[event.ObsIndex()]->AssertReady();

}

void Wireless::sendWirelessMessage(int channel, int type, int size, const char *data){
	if (tcpconnection[channel].state != CONNECTION_SENDING) {
#ifdef HEADER_ALIGNMENT
		strcpy(info.header, WDataHeader);
#endif
		info.robot = PLAYER_NUMBER;
		info.type = type;
		info.size = size;

		// copy the header
		memcpy(tcpconnection[channel].sendData, &info, sizeof(WDataInfo));

		// copy the data
		memcpy(tcpconnection[channel].sendData + sizeof(WDataInfo), data, info.size);

		// copy the size (header size + data size)
		tcpconnection[channel].sendSize = sizeof(WDataInfo) + info.size;

		TCPSend(channel);
	}
	else {
		cout << "connection busy" << endl;
	}
}


void Wireless::setPlayerNumber(const ONotifyEvent &event) {
//	cout << "Wireless::setPlayerNumber: ";
	PLAYER_NUMBER = *(const int *) event.Data(0);
	team = *(const int *) event.Data(1);
//	cout << PLAYER_NUMBER << " team: " << team << endl;
	observer[event.ObsIndex()]->AssertReady();
}


void Wireless::NetInfo(const ONotifyEvent &event) {
	cout << "Got NetInfo " << endl;

	WMShareInfo wi;
	wi = *static_cast<const WMShareInfo*>(event.Data(0));

#ifdef IR_LATENCY_TEST
	if (wi.obj.latMode == INIT_LAT_WM) {
		cout << "ping received from " << wi.playerNum << " ..." << endl;
		GetSystemTime(&latTime);
		latDog = wi.playerNum;
	}
	else if (wi.obj.latMode != NORMAL_WM) {
		if (wi.obj.latMode == PLAYER_NUMBER) {
			SystemTime temp;
			GetSystemTime(&temp);
			cout << "got time(temp) at " << temp.seconds << "s " << temp.useconds
				<< "us" << endl;
			SystemTime diff = temp - latTime;
			SystemTime diff2 = diff - wi.obj.waited;
			cout << "WMPing(" << wi.playerNum << ") - ";
			cout << diff2.seconds << "s " << diff2.useconds << "us" << endl;
		}
		else {
			cout << "ping " << wi.playerNum << "->" << wi.obj.latMode << endl;
		}
	}
#endif // IR_LATENCY_TEST

	ForwardNetInfo(&wi, sizeof(wi));

	observer[event.ObsIndex()]->AssertReady(event.SenderID());
}

/* Forwards information received over the net (UDP broadcast) to vision */
void Wireless::ForwardNetInfo(const void *data, size_t size) {
    
    // message too small, ingore it
    if (size < sizeof(worldModelHeader)) {
        cout << "ForwardNetInfo: ignoring short message (len " << size << ")"
            << endl;
        return;
        
    // forward a world model
    } else if (memcmp(data, worldModelHeader, sizeof(worldModelHeader)) == 0
            && size == sizeof(WMShareInfo)) {
        /* Forward a world model */
        const WMShareInfo* wi = (const WMShareInfo *)data;
        if (wi->playerNum == PLAYER_NUMBER) {
            return;	// ignore data from our own player number
        }
        if (wi->team != team) {
            cout << "Warning: ignoring recieved world model info from team "
                << wi->team << " player: " << wi->playerNum << endl;
            cout << "My team: " << team << " player: " << PLAYER_NUMBER << endl;
            return;
        }
                
        //cout << "Im receiving!!! team number!" << team << endl;

        // We require multiple sending channels as when we had a single channel
        // and getting wireless messages before we had processed previous ones
        // they were screwing up and messages were being dropped. While
        // debugging I never saw the third subject needed to used but I left it
        // in as a safeguard - Ross 11/06/03
        if (subject[sbjWorldModelReceived1]->IsAnyReady()) {
            subject[sbjWorldModelReceived1]->ClearBuffer();
            subject[sbjWorldModelReceived1]->SetData(data, size);
            subject[sbjWorldModelReceived1]->NotifyObservers();
        }
        else if (subject[sbjWorldModelReceived2]->IsAnyReady()) {
            subject[sbjWorldModelReceived2]->ClearBuffer();
            subject[sbjWorldModelReceived2]->SetData(data, size);
            subject[sbjWorldModelReceived2]->NotifyObservers();
        }
        else if (subject[sbjWorldModelReceived3]->IsAnyReady()) {
            subject[sbjWorldModelReceived3]->ClearBuffer();
            subject[sbjWorldModelReceived3]->SetData(data, size);
            subject[sbjWorldModelReceived3]->NotifyObservers();
        }
        
        /* no longer process game controller data here, it is passed on
           straight from the game controller */
        else {
            cout << "Warning: World model droppped - subject not ready" << endl;
        }
        
    // forward teamTalkMessage to vision, which forwards to Python
    } else if (memcmp(data, teamTalkHeader, sizeof(teamTalkHeader)) == 0) {        
        const TeamTalk* tt = static_cast<const TeamTalk*>(data);
        //cout << "Wireless received TeamTalk len " << tt->length << endl;
        subject[sbjTeamTalk]->ClearBuffer();
        subject[sbjTeamTalk]->SetData(&(tt->length), sizeof(tt->length));
        subject[sbjTeamTalk]->SetData(&(tt->message), tt->length);
        subject[sbjTeamTalk]->NotifyObservers();
                
    // anything else        
    } else {
        cout << "Warning: Ignoring unrecognised wireless data" << endl;
    }
}


/* Broadcasts the TeamTalk message containted in event.Data over UDP */
void Wireless::sendTeamTalk(const ONotifyEvent &event) {
    const int length = * static_cast<const int *>(event.Data(0));
    const char *message =  static_cast<const char *>(event.Data(1));
    
    int size = sizeof(teamTalkHeader) + INT_BYTES + length;

#ifdef USE_UDP
	if (udpconnection[UDPSendIndex].state == CONNECTION_CONNECTED) {		
		udpconnection[UDPSendIndex].sendSize = size;
		memcpy(udpconnection[UDPSendIndex].sendData, teamTalkHeader,
                sizeof(teamTalkHeader));
		memcpy(udpconnection[UDPSendIndex].sendData + sizeof(teamTalkHeader),
                &length, INT_BYTES);
		memcpy(udpconnection[UDPSendIndex].sendData + sizeof(teamTalkHeader)
                + INT_BYTES, message, length);

//        cout << "Wireless::sendTeamTalk sending message (len " << length
//        << "): '" << message << "'" << endl;
	
		UDPSend(UDPSendIndex);
	} else {
		cout << "Unable to send TeamTalk - UPD connection not connected" << endl;
	}

    observer[obsTeamTalk]->AssertReady();

#else
    cout << "Unable to send TeamTalk - USE_UDP is not defined" << endl;
#endif

}

void Wireless::readUDPPorts() {
	const char *filename = "/MS/udp.cfg";
	ifstream in(filename);

	if (in) {
		in >> udpRecPort >> udpSendPort;
		cout << "Broadcast to UDP Port: " << udpRecPort << endl;
		cout << "Assist UDP Port: " << udpSendPort << endl;
	}
}

