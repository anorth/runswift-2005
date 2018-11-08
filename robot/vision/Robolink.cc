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


/* Robolink.cc
 * Methods for communication over the robolink
 */

#ifndef OFFLINE
#include "Vision.h"
#include "../behaviours/Behaviours.h"   // for player/team number
#endif

#include "Robolink.h"


/* Static class members */
Vision* Robolink::vision;
int Robolink::conn_id;

Robolink::Robolink() {
    conn_id = -1;
}

void Robolink::initialize(Vision* v) {
    if (Robolink::vision) {
        cout << "ERROR: Robolink is singleton but initialize called twice"
            << endl;
        return;
    }
    Robolink::vision = v;
//    cout << "Robolink::initialize(). Vision = " << (void*) vision << endl;

#ifndef OFFLINE
    vision->TCPListen(ROBOLINK_DOG_PORT, &conn_id);
#endif
}

/* Send data of the specifed type and length to the base over Robolink.
 * If frame is not specified the current vision frame is assumed 
 */
void Robolink::sendToBase(byte *data, enum data_type type, unsigned int length,
                            int frame) {
    OStatus status = oFAIL;
    robolink_header header;

    if (conn_id < 0 || ! vision) {
        cout << "sendToBase: robolink not initialized" << endl;
    }
    if (frame == 0) {
#ifndef OFFLINE
        frame = vision->getFrameID();
    }
//    cerr << "Robolink::sendToBase, type " << type << " len " << length
//        << " frame " << frame << endl;

    memcpy(header.magic, ROBOLINK_MAGIC, sizeof(ROBOLINK_MAGIC));
    header.team_num = Behaviours::TEAM_NUMBER;
    header.player_num = Behaviours::PLAYER_NUMBER;
    header.frame_num = frame;
    header.data_type = type;
    header.data_len = length;

    //status = vision->TCPSend(conn_id, (byte *)&header, sizeof(header));
    status = vision->TCPQueue(conn_id, (byte *)&header, sizeof(header));
#endif

    // don't send the data if the header didn't send ok
    if (status != oSUCCESS || length <= 0)
        return;

#ifndef OFFLINE
    //vision->TCPSend(conn_id, data, length);
    status = vision->TCPQueue(conn_id, data, length);
    if (status != oSUCCESS) {
		OSYSLOG1((osyslogERROR, "Robolink::sendToBase(): data send failed "
                    "after header sent ok"));
		return;
    }
#endif
}


