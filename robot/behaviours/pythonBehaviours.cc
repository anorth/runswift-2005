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
 * Last modification background information
 * $Id
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 * 
 * Beckham Forward
 *
 **/ 


#include "pythonBehaviours.h"
#include "Behaviours.h"

#ifdef COMPILE_ALL_CPP
#include "UNSW2004.h"
#endif // COMPILE_ALL_CPP

#ifndef OFFLINE
#include "../vision/PyEmbed.h"
#include "../vision/Vision.h"
#endif //OFFLINE

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>

using namespace Behaviours;
using namespace std;

static const bool bDebugDiscardedCommand = false;
static const int ALL_PLAYER = 9; 

namespace PythonBehaviours{
    long actionCounter = 0;
    vector<string> pythonText;
    bool gotHeader = false;
    bool reloadPython = false;
    bool alreadyInitialized = false;

    static bool runBehindBallFirstCall = true;

    void initPythonBehaviours(){
#ifndef OFFLINE
        if (!alreadyInitialized){
            PyBehaviours::initializeBehaviours();
            alreadyInitialized = true;
        }
        else{
            PyBehaviours::reloadModule();
        }
#endif //OFFLINE
        runBehindBallFirstCall = true;
    }


    void gotWirelessPythonCommand(const char *value){
        istringstream valueSS (value);
        int playerNo;
        valueSS >> playerNo;
        if ((uint32)playerNo != Behaviours::PLAYER_NUMBER && playerNo != ALL_PLAYER){
            if (bDebugDiscardedCommand)
                cout << "Discard command line" << value << endl;
        }
        else{
            static const int MAX_BUFFER_SIZE = 200;
            char buffer[MAX_BUFFER_SIZE];
            valueSS.getline(buffer,MAX_BUFFER_SIZE);
#ifndef OFFLINE
            // send it to python.
            PyBehaviours::processCommand(buffer, strlen(buffer));
#endif //OFFLINE
        }
    }

    /* Process a command received from base station over wireless (TCP) */
    void gotWirelessCommand(const char *value){
        istringstream valueSS (value);
        string header;
        int playerNo;
        valueSS >> header >> playerNo;
        cout << "Python behaviours got header " << header << " playerNo = " << playerNo << endl;
        if ((uint32)playerNo != Behaviours::PLAYER_NUMBER && playerNo != ALL_PLAYER){
            if (bDebugDiscardedCommand)
                cout << "Discard command line" << value << endl;
        }
        else{
            if (header == "reload"){
                reloadPython = true;
                cout << "Reloading python behaviours" << endl;
            }
            else if (header == "eval"){
                static const int MAX_BUFFER_SIZE = 200;
                char buffer[MAX_BUFFER_SIZE];
                valueSS.getline(buffer,MAX_BUFFER_SIZE);
#ifndef OFFLINE
                char* result = PyBehaviours::evaluateExpression(buffer);
                string message;
                if (result == NULL){
                    message = "Cannot evaluate";
                }
                else {
                    message = string(result);
                }

                message = "****" + message;
                transmission->sendWirelessMessage(message);
#endif              
            }
        }
    }

    void DecideNextAction(){
        actionCounter ++;
        setStandParams();
#ifndef OFFLINE
        if (reloadPython){
            PyBehaviours::reloadModule();
            reloadPython = false;
        }
        PyBehaviours::processFrame();
#endif //OFFLINE
    }

    void getBehindBall(){
#ifdef COMPILE_ALL_CPP
        static long internalCounter = 0;
        bool chooseDir = true;
        if (++internalCounter  == actionCounter){
            chooseDir = false;
        }
        else chooseDir = true;

        double awayFromOwnGoal = RAD2DEG(atan2(vBall.y - 0,
                    vBall.x - (FIELD_WIDTH / 2.0)));
        saFarGetBehindBall(chooseDir, awayFromOwnGoal);
        UNSW2004::doUNSW2004TrackVisualBall();
#endif //COMPILE_ALL_CPP
    }

    void runBehindBall(){

#ifdef COMPILE_ALL_CPP
        if (runBehindBallFirstCall)
            Behaviours::resetSaRunBehindBall();
        runBehindBallFirstCall = false;

        saRunBehindBall(20, 270, ANTICLOCKWISE, 20, 0);

#endif //COMPILE_ALL_CPP
    }

    void locateBall(){
        Behaviours::locateBall(FIELD_WIDTH/2.0, FIELD_LENGTH / 4.0);
    }
}

