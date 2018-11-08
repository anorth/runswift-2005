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
 * $Id: PyEmbed.cc 7404 2005-07-07 11:13:37Z alexn $
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 *
**/

#include "Python.h"
#include "PyEmbed.h"

#include <iostream>
#include <sstream>

#include "../share/IndicatorsDef.h"
#include "../share/Common.h"
#include "../vision/CommonSense.h"
#include "../share/FileUtility.h"

#ifdef COMPILE_ALL_CPP
#include "../behaviours/UNSW2004.h"
#endif // COMPILE_ALL_CPP

#include "../behaviours/Behaviours.h"
#include "../behaviours/pythonBehaviours.h"
#include "../behaviours/WalkingLearner.h"
#include "InfoManager.h"
#include "PointProjection.h"
#include "Profile.h"
#include <stdlib.h>
#include <time.h>


#ifdef OFFLINE
#include <sys/time.h>
#include "offlineVision.h"
#endif // OFFLINE

using namespace std;

static PyObject *initVisionLinkModule(void);  // forward reference
static void addConstants(PyObject *module);

// see Python-Docs-2.3.3/ext/callingPython.html
// and Python-Docs-2.3.3/api/veryhigh.html

static char *VisionLinkModuleName = "VisionLink";
static char *BehaviourModuleName = "Behaviour";                                 

static const bool debugCouts = false;

static PyObject *behaviourModule = NULL;

static PyObject *ProcessFramePyFunc = NULL;
static PyObject *ProcessCommandPyFunc = NULL;


namespace PyBehaviours {
#ifndef OFFLINE
    Vision *transmission;
#endif	// OFFLINE
    VisualCortex *vision;
    Obstacle *obstacle;
    GPS *gps;
    CommonSense *sensors;
    struct PWalkInfoStruct *PWalkInfo;
    
    void initializeBehaviours() {
        cout << "Initializing Python Behaviours" << endl;

        if (ProcessFramePyFunc != NULL) {
            Py_DECREF(ProcessFramePyFunc);
            ProcessFramePyFunc = NULL;
        }
        if (ProcessCommandPyFunc != NULL) {
            Py_DECREF(ProcessCommandPyFunc);
            ProcessCommandPyFunc = NULL;
        }
        if (Py_IsInitialized()) {
            cout << "Finalizing previous Python Behaviours" << endl;
            Py_Finalize();
        }

        Py_VerboseFlag += 2;

        Py_Initialize();

        cout << "Importing VisionLink" << endl;

        PyObject *module = initVisionLinkModule();

        addConstants(module);

        cout << "Importing " << BehaviourModuleName << endl;

        if ( (behaviourModule = PyImport_ImportModule(BehaviourModuleName) ) == NULL) {
            cout << "Error Importing Behaviours" << endl;
        }

        cout << "Done initializing Python Behaviours" << endl;
    }

    void processFrame() {
        if (ProcessFramePyFunc == NULL) {
            cout
                << "PyBehaviours::processFrame(): Python callback not initialized.  Unable to call it."
                << endl;
            return;
        }

        PyObject *args = Py_BuildValue("()");
        if (args == NULL) {
            cout << "PyBehaviours::processFrame(): Unable to build args" << endl;
            if (PyErr_Occurred() == NULL) {
                cout << "But no Python exception available!?!" << endl;
            }
            else {
                PyErr_Print();
            }
            return;
        }
        PyObject *result = PyEval_CallObject(ProcessFramePyFunc, args);
        Py_DECREF(args);
        if (result == NULL) {
            cout << "PyBehaviours::processFrame(): Got NULL from python call"
                << endl;
            if (PyErr_Occurred() == NULL) {
                cout << "But no Python exception available!?!" << endl;
            }
            else {
                PyErr_Print();
#ifndef OFFLINE
                send_data[RLNK_DROPFRAME] = false; // disable frame dropping telnet print outs
#endif
            }
        }
        else {
            Py_DECREF(result);
        }
    }

    void processCommand(const char *cmd, int length) {
        //cout << "PyBehaviours::processCommand" << endl;
        if (ProcessCommandPyFunc == NULL) {
            cout
                << "PyBehaviours::processCommand(): Python callback not initialized.  Unable to call it."
                << endl;
            return;
        }

        // cmd can contain null bytes when using the s# parse token. This
        // means you can transmit things like pickled Python objects :)
        //cout << "  building args" << endl;
        PyObject *args = Py_BuildValue("(s#)", cmd, length);
              
        if (args == NULL) {
            cout << "PyBehaviours::processCommand(): Unable to build args" << endl;
            if (PyErr_Occurred() == NULL) {
                cout << "But no Python exception available!?!" << endl;
            }
            else {
                PyErr_Print();
            }
            return;
        }
        //cout << "  calling processCommandPyFunc" << endl;
        PyObject *result = PyEval_CallObject(ProcessCommandPyFunc, args);
        Py_DECREF(args);
        if (result == NULL) {
            cout << "PyBehaviours::processCommand(): Got NULL from python call"
                << endl;
            if (PyErr_Occurred() == NULL) {
                cout << "But no Python exception available!?!" << endl;
            }
            else {
                PyErr_Print();
            }
        }
        else {
            Py_DECREF(result);
        }
        //cout << "  done." << endl;
    }

    void runString(char *cmd) {
        PyRun_SimpleString(cmd);
    }

    void deleteAllPYCFiles(){
#ifndef OFFLINE
        static const char *PyDir = "/MS/PyCode";
        vector<string> fileList = listAllFiles( PyDir );
        for (unsigned int i=0;i<fileList.size();i++){
            if (fileType(fileList[i]) == "PYC"){
                deleteFile((string(PyDir) + string("/") + fileList[i]).c_str());
            }
        }
#endif
    }

    void reloadModule(){
        if (behaviourModule == NULL){
            cout << "Error : PyBehaviour module is not initialized" << endl;
        }
        else{
            deleteAllPYCFiles();
            PyObject *oldModule = behaviourModule;
            behaviourModule = PyImport_ReloadModule(behaviourModule);
            if (behaviourModule == NULL){
                cout << "Error : New Python Behaviour module can not be loaded, revert to old module" << endl;
                behaviourModule = oldModule;
            }
            else {
                cout << "New python behaviours reloaded successfully" << endl;
            }
        }
    }

    //This code is acknowledged to Rick L. Ratzel
    //http://elmer.sourceforge.net/PyCon04/elmer_pycon04.html 
    //however, doesn't work, I ended up using sendMessageToBase, which is more flexible
    // Sunny: turns out its used in behaviours/pythonBehaviours.cc
    char *evaluateExpression(const char *expression){
        static const int BUFFER_SIZE = 500;
        char buffer[BUFFER_SIZE];

        //PyObject* evalModule;
        PyObject* evalDict;
        PyObject* evalVal;
        //char* retString;

        sprintf(buffer, "result = `%s`", expression);
        cout << "Running " << buffer << endl;

        evalDict = PyModule_GetDict( behaviourModule );
        evalVal = PyDict_GetItemString( evalDict, "result" );

        if( evalVal == NULL ) {
            return NULL;
        } else {
            /*
             * PyString_AsString returns char* repr of PyObject, which should
             * not be modified in any way...this should probably be copied for
             * safety
             */
            return PyString_AsString( evalVal );
        }
    }
}

/*
 * Start porting global constants into module
 */

static void addConstants(PyObject *module){
    Profile::start(PR_BE_VISLINK, 1);
    PyObject *dic = PyModule_GetDict(module);

    PyObject *tmp;
    tmp = PyInt_FromLong(NUM_OF_EFFECTORS);
    PyDict_SetItemString(dic, "NUM_OF_EFFECTORS", tmp);
    Py_DECREF(tmp);

    tmp = PyInt_FromLong(NUM_SENSORS);
    PyDict_SetItemString(dic, "NUM_SENSORS", tmp);
    Py_DECREF(tmp);

    tmp = PyInt_FromLong(NUM_TEAM_MEMBER);
    PyDict_SetItemString(dic, "NUM_TEAM_MEMBER", tmp);
    Py_DECREF(tmp);

    tmp = PyFloat_FromDouble(BALL_RADIUS);
    PyDict_SetItemString(dic, "BALL_RADIUS", tmp);
    Py_DECREF(tmp);

    tmp = PyInt_FromLong(FIELD_WIDTH);
    PyDict_SetItemString(dic, "FIELD_WIDTH", tmp);
    Py_DECREF(tmp);

    tmp = PyInt_FromLong(FIELD_LENGTH);
    PyDict_SetItemString(dic, "FIELD_LENGTH", tmp);
    Py_DECREF(tmp);

    tmp = PyInt_FromLong(WALL_THICKNESS);
    PyDict_SetItemString(dic, "WALL_THICKNESS", tmp);
    Py_DECREF(tmp);

    tmp = PyInt_FromLong(GOAL_WIDTH);
    PyDict_SetItemString(dic, "GOAL_WIDTH", tmp);
    Py_DECREF(tmp);

    tmp = PyInt_FromLong(GOAL_LENGTH);
    PyDict_SetItemString(dic, "GOAL_LENGTH", tmp);
    Py_DECREF(tmp);
    Profile::stop(PR_BE_VISLINK);
}

/*
 * Start porting static python functions
 */
                                               

using namespace PyBehaviours;

/*
 * Helper functions to convert types from C++ to Python
 */

static PyObject * PyObject_FromWMObject(const WMObj &obj){
    Profile::start(PR_BE_VISLINK, 2);
    PyObject *t;

    t = PyTuple_New(5);
    PyTuple_SetItem(t, 0, PyFloat_FromDouble(obj.pos.x) );
    PyTuple_SetItem(t, 1, PyFloat_FromDouble(obj.pos.y) );
    PyTuple_SetItem(t, 2, PyFloat_FromDouble(obj.posVar) );
    PyTuple_SetItem(t, 3, PyFloat_FromDouble(obj.h));
    PyTuple_SetItem(t, 4, PyFloat_FromDouble(obj.hVar));
    
    Profile::stop(PR_BE_VISLINK);
    return t;
}

static PyObject * PyObject_FromVector(const Vector &vector){
    Profile::start(PR_BE_VISLINK, 4);
    PyObject *t;
                                               
    t = PyTuple_New(6);
    PyTuple_SetItem(t, 0, PyFloat_FromDouble(vector.x) );
    PyTuple_SetItem(t, 1, PyFloat_FromDouble(vector.y) );
    PyTuple_SetItem(t, 2, PyFloat_FromDouble(vector.d) );
    PyTuple_SetItem(t, 3, PyFloat_FromDouble(vector.theta));
    PyTuple_SetItem(t, 4, PyFloat_FromDouble(vector.head));
    PyTuple_SetItem(t, 5, PyFloat_FromDouble(vector.angleNorm));
    
    Profile::stop(PR_BE_VISLINK);
    return t;
}

static PyObject * PyObject_FromCoordArray(const coordArray *coord){
    Profile::start(PR_BE_VISLINK, 5);
    PyObject *t;

    t = PyTuple_New(8);
    for (int i=0;i<8;i++){
        PyObject *pos = PyTuple_New(2);
        PyTuple_SetItem(pos,0, PyInt_FromLong((*coord)[i][0]));
        PyTuple_SetItem(pos,1, PyInt_FromLong((*coord)[i][1]));
        PyTuple_SetItem(t,i,pos);
    }
    Profile::stop(PR_BE_VISLINK);
    return t;
}

static PyObject * PyObject_From3x3Maxtrix(const MMatrix3 &maxtrix){
    Profile::start(PR_BE_VISLINK, 6);
    PyObject *t;

    t = PyTuple_New(3);
    for (int i=0;i<3;i++){
        PyObject *row = PyTuple_New(3);

        for (int j=0;j<3;j++)
            PyTuple_SetItem(row, j, PyFloat_FromDouble(maxtrix(i,j)));
        PyTuple_SetItem(t,i,row);
    }
    
    Profile::stop(PR_BE_VISLINK);
    return t;
}

static PyObject * VisionLink_dummy(PyObject * /*self*/, PyObject *args) {
    Profile::start(PR_BE_VISLINK, 7);
    if (debugCouts) {
        cout << "oh no, your're runnig dummy function " << endl;
    }

    PyErr_Format(PyExc_RuntimeError,
            "%s not completely implemented in file %s at line %d.",
            __PRETTY_FUNCTION__,
            __FILE__,
            __LINE__);

    Profile::stop(PR_BE_VISLINK);
    return NULL;
}

static PyObject * VisionLink_setCallbacks(PyObject * /* self */, PyObject *args) {
    Profile::start(PR_BE_VISLINK, 8);
    PyObject *result = NULL;
    PyObject *processFramePtr;
    PyObject *processCmdPtr;

    cout << "Setting Python VisionLink callbacks" << endl;

    if (PyArg_ParseTuple(args,
                "OO:setCallbacks",
                &processFramePtr,
                &processCmdPtr)) {
        if (!PyCallable_Check(processFramePtr) || !PyCallable_Check(processCmdPtr)) {
            PyErr_SetString(PyExc_TypeError, "parameter must be callable");
            Profile::stop(PR_BE_VISLINK);
            return NULL;
        }
        Py_XINCREF(processFramePtr);         /* Add a reference to new callback */
        if (ProcessFramePyFunc != NULL) {
            Py_DECREF(ProcessFramePyFunc);  /* Dispose of previous callback */
        }
        ProcessFramePyFunc = processFramePtr;       /* Remember new callback */
        Py_XINCREF(processCmdPtr);         /* Add a reference to new callback */
        if (ProcessCommandPyFunc != NULL) {
            Py_DECREF(ProcessCommandPyFunc);  /* Dispose of previous callback */
        }
        ProcessCommandPyFunc = processCmdPtr;       /* Remember new callback */
        /* Boilerplate to return "None" */
        Py_INCREF(Py_None);
        result = Py_None;
    }
    
    Profile::stop(PR_BE_VISLINK);
    return result;
}
/*  Set the atomic action. */
static PyObject * VisionLink_sendAtomicAction(PyObject * /*self*/, PyObject *args) {
    Profile::start(PR_BE_VISLINK, 9);
    //struct AtomicAction thisAA;
    long cmd;

    if (debugCouts) {
        cout << "starting send of atomic action" << endl;
    }

    if (PyArg_ParseTuple(args,
                "liiddddddiddd:sendAtomicAction",
                &cmd,
                &Behaviours::walkType ,
                &Behaviours::minorWalkType ,
                &Behaviours::forward ,
                &Behaviours::left ,
                &Behaviours::turnCCW ,
                &Behaviours::forwardMaxStep ,
                &Behaviours::leftMaxStep , 
                &Behaviours::turnCCWMaxStep ,
                &Behaviours::headtype,
                &Behaviours::panx,
                &Behaviours::tilty,
                &Behaviours::cranez
                )) {

      // HACK: is the command is setting high / low gain, then continue
      // the motion, but hack the cmd back to aaParaWalk
        if (cmd == aaSetHighGain || cmd == aaSetLowGain) {
            AtomicAction aa ;
            aa.cmd      = cmd;
            //cout << "eWHICH JOINTS: "<< aa.walkType << endl;
#ifndef OFFLINE
            transmission->sendAA(aa);
#endif //OFFLINE
            Behaviours::acCmd = aaParaWalk;
        }
        else{
            Behaviours::acCmd = cmd;
        }
        Py_INCREF(Py_None);
        
        Profile::stop(PR_BE_VISLINK);
        return Py_None;
    }
    
    Profile::stop(PR_BE_VISLINK);
    return NULL;
}

/*  Set the LEDs, tail and ears. */
static PyObject * VisionLink_sendIndicators(PyObject * /*self*/, PyObject *args) {
    Profile::start(PR_BE_VISLINK, 10);
    if (debugCouts) {
        cout << "starting sendIndicators" << endl;
    }

    int inds[NUM_OF_INDICATORS];

    if (PyTuple_Size(args) != NUM_OF_INDICATORS) {
        PyErr_Format(PyExc_RuntimeError,
                "Incorrect number of arguments to sendIndicators.  Should be %d",
                (int)
                NUM_OF_INDICATORS);                                               

        Profile::stop(PR_BE_VISLINK);
        return NULL;
    }

    for (int i = 0; i < NUM_OF_INDICATORS; i++) {
        PyObject *thisInd = PyTuple_GetItem(args, i);
        if (thisInd == NULL) {
            Profile::stop(PR_BE_VISLINK);
            return NULL;   
        }
        inds[i] = PyInt_AsLong(thisInd);
        if (PyErr_Occurred()) {
            Profile::stop(PR_BE_VISLINK);
            return NULL;
        }
    }	

#ifndef OFFLINE

    // Turn on the wireless indicator if a packet is received. ind[24] is the wireless indicator.
	if (transmission->gotTMWM) {
		inds[24] = IND_LED2_ON;
		transmission->gotTMWM = 0;
	} else {
		inds[24] = IND_LED2_OFF;
	}

	// If there are three or more teammates with valid world models turn the wireless indicator on.
	int tm = 0;
	for (int i=0 ; i < NUM_TEAM_MEMBER ; i++) {
		if (gps->tmObj(i).counter > 0) {
			tm++;
		}
	}
	if (tm >= 3) {
		inds[24] = IND_LED2_ON;
	} 
	
    transmission->sendIndicators(inds, sizeof(inds));

#endif // OFFLINE

    Py_INCREF(Py_None);
    
    Profile::stop(PR_BE_VISLINK);
    return Py_None;
}

static PyObject * VisionLink_resetGPS(PyObject * /*self*/, PyObject *args) {
    Profile::start(PR_BE_VISLINK, 11);
    
    gps->resetGaussians();
    
    Profile::stop(PR_BE_VISLINK);
    Py_XINCREF(Py_None);
    return Py_None;
}

/* Send wireless back to base station, prefix by "****" */
static PyObject * VisionLink_sendMessageToBase(PyObject * /*self*/, PyObject *args) {
    Profile::start(PR_BE_VISLINK, 12);

    char *message;
    if (PyArg_ParseTuple(args,
                "s:sendMessageToBase",&message)){
#ifndef OFFLINE
        transmission->sendWirelessMessage(message);
#endif // OFFLINE
    }
    else {
        Profile::stop(PR_BE_VISLINK);
        return NULL;
    }

    Py_INCREF(Py_None);
    
    Profile::stop(PR_BE_VISLINK);
    return Py_None;
}
/* Send my behaviours to teammates via gps */
static PyObject * VisionLink_sendMyBehaviourInfo(PyObject * /*self*/, PyObject *args) {
    Profile::start(PR_BE_VISLINK, 13);

    int timeToReachBall;
    bool hasGrabbedBall;
    bool hasSeenBall;
    bool hasLostBall;
    int role;
    int roleCounter;
    interpBehav info;
    if (PyArg_ParseTuple(args,
                         "ibbbii:sendMyBehaviourInfo",
                         &timeToReachBall,
                         &hasGrabbedBall,
                         &hasSeenBall,
                         &hasLostBall,
                         &role,
                         &roleCounter)) {
    
        info.timeToReachBall = short(timeToReachBall);
        info.hasGrabbedBall  = int(hasGrabbedBall);
        info.hasSeenBall     = int(hasSeenBall);
        info.hasLostBall     = int(hasLostBall);
        info.role            = int(role);
        info.roleCounter     = int(roleCounter);
        gps->sharedBehaviour() = info.convert();
        
    } else {
        Profile::stop(PR_BE_VISLINK);
        return NULL;
    }
    
    Py_INCREF(Py_None);
    
    Profile::stop(PR_BE_VISLINK);
    return Py_None;
}

/* Toggle sending of a particular data type */
static PyObject * VisionLink_sendData(PyObject * /*self*/, PyObject *args) {
    Profile::start(PR_BE_VISLINK, 14);
    
#ifndef OFFLINE    
    int dataType;
    if (PyArg_ParseTuple(args, "i", &dataType)) {
        // set SEND_ENABLED and toggle the requested type
        send_data[RLNK_ALL] = true;
        send_data[dataType] = ! send_data[dataType];
    } else {
        Profile::stop(PR_BE_VISLINK);
        return NULL;
    }
#endif    

    Py_INCREF(Py_None);
    
    Profile::stop(PR_BE_VISLINK);
    return Py_None;
}

/*  Send our world model to our teammates. */
static PyObject * VisionLink_sendEnvironmentTeamMates(PyObject * /*self*/,
        PyObject *args) {
    Profile::start(PR_BE_VISLINK, 15);    
        
    if (debugCouts) {
        cout << "starting sendEnvironmentTeamMates" << endl;
    }

#ifndef OFFLINE
    transmission->shareEnvironment();
#endif // OFFLINE
    Py_INCREF(Py_None);
    
    Profile::stop(PR_BE_VISLINK);
    return Py_None;
}

/*  Send the CPlane to the base station. */
static PyObject * VisionLink_sendCompressedCPlane(PyObject * /*self*/,
        PyObject *args) {
    Profile::start(PR_BE_VISLINK, 16);    
        
    if (debugCouts) {
        cout << "starting sendCompressedCPlane" << endl;
    }

#ifndef OFFLINE
    CorrectedImage::fillCPlane();
    CorrectedImage::sendCPlane();
#endif // OFFLINE
    Py_INCREF(Py_None);
    
    Profile::stop(PR_BE_VISLINK);
    return Py_None;
}

/*  Send the OPlane to the base station. */
static PyObject * VisionLink_sendOPlane(PyObject * /*self*/, PyObject *args) {
    Profile::start(PR_BE_VISLINK, 17);

    if (debugCouts) {
        cout << "starting sendOPlane" << endl;
    }
    // disabled. Should make this send SubObject
    Py_INCREF(Py_None);
    
    Profile::stop(PR_BE_VISLINK);
    return Py_None;
}

/*  Send the world model to the base station. */
static PyObject * VisionLink_sendEnvironmentBaseStation(PyObject * /*self*/,
        PyObject *args) {
    Profile::start(PR_BE_VISLINK, 18);
        
    if (debugCouts) {
        cout << "starting sendEnvironmentBaseStation" << endl;
    }

#ifndef OFFLINE
    transmission->displayEnvironment();
#endif // OFFLINE
    Py_INCREF(Py_None);
    
    Profile::stop(PR_BE_VISLINK);
    return Py_None;
}

/*  Send the YUV plane to robolink */
static PyObject * VisionLink_sendYUVPlane(PyObject * /*self*/, PyObject *args) {
    Profile::start(PR_BE_VISLINK, 19);

    if (debugCouts) {
        cout << "starting sendYUVPlane" << endl;
    }

#ifndef OFFLINE
    transmission->sendYUVPlane();
#endif // OFFLINE
    Py_INCREF(Py_None);
    
    Profile::stop(PR_BE_VISLINK);
    return Py_None;
}

/*  Send the PWalk Info and current atomic action to robolink. */
static PyObject * VisionLink_sendWalkInfo(PyObject * /*self*/, PyObject *args) {   
  Profile::start(PR_BE_VISLINK, 20);
 
  int timeElapsed = 0;
  if (!PyArg_ParseTuple(args, "i", &timeElapsed))
    timeElapsed = 0;

    if (debugCouts) {
        cout << "starting sendWalkInfoPlane" << endl;
    }
#ifndef OFFLINE
    vision->sendPWalkSensors();
    transmission->sendPythonAction(timeElapsed);
#endif // OFFLINE
    Py_INCREF(Py_None);
    
    Profile::stop(PR_BE_VISLINK);
    return Py_None;
}

/*  Send OPlane to robolink. */
static PyObject * VisionLink_sendObjects(PyObject * /*self*/, PyObject *args) {
    Profile::start(PR_BE_VISLINK, 30);

    if (debugCouts) {
        cout << "starting sendObjects" << endl;
    }
#ifndef OFFLINE
    vision->sendObjects();
#endif // OFFLINE
    Py_INCREF(Py_None);
    
    Profile::stop(PR_BE_VISLINK);
    return Py_None;
}


/* Broadcast a behaviour coordination message */
static PyObject * VisionLink_sendTeamTalk(PyObject * /*self*/, PyObject *args) {
    Profile::start(PR_BE_VISLINK, 31);

    int length;
    char *message;
    if (PyArg_ParseTuple(args, 
                "s#:sendTeamTalk", &message, &length)){ // may contain '\0's
#ifndef OFFLINE
        transmission->sendTeamTalk(message, length); // send to Vision
#endif // OFFLINE
    }
    else {
        Profile::stop(PR_BE_VISLINK);
        return NULL;
    }

    Py_INCREF(Py_None);
    
    Profile::stop(PR_BE_VISLINK);
    return Py_None;
}

/*  Get the robot's battery level. */
static PyObject * VisionLink_getBatteryLevel(PyObject * /*self*/,
        PyObject *args) {
    Profile::start(PR_BE_VISLINK, 32);    
        
    if (debugCouts) {
        cout << "starting getBatteryLevel" << endl;
    }
    int batLevel = 50;
#ifndef OFFLINE
    OStatus result;
    OPowerStatus currentStatus;
    result = OPENR::GetPowerStatus(&currentStatus);
    
    if (result == oSUCCESS) {
        PyObject *rc = PyInt_FromLong(currentStatus.remainingCapacity);
        Profile::stop(PR_BE_VISLINK);  
        return rc;
    }
    else {
        // Return an invalid value.
        PyObject *inv = PyInt_FromLong(-3);
        Profile::stop(PR_BE_VISLINK);
        return inv;
    }
#endif // OFFLINE

    PyObject *bl = PyInt_FromLong(batLevel);
    
    Profile::stop(PR_BE_VISLINK);
    return bl;
}

static PyObject * VisionLink_getBatteryCurrent(PyObject * /*self*/,
        PyObject *args) {
        
    Profile::start(PR_BE_VISLINK, 33);    

#ifndef OFFLINE
    OStatus result;
    OPowerStatus currentStatus;

    result = OPENR::GetPowerStatus(&currentStatus);
    if (result != oSUCCESS) {
        PyErr_Format(PyExc_RuntimeError,
                "Error getting power status: %d",
                (int) result);
                
        Profile::stop(PR_BE_VISLINK);        
        return NULL;
    }
    
    PyObject *cs = PyInt_FromLong(-currentStatus.current);
    Profile::stop(PR_BE_VISLINK);
    return cs;
#else // OFFLINE
    PyObject *cs = PyInt_FromLong(1500);
    Profile::stop(PR_BE_VISLINK);
    return cs;
#endif // OFFLINE
}

/*  Get the state of the robot's touch sensors. */
static PyObject * VisionLink_getPressSensorCount(PyObject * /*self*/,
        PyObject *arg) {
    Profile::start(PR_BE_VISLINK, 34);    
        
    if (debugCouts) {
        cout << "starting getPressSensorCount" << endl;
    }

    int sensorID = PyInt_AsLong(arg);

    if (PyErr_Occurred()) {
        Profile::stop(PR_BE_VISLINK);
        return NULL;
    }

    if (sensorID <0 || sensorID> NUM_PRESSES) {
        PyErr_Format(PyExc_RuntimeError, "Bad press sensor ID: %d", sensorID);
        Profile::stop(PR_BE_VISLINK);
        return NULL;
    }

    PyObject *s = PyInt_FromLong(sensors->press_sensor[sensorID].count);
    
    Profile::stop(PR_BE_VISLINK);
    return s;
}

/*  Get the state of the robot's switch sensors. */
static PyObject * VisionLink_getSwitchSensorCount(PyObject * /*self*/,
        PyObject *arg) {
    Profile::start(PR_BE_VISLINK);    
        
    if (debugCouts) {
        cout << "starting getSwitchSensorCount" << endl;
    }

    int sensorID = PyInt_AsLong(arg);

    if (PyErr_Occurred()) {
        Profile::stop(PR_BE_VISLINK);
        return NULL;
    }

    if (sensorID <0 || sensorID> NUM_SWITCHES) {
        PyErr_Format(PyExc_RuntimeError, "Bad switch sensor ID: %d", sensorID);
        Profile::stop(PR_BE_VISLINK);
        return NULL;
    }

    PyObject *s = PyInt_FromLong(sensors->switch_sensor[sensorID].count);
    
    Profile::stop(PR_BE_VISLINK);
    return s;
}


/*  Have we fallen over? */
static PyObject * VisionLink_getFallenState(PyObject * /*self*/, PyObject *args) {
    Profile::start(PR_BE_VISLINK, 35);

    if (debugCouts) {
        cout << "starting getFallenState" << endl;
    }

    PyObject *hf = PyInt_FromLong(sensors->hasFallen);
    
    Profile::stop(PR_BE_VISLINK);
    return hf;
}

/*  Tell GPS which goal is ours. */
static PyObject * VisionLink_setGPSGoal(PyObject * /*self*/, PyObject *arg) {
    Profile::start(PR_BE_VISLINK, 36);
    
    if (debugCouts) {
        cout << "starting setGPSGoal" << endl;
    }

    bool goalID = (bool)PyInt_AsLong(arg);

    if (PyErr_Occurred()) {
        Profile::stop(PR_BE_VISLINK);
        return NULL;
    }

    gps->SetGoals(goalID);
    // might want to fix this in future...
#ifndef OFFLINE
    transmission->sendPlayerNumber(Behaviours::PLAYER_NUMBER, goalID);
#endif // OFFLINE

    Py_INCREF(Py_None);
    
    Profile::stop(PR_BE_VISLINK);
    return Py_None;
}

/*  Pause GPS updates.  Useful when penalised. */                                               

static PyObject * VisionLink_setGPSPaused(PyObject * /*self*/, PyObject *arg) {
    Profile::start(PR_BE_VISLINK, 37);
    
    if (debugCouts) {
        cout << "starting setGPSPaused" << endl;
    }

    int penal = PyInt_AsLong(arg);

    if (PyErr_Occurred()) {
        Profile::stop(PR_BE_VISLINK);
        return NULL;
    }

    gps->setPenalised((penal != 0));

    Py_INCREF(Py_None);
    
    Profile::stop(PR_BE_VISLINK);
    return Py_None;
}


/*  Get information on the ball. */
static PyObject * VisionLink_getGPSBallInfo(PyObject * /*self*/, PyObject *args) {
    Profile::start(PR_BE_VISLINK, 38);

    int context;

    if (!PyArg_ParseTuple(args, "i", &context) || (context != GLOBAL && context != LOCAL) ) {
        Profile::stop(PR_BE_VISLINK);
        Py_XINCREF(Py_None);
        return Py_None;
    }

    PyObject *gb = PyObject_FromVector(gps->getBall(context));
    
    Profile::stop(PR_BE_VISLINK);
    return gb;
}

/*  Get information on the ball's velocity. */
static PyObject * VisionLink_getGPSBallVInfo(PyObject * /*self*/,
        PyObject *args) {
    Profile::start(PR_BE_VISLINK, 39);    
        
    int context;
    if (!PyArg_ParseTuple(args, "i", &context) || (context != GLOBAL && context != LOCAL) ) {
        Profile::stop(PR_BE_VISLINK);
        Py_XINCREF(Py_None);
        return Py_None;
    }

    PyObject *bv = PyObject_FromVector(gps->getVBall(context));
    
    Profile::stop(PR_BE_VISLINK);
    return bv;
}

/*  Get information on an opponent. */
static PyObject * VisionLink_getGPSOpponentInfo(PyObject * /*self*/,
        PyObject *args) {
    Profile::start(PR_BE_VISLINK, 40);    
        
    int context, opponentNumber;
    if (!PyArg_ParseTuple(args, "ii", &context, &opponentNumber) || (context != GLOBAL && context != LOCAL) || opponentNumber<1 || opponentNumber > 4 ) {
        Profile::stop(PR_BE_VISLINK);
        Py_XINCREF(Py_None);
        return Py_None;
    }
    
    PyObject *oi = PyObject_FromVector(gps->getOppPos(opponentNumber-1, context));
    
    Profile::stop(PR_BE_VISLINK);
    return oi;
}

static PyObject * VisionLink_getGPSOpponentCoxMax(PyObject * /*self*/,
        PyObject *args) {
    Profile::start(PR_BE_VISLINK, 41);    
        
    int opponentNumber;
    if (!PyArg_ParseTuple(args, "i", &opponentNumber) || opponentNumber < 1 || opponentNumber > 4 ) {
        Profile::stop(PR_BE_VISLINK);
        Py_XINCREF(Py_None);
        return Py_None;
    }
    
    PyObject *ocm = PyFloat_FromDouble(gps->getOppCovMax(opponentNumber-1));
    
    Profile::stop(PR_BE_VISLINK);
    return ocm;
}

static PyObject * VisionLink_getGPSBallMaxVar(PyObject * /*self*/,
        PyObject *args) {
    Profile::start(PR_BE_VISLINK, 42);    
    
    PyObject *bmv = PyFloat_FromDouble(gps->getBallMaxVar());
    
    Profile::stop(PR_BE_VISLINK);
    return bmv;
}

#if 0
static PyObject * VisionLink_getGPSVBallMaxVar(PyObject * /*self*/,
        PyObject *args) {
    return PyFloat_FromDouble(gps->getVBallMaxVar()) ;
}
#endif

// OBSTACLE FUNCTIONS
// --------------------------------------------------------------

static PyObject * VisionLink_ObstacleGPSValid(PyObject * /*self*/,
        PyObject *args) 
{
    Profile::start(PR_BE_VISLINK_OBS, 120);   
    
    PyObject *r;
    if (obstacle->ObstacleGPSValid()) {
        r = PyInt_FromLong(1);
    }
    else {
        r = PyInt_FromLong(0); 
    }
    
    Profile::stop(PR_BE_VISLINK_OBS);   
    return r;

} 

static PyObject * VisionLink_getNoObstacleInBox(PyObject * /*self*/,
        PyObject *args) 
{
    Profile::start(PR_BE_OBS_BOX, 121);   
    
    int topleftx; 
    int toplefty;
    int bottomrightx;
    int bottomrighty;
    int minObstacle = MIN_OBSTACLE;
    int flags = OBS_USE_DEFAULT;
    
    if (obstacle == NULL)
    {
        FCOUT << "Obstacle object is NULL" << endl;
        Profile::stop(PR_BE_OBS_BOX);  
        Py_XINCREF(Py_None);
        return Py_None; 
    }
    if (!PyArg_ParseTuple(args, "iiii|ii", &topleftx, &toplefty, 
    &bottomrightx, &bottomrighty, &minObstacle, &flags) ) 
    {
        FCOUT << "from C++, wrong input argument!" << endl;
        Profile::stop(PR_BE_OBS_BOX); 
        Py_XINCREF(Py_None);
        return Py_None; 
    }
        
    PyObject *oib = PyInt_FromLong(obstacle->getNoObstacleInBox(topleftx, toplefty, bottomrightx, bottomrighty,
    minObstacle, flags));
    
    Profile::stop(PR_BE_OBS_BOX);   
    return oib;
}

static PyObject * VisionLink_getNoObstacleInHeading(PyObject * /*self*/,
        PyObject *args) 
{
    Profile::start(PR_BE_OBS_HEAD, 122);   
    int minHeading; 
    int maxHeading;
    int minDist;
    int maxDist;
    int minObstacle = MIN_OBSTACLE;
    int flags = OBS_USE_DEFAULT;    
    
    if (obstacle == NULL)
    {
        cout << "Obstacle object is NULL" << endl;
        Profile::stop(PR_BE_OBS_HEAD);   
        Py_XINCREF(Py_None);
        return Py_None; 
    }
    if (!PyArg_ParseTuple(args, "iiii|ii", &minHeading, &maxHeading, &minDist, &maxDist,
    &minObstacle, &flags) ) 
    {
        cout << "VisionLink_getNoObstacleInHeading from C++, wrong input argument!" << endl;
        Profile::stop(PR_BE_OBS_HEAD);  
        Py_XINCREF(Py_None);
        return Py_None; 
    }
        
    int dist;    
    int result = obstacle->getNoObstacleInHeading(&dist, minHeading, maxHeading, 
    minDist, maxDist, minObstacle, flags);
    
    PyObject *t;

    t = PyTuple_New(2);
    PyTuple_SetItem(t, 0, PyInt_FromLong(result));
    PyTuple_SetItem(t, 1, PyInt_FromLong(dist));        
    
    Profile::stop(PR_BE_OBS_HEAD);   
    return t;
}

static PyObject * VisionLink_getBestGap(PyObject * /*self*/, PyObject *args)
{
    Profile::start(PR_BE_OBS_GAP, 123);   
    double destX;
    double destY;
    int gapLeft, gapRight, bestHeading, gapAngle;
    int maxdist = GAP_MAX_DIST;
    int mindist = MIN_DIST;
    int minGapAngle = MIN_GAP_ANGLE;
    int minObstacle = MIN_OBSTACLE;
    int flags = OBS_USE_DEFAULT;
    
    if (obstacle == NULL)
    {
        cout << "Obstacle object is NULL" << endl;
        Profile::stop(PR_BE_OBS_GAP);  
        Py_XINCREF(Py_None);
        return Py_None; 
    }    
    if (!PyArg_ParseTuple(args, "dd|iiiii", &destX, &destY, &maxdist, 
    &mindist, &minGapAngle, &minObstacle, &flags) ) 
    {
        cout << __func__ << " from C++, wrong input argument!" << endl;
        Profile::stop(PR_BE_OBS_GAP);   
        Py_XINCREF(Py_None);
        return Py_None; 
    }    
    
    bool result = obstacle->getBestGap(destX, destY, 
    &gapLeft, &gapRight, &bestHeading, &gapAngle,
    maxdist, mindist, minGapAngle, minObstacle, flags);
    
    if (!result) 
    {
        Profile::stop(PR_BE_OBS_GAP);   
        Py_XINCREF(Py_None);
        return Py_None; 
    }
    
    PyObject *t;

    t = PyTuple_New(4);
    PyTuple_SetItem(t, 0, PyInt_FromLong(gapLeft));
    PyTuple_SetItem(t, 1, PyInt_FromLong(gapRight));
    PyTuple_SetItem(t, 2, PyInt_FromLong(bestHeading));
    PyTuple_SetItem(t, 3, PyInt_FromLong(gapAngle));        
    
    Profile::stop(PR_BE_OBS_GAP);   
    return t;
}

static PyObject * VisionLink_getNoObstacleBetween(PyObject * /*self*/, PyObject *args)
{
    Profile::start(PR_BE_OBS_BETWEEN, 124);   
    
    int p1x, p1y, p2x, p2y;
    int corridorWidth = CORRIDOR_WIDTH;
    int minDist = MIN_DIST;
    int minObstacle = MIN_OBSTACLE;
    int flags = OBS_USE_DEFAULT;
    
    if (obstacle == NULL)
    {
        cout << "Obstacle object is NULL" << endl;
        Profile::stop(PR_BE_OBS_BETWEEN);  
        Py_XINCREF(Py_None);
        return Py_None; 
    }    
    if (!PyArg_ParseTuple(args, "iiii|iiii", &p1x, &p1y, &p2x, &p2y, 
    &corridorWidth, &minDist, &minObstacle, &flags)) 
    {
        cout << "VisionLink_getNoObstacleBetween from C++, wrong input argument!" << endl;
        Profile::stop(PR_BE_OBS_BETWEEN);
        Py_XINCREF(Py_None);
        return Py_None; 
    }    
    
    PyObject *ob = PyInt_FromLong(obstacle->getNoObstacleBetween(p1x, p1y, p2x, p2y, 
    corridorWidth, minDist, minObstacle, flags));
    
    Profile::stop(PR_BE_OBS_BETWEEN);   
    return ob;
}

static PyObject * VisionLink_getPointToNearestObstacleInBox(PyObject * /*self*/,
        PyObject *args) 
{
    Profile::start(PR_BE_OBS_NEAREST, 125);   
    int topleftx; 
    int toplefty;
    int bottomrightx;
    int bottomrighty;
    int posX;
    int posY;
    int minObstacle = MIN_OBSTACLE;
    int flags = OBS_USE_DEFAULT;
    
    if (obstacle == NULL)
    {
        cout << "Obstacle object is NULL" << endl;
        Profile::stop(PR_BE_OBS_NEAREST);   
        Py_XINCREF(Py_None);
        return Py_None; 
    }
    if (!PyArg_ParseTuple(args, "iiii|ii", &topleftx, &toplefty, &bottomrightx, &bottomrighty,
    &minObstacle, &flags) ) 
    {
        cout << "VisionLink_getPointToNearestObstacle from C++, wrong input argument!" << endl;
        Profile::stop(PR_BE_OBS_NEAREST); 
        Py_XINCREF(Py_None);
        return Py_None; 
    }
    
    bool result = obstacle->getPointToNearestObstacleInBox(topleftx, toplefty, bottomrightx, bottomrighty, &posX, &posY, minObstacle, flags);
    if (! result) 
    { //no obstacle under given criteria, return None
        //cout << "no obstacle under given criteria, return None" <<endl;
        Profile::stop(PR_BE_OBS_NEAREST);  
        Py_XINCREF(Py_None);
        return Py_None; 
    }
    
    PyObject *t;

    t = PyTuple_New(2);
    PyTuple_SetItem(t, 0, PyInt_FromLong(posX));
    PyTuple_SetItem(t, 1, PyInt_FromLong(posY));
    
    Profile::stop(PR_BE_OBS_NEAREST);   
    return t;
}

static PyObject * VisionLink_getFieldFeatureCount(PyObject * /*self*/,
        PyObject *args) {
    Profile::start(PR_BE_VISLINK, 43);           
        
#ifdef OFFLINE
    PyObject *r = PyInt_FromLong(20);
    Profile::stop(PR_BE_VISLINK); 
    return r;
#endif

    PyObject *f = PyInt_FromLong(vision->getFieldFeatureCount()); 
       
    Profile::stop(PR_BE_VISLINK); 
    return f;             
}
        
        
/*  Get information about a teammates. */
static PyObject * VisionLink_getGPSTeammateInfo(PyObject * /*self*/,
        PyObject *args) {
    Profile::start(PR_BE_VISLINK, 44); 
            
    int context, teammateNumber;
    if (!PyArg_ParseTuple(args, "ii", &context, &teammateNumber) || (context != GLOBAL && context != LOCAL) || teammateNumber< 1 || teammateNumber > 4 ) {
	cout << "VisionLink_getGPSTeammateInfo from C++, wrong input argument!" << endl;
        Profile::stop(PR_BE_VISLINK); 
        Py_XINCREF(Py_None);
        return Py_None;
    }
    PyObject *gti = NULL;
    switch (context){
        case GLOBAL:
            gti = PyObject_FromWMObject(gps->teammate(teammateNumber, 'g'));
            Profile::stop(PR_BE_VISLINK); 
            return gti;    
        case LOCAL:
            gti = PyObject_FromWMObject(gps->teammate(teammateNumber, 'l'));
            Profile::stop(PR_BE_VISLINK); 
            return gti;    
        default:
            Profile::stop(PR_BE_VISLINK); 
            return gti;                
    }

}

/*  Get information about the opponent's goal. */
static PyObject * VisionLink_getOppGoalInfo(PyObject * /*self*/, PyObject *args) {
    Profile::start(PR_BE_VISLINK, 45); 
    
    PyObject *i = PyObject_FromWMObject(gps->tGoal());
    
    Profile::stop(PR_BE_VISLINK); 
    return i;
}

/*  Get information about our own goal. */
static PyObject * VisionLink_getOwnGoalInfo(PyObject * /*self*/, PyObject *args) {
    Profile::start(PR_BE_VISLINK, 46); 
    
    PyObject *i = PyObject_FromWMObject(gps->oGoal());

    Profile::stop(PR_BE_VISLINK); 
    return i;
}

/*  Get coordinate array (see GPS::coord), depending on which team it is in . */
static PyObject * VisionLink_getGPSCoordArray(PyObject * /*self*/, PyObject * /*args */) {
    Profile::start(PR_BE_VISLINK, 47); 
    
    PyObject *i = PyObject_FromCoordArray(gps->coord);

    Profile::stop(PR_BE_VISLINK); 
    return i;
}


static PyObject * PyObject_FromVisualObject(VisualObject &obj){
    Profile::start(PR_BE_VISLINK, 48); 

    PyObject *t;
    const WMObj& r = gps->self();
    double d = obj.dist2;
    double globalx,globaly;

    calPosition (r.pos.x, r.pos.y, r.h, d, obj.head , &globalx, &globaly );

    t = PyTuple_New(15);
    PyTuple_SetItem(t, 0, PyInt_FromLong(obj.cf));
    PyTuple_SetItem(t, 1, PyFloat_FromDouble(obj.head));
    PyTuple_SetItem(t, 2, PyFloat_FromDouble(d)); //this distance is weird when fireball
    PyTuple_SetItem(t, 3, PyFloat_FromDouble(obj.var));
    PyTuple_SetItem(t, 4, PyFloat_FromDouble(globalx));
    PyTuple_SetItem(t, 5, PyFloat_FromDouble(globaly));

    PyTuple_SetItem(t, 6, PyFloat_FromDouble(obj.cx) );
    PyTuple_SetItem(t, 7, PyFloat_FromDouble(obj.cy) );
    PyTuple_SetItem(t, 8, PyFloat_FromDouble(obj.height));
    PyTuple_SetItem(t, 9, PyFloat_FromDouble(obj.width));
    PyTuple_SetItem(t, 10, PyFloat_FromDouble(obj.imgHead));
    PyTuple_SetItem(t, 11, PyFloat_FromDouble(obj.imgElev));
    PyTuple_SetItem(t, 12, PyInt_FromLong(obj.imgDir));
    PyTuple_SetItem(t, 13, PyFloat_FromDouble(obj.dist2)); //this distance is better,esp fireball
    PyTuple_SetItem(t, 14, PyFloat_FromDouble(obj.elev)); 

    Profile::stop(PR_BE_VISLINK); 
    return t;
}

static PyObject * VisionLink_getVisualObject(PyObject * /*self*/,
        PyObject *args) {
    Profile::start(PR_BE_VISLINK, 49);     
        
    int vobIndex;
    if (PyArg_ParseTuple(args,
                "i:getVisualObject",
                &vobIndex) && vobIndex >= 0 && vobIndex < VOB_COUNT  ) {
        PyObject *vo = PyObject_FromVisualObject(vision->vob[vobIndex]);      
        Profile::stop(PR_BE_VISLINK);         
        return vo;
    }
    else {
        Profile::stop(PR_BE_VISLINK); 
        Py_XINCREF(Py_None);
        return Py_None;
    }
}


/*  Get information about the ball sent by our teammates. */
static PyObject * VisionLink_getSharedBallInfo(PyObject * /*self*/,
        PyObject *args) {
    Profile::start(PR_BE_VISLINK, 50); 
        
    PyObject *t = PyTuple_New(4);
    PyTuple_SetItem(t, 0, PyFloat_FromDouble( gps->shareBallx));
    PyTuple_SetItem(t, 1, PyFloat_FromDouble( gps->shareBally));
    PyTuple_SetItem(t, 2, PyFloat_FromDouble( gps->shareBallvar));
    PyTuple_SetItem(t, 3, PyInt_FromLong( gps->sbRobotNum));

    Profile::stop(PR_BE_VISLINK); 
    return t;
}

static PyObject * VisionLink_getJointSensor(PyObject * /*self*/,
        PyObject *args) {
    Profile::start(PR_BE_VISLINK, 51); 
        
    if (debugCouts) {
        cout << "starting getJointSensor" << endl;
    }

    int jointEnum;
    if (PyArg_ParseTuple(args,
                "i:getJointSensor",
                &jointEnum)) {
        PyObject *js = PyFloat_FromDouble(gInfoManager.getJointSensor((JointEnum)jointEnum));
        Profile::stop(PR_BE_VISLINK);         
        return js;
    }
    else {
        Profile::stop(PR_BE_VISLINK); 
        Py_XINCREF(Py_None);
        return Py_None;
    }
}

static PyObject * VisionLink_getAnySensor(PyObject * /*self*/,
        PyObject *args) {
    Profile::start(PR_BE_VISLINK, 52);     
    if (debugCouts) {
        cout << "starting getJointSensor" << endl;
    }
    int sensorIndex;
    if (PyArg_ParseTuple(args,
                "i:getAnySensor",
                &sensorIndex)) {
        PyObject *as = PyLong_FromLong(gInfoManager.getAnySensor(sensorIndex));
        Profile::stop(PR_BE_VISLINK);         
        return as;
    }
    else {
        Profile::stop(PR_BE_VISLINK); 
        Py_XINCREF(Py_None);
        return Py_None;
    }
}

static PyObject* VisionLink_getLastMaxStepPWM(PyObject * /*self*/, PyObject *args) {
  Profile::start(PR_BE_VISLINK);
  if (debugCouts) {
    cout << "starting getLastMaxStepPWM" << endl;
  }

  int sensorIndex;
  if (PyArg_ParseTuple(args,
		       "i:getStepMaxSensorPWM",
		       &sensorIndex)) {
    PyObject *t = PyTuple_New(2);
    PyTuple_SetItem(t, 0, PyLong_FromLong(sensors->lastStepID));
    PyTuple_SetItem(t, 1, PyLong_FromLong(sensors->lastMaxStepPWM[sensorIndex]));
    Profile::stop(PR_BE_VISLINK);         
    return t;
  }
  else {
    Profile::stop(PR_BE_VISLINK); 
    Py_XINCREF(Py_None);
    return Py_None;
  }
}




static PyObject * VisionLink_getJointPWMDuty(PyObject * /*self*/,
        PyObject *args) {
    Profile::start(PR_BE_VISLINK, 53);     
    if (debugCouts) {
        cout << "starting getJointPWMDuty" << endl;
    }

    int jointEnum;
    if (PyArg_ParseTuple(args,
                "i:getJointPWMDuty",
                &jointEnum)) {
        PyObject *jd = PyLong_FromLong(gInfoManager.getJointPWMDuty((JointEnum)jointEnum));
        Profile::stop(PR_BE_VISLINK);         
        return jd;
    }
    else {
        Profile::stop(PR_BE_VISLINK); 
        Py_XINCREF(Py_None);
        return Py_None;
    }
}


static PyObject * VisionLink_getPWalkInfo(PyObject * /*self*/,
        PyObject *args) {
    Profile::start(PR_BE_VISLINK, 54);     

    PyObject *ret;
    ret = PyTuple_New(11);
    PyTuple_SetItem(ret, 0, PyInt_FromLong(*PWalkInfo->currentWalkType));
    PyTuple_SetItem(ret, 1, PyInt_FromLong(*PWalkInfo->currentMinorWalkType));
    PyTuple_SetItem(ret, 2, PyInt_FromLong(*PWalkInfo->currentStep));
    PyTuple_SetItem(ret, 3, PyInt_FromLong(*PWalkInfo->currentPG));    
    PyTuple_SetItem(ret, 4, PyInt_FromLong(*PWalkInfo->leading_leg));
    PyTuple_SetItem(ret, 5, PyFloat_FromDouble(*PWalkInfo->frontShoulderHeight));
    PyTuple_SetItem(ret, 6, PyFloat_FromDouble(*PWalkInfo->bodyTilt));
    PyTuple_SetItem(ret, 7, PyFloat_FromDouble(*PWalkInfo->desiredPan));
    PyTuple_SetItem(ret, 8, PyFloat_FromDouble(*PWalkInfo->desiredTilt));
    PyTuple_SetItem(ret, 9, PyFloat_FromDouble(*PWalkInfo->desiredCrane));
    PyTuple_SetItem(ret, 10, PyInt_FromLong(*PWalkInfo->isHighGain));
    //PyTuple_SetItem(rec, 11, PyInt_FromLong(*PWalkInfo->stepID)); <- this is not useful yet, so leave out
    Profile::stop(PR_BE_VISLINK); 
    return ret;
}

static PyObject * VisionLink_getSelfLocation(PyObject * /*self*/,
        PyObject * /*args*/ ) {
    Profile::start(PR_BE_VISLINK, 55); 
        
    if (debugCouts) {
        cout << "starting getSelfLocation" << endl;
    }

    PyObject *t;

    t = PyTuple_New(4);
    PyTuple_SetItem(t, 0, PyFloat_FromDouble(0.0));
    PyTuple_SetItem(t, 1, PyFloat_FromDouble(gInfoManager.getSelfLocation().pos.x));
    PyTuple_SetItem(t, 2, PyFloat_FromDouble(gInfoManager.getSelfLocation().pos.y));
    PyTuple_SetItem(t, 3, PyFloat_FromDouble(gInfoManager.getSelfLocation().h));

    Profile::stop(PR_BE_VISLINK); 
    return t;
}

/*  Get our location info. */
static PyObject * VisionLink_getGPSSelfInfo(PyObject * /*self*/, PyObject *args) {
    Profile::start(PR_BE_VISLINK, 56);
    
    PyObject *gsi = PyObject_FromWMObject(gps->self());
    
    Profile::stop(PR_BE_VISLINK);
    return gsi;
}

/*  Get self covariane matrix (3x3). */
static PyObject * VisionLink_getGPSSelfCovariance(PyObject * /*self*/, PyObject *args) {
    Profile::start(PR_BE_VISLINK, 57);
    
    PyObject *sc = PyObject_From3x3Maxtrix(gps->selfCov());
    
    Profile::stop(PR_BE_VISLINK);
    return sc;
}

static PyObject * VisionLink_getBallDistance(PyObject * /*self*/,
        PyObject * /*args*/ ) {
    Profile::start(PR_BE_VISLINK, 58); 
       
    PyObject *bd = PyFloat_FromDouble(gInfoManager.getBallDistance());
    
    Profile::stop(PR_BE_VISLINK);
    return bd;
}

static PyObject * VisionLink_getBallHeading(PyObject * /*self*/,
        PyObject * /*args*/ ) {
    Profile::start(PR_BE_VISLINK, 59);    
    
    PyObject *bh = PyFloat_FromDouble(gInfoManager.getBallHeading());
    
    Profile::stop(PR_BE_VISLINK); 
    return bh;
}

static PyObject * VisionLink_getBallConfidence(PyObject * /*self*/,
        PyObject * /*args*/ ) {
    Profile::start(PR_BE_VISLINK, 60); 
        
    PyObject *bc = PyFloat_FromDouble(gInfoManager.getBallConfidence());
    
    Profile::stop(PR_BE_VISLINK); 
    return bc;
}

static PyObject * VisionLink_reloadNNMC(PyObject * /*self*/,
        PyObject * /*args*/ ) {
    Profile::start(PR_BE_VISLINK, 61);   
      
    vision->reloadNNMC();
    Py_INCREF(Py_None);
    
    Profile::stop(PR_BE_VISLINK); 
    return Py_None;
}

static PyObject * VisionLink_getBallLocation(PyObject * /*self*/,
        PyObject * /*args*/ ) {
    Profile::start(PR_BE_VISLINK, 62); 
        
    if (debugCouts) {
        cout << "starting getBallLocationGlobal" << endl;
    }

    PyObject *t;

    t = PyTuple_New(4);
    PyTuple_SetItem(t, 0, PyFloat_FromDouble(gInfoManager.getBallConfidence()));
    PyTuple_SetItem(t, 1, PyFloat_FromDouble(gInfoManager.getBallLocation().x));
    PyTuple_SetItem(t, 2, PyFloat_FromDouble(gInfoManager.getBallLocation().y));
    PyTuple_SetItem(t, 3, PyFloat_FromDouble(gInfoManager.getBallLocation().d));

    Profile::stop(PR_BE_VISLINK); 
    return t;
}

static PyObject * VisionLink_getCameraFrame(PyObject * /*self*/, PyObject * /*args*/ ) { 
    Profile::start(PR_BE_VISLINK, 63); 
    //PyObject *t; 
    
    PyObject *cf = PyInt_FromLong(vision->cameraFrame);
    
    Profile::stop(PR_BE_VISLINK); 
    return cf;
}

static PyObject * VisionLink_getCurrentPreviousFrameID(PyObject * /*self*/, PyObject * /*args*/ ) {
    Profile::start(PR_BE_VISLINK, 64); 
    
    PyObject *t;
    t = PyTuple_New(2);
    PyTuple_SetItem(t, 0, PyInt_FromLong(Behaviours::currentFrameID));
    PyTuple_SetItem(t, 1, PyInt_FromLong(Behaviours::lastFrameID));
    
    Profile::stop(PR_BE_VISLINK); 
    return t;
}

static PyObject * VisionLink_getCurrentTime(PyObject * /*self*/, PyObject * /*args*/ ) {
    Profile::start(PR_BE_VISLINK, 65); 
    
    PyObject *t;
    t = PyTuple_New(2);
#ifndef OFFLINE
    SystemTime curTime;
    GetSystemTime(&curTime);
    PyTuple_SetItem(t, 0, PyInt_FromLong(curTime.seconds));
    PyTuple_SetItem(t, 1, PyInt_FromLong(curTime.useconds));
#else
	struct timeval tval;
	gettimeofday(&tval, NULL);
    PyTuple_SetItem(t, 0, PyInt_FromLong(tval.tv_sec));
    PyTuple_SetItem(t, 1, PyInt_FromLong(tval.tv_usec));
#endif

    Profile::stop(PR_BE_VISLINK); 
    return t;
}

static PyObject * VisionLink_setTimeCritical(PyObject * /*self*/, PyObject *args)
{
    Profile::start(PR_BE_VISLINK, 66); 
    
    //cerr << "PyEmbed setTimeCritical" << endl;
    Behaviours::setTimeCritical();
    
    Profile::stop(PR_BE_VISLINK); 
    Py_XINCREF(Py_None);
    return Py_None;
}

static PyObject * VisionLink_clearTimeCritical(PyObject * /*self*/, PyObject *args)
{
    Profile::start(PR_BE_VISLINK, 67); 
    
    //cerr << "PyEmbed clearTimeCritical" << endl;
    Behaviours::clearTimeCritical();
    
    Profile::stop(PR_BE_VISLINK); 
    Py_XINCREF(Py_None);
    return Py_None;
}


static PyObject * VisionLink_isTimeCritical(PyObject * /*self*/, PyObject *args)
{
    Profile::start(PR_BE_VISLINK, 68); 

    PyObject *tc = PyInt_FromLong(Behaviours::isTimeCritical());
    
    Profile::stop(PR_BE_VISLINK); 
    return tc;
}


static PyObject * VisionLink_setUseBeacons(PyObject * /*self*/, PyObject *args) {
    Profile::start(PR_BE_VISLINK, 69); 
    
    bool update;
    PyArg_ParseTuple(args, "i:setIfUseBeacons", &update);
    vision->lookForBeaconsAndGoals = update;
    cout << "Setting use beacons to: " << vision->lookForBeaconsAndGoals << endl;	
    
    Profile::stop(PR_BE_VISLINK);
    Py_XINCREF(Py_None);
    return Py_None;
}

static PyObject * VisionLink_setGrabbed(PyObject * /*self*/, PyObject *args) {
    Profile::start(PR_BE_VISLINK, 70); 
    
    vision->isGrabbed = true;
    
    Profile::stop(PR_BE_VISLINK); 
    Py_XINCREF(Py_None);
    return Py_None;
}

static PyObject * VisionLink_setGrabbing(PyObject * /*self*/, PyObject *args) {
    Profile::start(PR_BE_VISLINK, 71); 

    vision->isGrabbing = true;
    
    Profile::stop(PR_BE_VISLINK);
    Py_XINCREF(Py_None);
    return Py_None;
}
    
static const int FIRE_PAW_KICK_AUTO_BASIC = 0;
static const int GET_BEHIND_BALL          = 1;
static const int RUN_BEHIND_BALL          = 2;
static const int LOCATE_BALL              = 3;

static PyObject * VisionLink_getWirelessTeammateInfo(PyObject * /*self*/, PyObject *args) {
    Profile::start(PR_BE_VISLINK, 72); 

	PyObject *p;
	p = PyTuple_New(40);
             
	// Valid/invalid teammates.
	PyTuple_SetItem(p, 0, PyInt_FromLong(gps->tmObj(0).counter));               
	PyTuple_SetItem(p, 1, PyInt_FromLong(gps->tmObj(1).counter));
    PyTuple_SetItem(p, 2, PyInt_FromLong(gps->tmObj(2).counter));
    PyTuple_SetItem(p, 3, PyInt_FromLong(gps->tmObj(3).counter));

    // Teammate ball distance.
    PyTuple_SetItem(p, 4, PyInt_FromLong((((interpBehav) gps->tmObj(0).behavioursVal)).timeToReachBall));
    PyTuple_SetItem(p, 5, PyInt_FromLong((((interpBehav) gps->tmObj(1).behavioursVal)).timeToReachBall));
    PyTuple_SetItem(p, 6, PyInt_FromLong((((interpBehav) gps->tmObj(2).behavioursVal)).timeToReachBall));
    PyTuple_SetItem(p, 7, PyInt_FromLong((((interpBehav) gps->tmObj(3).behavioursVal)).timeToReachBall));
    
    // Teammate ball x.
    PyTuple_SetItem(p, 8, PyFloat_FromDouble(gps->tmBallObj(0).pos.getX()));
    PyTuple_SetItem(p, 9, PyFloat_FromDouble(gps->tmBallObj(1).pos.getX()));
    PyTuple_SetItem(p, 10, PyFloat_FromDouble(gps->tmBallObj(2).pos.getX()));
    PyTuple_SetItem(p, 11, PyFloat_FromDouble(gps->tmBallObj(3).pos.getX()));
    
    // Teammate ball y.
    PyTuple_SetItem(p, 12, PyFloat_FromDouble(gps->tmBallObj(0).pos.getY()));
    PyTuple_SetItem(p, 13, PyFloat_FromDouble(gps->tmBallObj(1).pos.getY()));
    PyTuple_SetItem(p, 14, PyFloat_FromDouble(gps->tmBallObj(2).pos.getY()));
    PyTuple_SetItem(p, 15, PyFloat_FromDouble(gps->tmBallObj(3).pos.getY()));   
    
    // Teammate ball pos var.
    PyTuple_SetItem(p, 16, PyFloat_FromDouble(gps->tmBallObj(0).posVar));
    PyTuple_SetItem(p, 17, PyFloat_FromDouble(gps->tmBallObj(1).posVar));
    PyTuple_SetItem(p, 18, PyFloat_FromDouble(gps->tmBallObj(2).posVar));
    PyTuple_SetItem(p, 19, PyFloat_FromDouble(gps->tmBallObj(3).posVar)); 

	// Teammate has grabbed a ball?
	PyTuple_SetItem(p, 20, PyInt_FromLong((((interpBehav) gps->tmObj(0).behavioursVal)).hasGrabbedBall));
	PyTuple_SetItem(p, 21, PyInt_FromLong((((interpBehav) gps->tmObj(1).behavioursVal)).hasGrabbedBall));
	PyTuple_SetItem(p, 22, PyInt_FromLong((((interpBehav) gps->tmObj(2).behavioursVal)).hasGrabbedBall));
	PyTuple_SetItem(p, 23, PyInt_FromLong((((interpBehav) gps->tmObj(3).behavioursVal)).hasGrabbedBall));
       
	// Teammate has seen a ball?
	PyTuple_SetItem(p, 24, PyInt_FromLong((((interpBehav) gps->tmObj(0).behavioursVal)).hasSeenBall));
	PyTuple_SetItem(p, 25, PyInt_FromLong((((interpBehav) gps->tmObj(1).behavioursVal)).hasSeenBall));
	PyTuple_SetItem(p, 26, PyInt_FromLong((((interpBehav) gps->tmObj(2).behavioursVal)).hasSeenBall));
	PyTuple_SetItem(p, 27, PyInt_FromLong((((interpBehav) gps->tmObj(3).behavioursVal)).hasSeenBall));
                            
	// Teammate has lost ball?
	PyTuple_SetItem(p, 28, PyInt_FromLong((((interpBehav) gps->tmObj(0).behavioursVal)).hasLostBall));
	PyTuple_SetItem(p, 29, PyInt_FromLong((((interpBehav) gps->tmObj(1).behavioursVal)).hasLostBall));
	PyTuple_SetItem(p, 30, PyInt_FromLong((((interpBehav) gps->tmObj(2).behavioursVal)).hasLostBall));
	PyTuple_SetItem(p, 31, PyInt_FromLong((((interpBehav) gps->tmObj(3).behavioursVal)).hasLostBall));

    // role? 
    PyTuple_SetItem(p, 32, PyInt_FromLong((((interpBehav) gps->tmObj(0).behavioursVal)).role));
    PyTuple_SetItem(p, 33, PyInt_FromLong((((interpBehav) gps->tmObj(1).behavioursVal)).role));
    PyTuple_SetItem(p, 34, PyInt_FromLong((((interpBehav) gps->tmObj(2).behavioursVal)).role));
    PyTuple_SetItem(p, 35, PyInt_FromLong((((interpBehav) gps->tmObj(3).behavioursVal)).role));
    
    // role counter? 
    PyTuple_SetItem(p, 36, PyInt_FromLong((((interpBehav) gps->tmObj(0).behavioursVal)).roleCounter));
    PyTuple_SetItem(p, 37, PyInt_FromLong((((interpBehav) gps->tmObj(1).behavioursVal)).roleCounter));
    PyTuple_SetItem(p, 38, PyInt_FromLong((((interpBehav) gps->tmObj(2).behavioursVal)).roleCounter));
    PyTuple_SetItem(p, 39, PyInt_FromLong((((interpBehav) gps->tmObj(3).behavioursVal)).roleCounter));
            
    Profile::stop(PR_BE_VISLINK);                     
	return p;
}

static PyObject * VisionLink_doBasicBehaviour(PyObject * /*self*/, PyObject *args) {
    Profile::start(PR_BE_VISLINK, 73); 

    int action;
    if (PyArg_ParseTuple(args, "i:doBasicBehaviour", &action)) {
        switch (action){
            case FIRE_PAW_KICK_AUTO_BASIC:
                Behaviours::firePawKick(Behaviours::FIRE_PAWKICK_AUTO);
                break;
            case GET_BEHIND_BALL:
                PythonBehaviours::getBehindBall();
                break;
            case RUN_BEHIND_BALL:
                PythonBehaviours::runBehindBall();
                break;
            case LOCATE_BALL:
                PythonBehaviours::locateBall();
                break;
            default:
                cout << "Unknown basic behaviour" << endl;
        }
    }
    
    Profile::stop(PR_BE_VISLINK); 
    Py_XINCREF(Py_None);
    return Py_None;
}

/* Get the project for ball. This returns the point on the field that is the
 * projection of the top of the ball onto the field plane. It assumes
 * that the robot can see the ball.
 */
static PyObject * VisionLink_getProjectedBall(PyObject * /*self*/, PyObject *args) {    
    Profile::start(PR_BE_VISLINK, 74); 

    // Track visual ball.
    double inpoints[2] = {TOPLEFT2CENTER_X(vision->vob[vobBall].cx),
                        TOPLEFT2CENTER_Y(vision->vob[vobBall].y)};
    double outpoints[2] = {-1, -1};
    vision->projectPoints(inpoints, 1, outpoints, 0);
    double ballx = -outpoints[0];
    double bally = outpoints[1];

    PyObject *t;
    t = PyTuple_New(2);
    PyTuple_SetItem(t, 0, PyFloat_FromDouble(ballx));
    PyTuple_SetItem(t, 1, PyFloat_FromDouble(bally));
    
    Profile::stop(PR_BE_VISLINK); 
    return t;
}

/* Project a point from the vision frame onto the ground. */
static PyObject * VisionLink_getProjectedPoint(PyObject * /*self*/, PyObject *args) {    
    Profile::start(PR_BE_VISLINK, 75); 

	double inpoints[2];

    if (PyArg_ParseTuple(args,
                "dd:getProjectedPoint",
                &(inpoints[0]), &(inpoints[1]))) {

		double outpoints[2] = {-1, -1};
		vision->projectPoints(inpoints, 1, outpoints, 0);
	
		PyObject *t;
		t = PyTuple_New(2);
		PyTuple_SetItem(t, 0, PyFloat_FromDouble(-outpoints[0]));
		PyTuple_SetItem(t, 1, PyFloat_FromDouble(outpoints[1]));
        Profile::stop(PR_BE_VISLINK); 
		return t;
    } else {
        Profile::stop(PR_BE_VISLINK); 
        return NULL;
    }

}

static PyObject * VisionLink_PointToElevation(PyObject * /*self*/, PyObject *args) {    
    Profile::start(PR_BE_VISLINK, 76); 

    double y;
    if (PyArg_ParseTuple(args,
                "d:PointToElevation",
                &y)) {
        //cout << "Got " << y << "  " << pointToElevation(y) << endl;
        PyObject *pte = PyFloat_FromDouble(pointToElevation(y));
        Profile::stop(PR_BE_VISLINK); 
		return pte;
    } else {
        Profile::stop(PR_BE_VISLINK); 
        return NULL;
    }

}

static PyObject * VisionLink_PointToHeading(PyObject * /*self*/, PyObject *args) {    
    Profile::start(PR_BE_VISLINK, 77); 

    double x;
    if (PyArg_ParseTuple(args,
                "d:PointToHeading",
                &x)) {

        //cout << "Got " << x << "  " << pointToHeading(x) << endl;
        PyObject *pth = PyFloat_FromDouble(pointToHeading(x));
        Profile::stop(PR_BE_VISLINK); 
		return pth;
    } else {
        Profile::stop(PR_BE_VISLINK); 
        return NULL;
    }

}

#if 0 // disabled for SubVIsion
static PyObject * VisionLink_getRedBlobInfo(PyObject * /*self*/, PyObject *args) {    
    Profile::start(PR_BE_VISLINK, 78); 

    PyObject *t;
    t = PyTuple_New(2);
    int x,y;
    gInfoManager.getAverageBlobInfo(x,y);
    PyTuple_SetItem(t, 0, PyInt_FromLong(x));
    PyTuple_SetItem(t, 1, PyInt_FromLong(y));
    
    Profile::stop(PR_BE_VISLINK); 
    return t;
}
#endif
static PyObject * VisionLink_getTestingInfo(PyObject * /*self*/, PyObject *args) {    
    Profile::start(PR_BE_VISLINK, 79); 

    /*
    //get total area of red
    int sumArea = 0;
    for (int i=0;i< vision->blobs->count(cROBOT_RED) ; i++){
        BlobInfo& redBlob = vision->blobs->getBlob(cROBOT_RED,i);
        sumArea += redBlob.area;
    }
    PyObject *r = PyInt_FromLong( sumArea );
    Profile::stop(PR_BE_VISLINK);
    return r;
    */
    
    int height = 0;
    //get dog's vision blob height
    for (int i=vobRedDog;i<=vobRedDog4;i++){
        if (vision->vob[i].height > 1){
            cout << "Object " << i << "[" << i - vobRedDog + 1 << "]" << vision->vob[i].height << endl;
            height = int(vision->vob[i].height);
        }
    }

    PyObject *h = PyInt_FromLong( height );
    Profile::stop(PR_BE_VISLINK); 
    return h;
}

static PyObject * VisionLink_setWalkLearningParameters(PyObject * /*self*/, PyObject *args) {
    Profile::start(PR_BE_VISLINK, 80); 

    const char *paraStr;
    if (PyArg_ParseTuple(args,"s:setWalkLearningParameters",&paraStr)){
      WalkLearningParams  wlParams;
      double forwardSpeed;
      istringstream ss(paraStr);
      FFCOUT << "Got " << paraStr << endl;
      string type;
      ss >> type;
      if (type == "ELI"){
        wlParams.walkType = EllipseWalkWT;
      } else if (type == "SKE"){
        wlParams.walkType = SkellipticalWalkWT;
      } else if (type == "SKI"){
        wlParams.walkType = SkiWalkWT;
      } else if (type == "NOR") { 
        wlParams.walkType = NormalWalkWT;
      } else {
	    wlParams.walkType = EllipseWalkWT;
      }
   
      switch (wlParams.walkType) {
        case EllipseWalkWT:
	       wlParams.ep.read(ss);
	       break;
        case SkellipticalWalkWT:
	       wlParams.sp.read(ss);
	       //wlParams.sp.print(cout);
	       break;
        case SkiWalkWT:
	       ss >> forwardSpeed;
	       wlParams.skp.read(ss);
	       //wlParams.skp.print(cout);
	       cout << "Ski forwardSpeed = " << forwardSpeed << endl;
	       break;
        case NormalWalkWT: 
            wlParams.np.read(ss); 
            //wlParams.np.print(cout);
            break;
        default:
	       cout << "Unknown walk type in WalkingLearner::setLearningParameters" << endl;
	       break;
      }

#ifndef OFFLINE
      transmission->sendLearningParams(wlParams);
#endif //OFFLINE
    }
    
    Profile::stop(PR_BE_VISLINK); 
    Py_XINCREF(Py_None);
    return Py_None;
}

static PyObject * VisionLink_getTeamColor (PyObject * /*self*/,
        PyObject * /*args*/ ) {
  Profile::start(PR_BE_VISLINK, 81);       
        
  if ( gps->targetGoal == vobYellowGoal ) {
    PyObject *r = PyInt_FromLong(0);
    Profile::stop(PR_BE_VISLINK); 
    return r;
  }
  else {
    PyObject *r = PyInt_FromLong(1);
    Profile::stop(PR_BE_VISLINK); 
    return r;
  }
}

static PyObject * VisionLink_getMyPlayerNum (PyObject * /*self*/,
        PyObject * /*args*/ ) {
    Profile::start(PR_BE_VISLINK, 82);  
    
    PyObject *pn = PyInt_FromLong(Behaviours::PLAYER_NUMBER);
    
    Profile::stop(PR_BE_VISLINK);  
    return pn;
}

static PyObject * VisionLink_getGPSTeammateBallInfo (PyObject * /*self*/,
        PyObject *args) {
    Profile::start(PR_BE_VISLINK, 83); 
    
    int tmIndex;
    if (PyArg_ParseTuple(args,
                "i:getGPSTeammateBallInfo",
                &tmIndex) && tmIndex >= 0 && tmIndex < NUM_TEAM_MEMBER ) {
        PyObject *tbi = PyObject_FromWMObject(gps->tmBallObj(tmIndex));
        Profile::stop(PR_BE_VISLINK); 
        return tbi;
    }
    else {
    
        Profile::stop(PR_BE_VISLINK); 
        Py_XINCREF(Py_None);
        return Py_None;
    }
}

static PyObject * VisionLink_printCPlusPlus(PyObject * /*self*/, PyObject *args) {
    Profile::start(PR_BE_VISLINK, 84); 
    //int selection;
#ifdef COMPILE_ALL_CPP
    if (PyArg_ParseTuple(args, "i:printCPlusPlus", &selection)) {
        cout << "Selection: " << selection << endl;
        switch (selection){
            // Print results from vision strikerpoint.
            case 0:
                if (vision->vob[0].cf == 0)
                    cout << "Vision_Link_printCPlusPlus ball confidence problem!" << endl;
                else {
                    UNSW2004::determineBallSource();
                    double ptx, pty;
                    UNSW2004::getUNSW2004StrikerPoint(&ptx, &pty);
                    cout << "C++ striker point x: "  << ptx << endl;
                    cout << "C++ strriker point y: " << pty << endl;
                    UNSW2004::doUNSW2004StrikerForward(ptx, pty);
                    cout << "C++ striker forward: " << Behaviours::forward << endl;     
                    cout << "C++ striker turnCCW: " << Behaviours::turnCCW << endl;
                    cout << "C++ striker left:    " << Behaviours::left    << endl;
                    Behaviours::forward = 0;
                    Behaviours::turnCCW = 0;
                    Behaviours::left    = 0;
                }               
                break;          
            // Print GPS ball values for comparision.
            case 1:
                cout << "gps->getBall(GLOBAL).x: " << gps->getBall(GLOBAL).x << endl;
                cout << "gps->getBall(GLOBAL).y: " << gps->getBall(GLOBAL).y << endl;
                cout << "gps->getBall(GLOBAL).head: " << gps->getBall(GLOBAL).head << endl;
                break;
			case 2:
				Behaviours::saGetBehindBall(40, 90, 1);
				cout << "C++ behind forward: " << Behaviours::forward << endl;
				cout << "C++ behind turnCCW: " << Behaviours::turnCCW << endl;
				cout << "C++ behind left:    " << Behaviours::left    << endl;
				//Behaviours::forward = 0;
				//Behaviours::turnCCW = 0;
				//Behaviours::left    = 0;
				break;
			case 3:
				UNSW2004::determineBallSource();
				UNSW2004::doUNSW2004TrackVisualBall();
				UNSW2004::doUNSW2004Defend();
				cout << "C++ forward: " << Behaviours::forward << endl;
				cout << "C++ turnCCW: " << Behaviours::turnCCW << endl;
				cout << "C++ left:    " << Behaviours::left    << endl;
                Behaviours::forward = 0;
                Behaviours::turnCCW = 0;
                Behaviours::left    = 0;
				break;					
            default:
                cout << "PrintCPlusCPlus() got an invalid selection: " << selection << "!" << endl; 
        }
    }
#endif //COMPILE_ALL_CPP

    Profile::stop(PR_BE_VISLINK); 
    Py_XINCREF(Py_None);
    return Py_None;
}

// This wraps the getHeadingToBestGap(&lmin,&lmax,&rmin,&rmax), which
// determines the best range for goal scoring
static PyObject * VisionLink_getHeadingToBestGap (PyObject * /*self*/,
        PyObject *args) {
    Profile::start(PR_BE_VISLINK, 85);         

    PyObject *ret;
    double lmin, lmax, rmin, rmax;

    vision->getHeadingToBestGap(gps->targetGoal, &lmin, &lmax, &rmin, &rmax);
    
    ret = PyTuple_New(4);
    PyTuple_SetItem(ret, 0, PyFloat_FromDouble(lmin));
    PyTuple_SetItem(ret, 1, PyFloat_FromDouble(lmax));
    PyTuple_SetItem(ret, 2, PyFloat_FromDouble(rmin));
    PyTuple_SetItem(ret, 3, PyFloat_FromDouble(rmax));

    Profile::stop(PR_BE_VISLINK); 
    return ret;
}

static PyObject * VisionLink_getIfGPSMotionUpdate(PyObject * /*self*/, PyObject *args) {
    Profile::start(PR_BE_VISLINK, 86); 
    
	PyObject *mu = PyInt_FromLong(gps->doGPSMotionUpdate);
    
    Profile::stop(PR_BE_VISLINK); 
    return mu;
}

static PyObject * VisionLink_getIfGPSVisionUpdate(PyObject * /*self*/, PyObject *args) {
    Profile::start(PR_BE_VISLINK, 87); 
    
	PyObject *vu = PyInt_FromLong(gps->doGPSVisionUpdate);
    
    Profile::stop(PR_BE_VISLINK); 
    return vu;
}

static PyObject * VisionLink_setIfGPSMotionUpdate(PyObject * /*self*/, PyObject *args) {
    Profile::start(PR_BE_VISLINK, 88); 
    
    bool update;
    PyArg_ParseTuple(args, "i:setIfGpsMotionUpdate", &update);
    gps->doGPSMotionUpdate = update;
    //cout << "Setting gps motion update to: " << gps->doGPSMotionUpdate << endl;	
    
    Profile::stop(PR_BE_VISLINK); 
    Py_XINCREF(Py_None);
    return Py_None;
}

static PyObject * VisionLink_setIfObstacleUpdate(PyObject * /*self*/, PyObject *args) {
    Profile::start(PR_BE_VISLINK_OBS, 126); 
    
    bool update;
    PyArg_ParseTuple(args, "i:setIfObstacleUpdate", &update);
    obstacle->doObstacleUpdate = update;
    cout << "Setting obstacle update to: " << update << endl;	
    
    Profile::stop(PR_BE_VISLINK_OBS); 
    Py_XINCREF(Py_None);
    return Py_None;
}

static PyObject * VisionLink_setIfGPSVisionUpdate(PyObject * /*self*/, PyObject *args) {
    Profile::start(PR_BE_VISLINK, 89); 
    
    bool update;
    PyArg_ParseTuple(args, "i:setIfGpsVisionUpdate", &update);
    gps->doGPSVisionUpdate = update;
    //cout << "Setting gps vision update to: " << gps->doGPSVisionUpdate << endl;	
    
    Profile::stop(PR_BE_VISLINK); 
    Py_XINCREF(Py_None);
    return Py_None;
}

static PyObject * VisionLink_setIfGPSOpponentUpdate(PyObject * /*self*/, PyObject *args) {
    Profile::start(PR_BE_VISLINK, 90); 
    
	bool update;
	PyArg_ParseTuple(args, "i:setIfGpsOpponentUpdate", &update);
    gps->doGPSOpponentUpdate = update;
    
    Profile::stop(PR_BE_VISLINK); 
    Py_XINCREF(Py_None);
    return Py_None;
}

static PyObject * VisionLink_setGlobalMaxEdge(PyObject * /*self*/, PyObject *args) {
    Profile::start(PR_BE_VISLINK, 91); 
    
    gps->GPSGlobalMaxEdgeSet(vision);
    
    Profile::stop(PR_BE_VISLINK); 
    Py_XINCREF(Py_None);    
    return Py_None;
}

static PyObject * VisionLink_resetEdgeMap(PyObject * /*self*/, PyObject *args) {
    Profile::start(PR_BE_VISLINK, 92); 
    
    vision->invalidateEdgeMap();
    
    Profile::stop(PR_BE_VISLINK); 
    Py_XINCREF(Py_None);    
    return Py_None;
}

static PyObject * VisionLink_resetBall(PyObject * /*self*/, PyObject *args) {
    Profile::start(PR_BE_VISLINK, 93); 

    double x, y, dx, dy;
    if (!PyArg_ParseTuple(args, "dddd:VisionLink_resetBall", &x, &y, &dx, &dy) ) 
    {
        cout << "VisionLink_resetBall from C++, wrong input argument!" << endl;
        Profile::stop(PR_BE_VISLINK);
        Py_XINCREF(Py_None);
        return Py_None; 
    }
    gps->resetBall(x,y,dx,dy);
    
    Profile::stop(PR_BE_VISLINK);
    Py_XINCREF(Py_None);    
    return Py_None;
}

static PyObject * VisionLink_setAllowPinkMapping(PyObject * /*self*/, PyObject *args) {
    Profile::start(PR_BE_VISLINK, 94); 
    
    bool update;
    PyArg_ParseTuple(args, "i:setAllowPinkMapping", &update);
    gps->allowPinkMapping = update;
    
    Profile::stop(PR_BE_VISLINK); 
    Py_XINCREF(Py_None);
    return Py_None;
}

static PyObject * VisionLink_getUsePinkUpdate(PyObject * /*self*/, PyObject *args) {
    Profile::start(PR_BE_VISLINK, 95); 
    
    PyObject *upu = PyInt_FromLong(gps->pinkUpdate);
    
    Profile::stop(PR_BE_VISLINK);
    return upu; 
}

static PyObject * VisionLink_resetPinkMapping(PyObject * /*self*/, PyObject *args) {
    Profile::start(PR_BE_VISLINK, 96); 
    
    gps->ResetPinkMapping();
    
    Profile::stop(PR_BE_VISLINK); 
    Py_XINCREF(Py_None);
    return Py_None;
}

static PyObject * VisionLink_resetPinkUpdate(PyObject * /*self*/, PyObject *args) {

#ifdef LOCALISATION_CHALLENGE
    gps->ResetPinkUpdate();
#endif    
    Py_XINCREF(Py_None);
    return Py_None;
}

static PyObject * VisionLink_initPinkObject(PyObject * /*self*/, PyObject *args) {
    Profile::start(PR_BE_VISLINK, 97); 
    
    gps->landmarksInitialised = false;
#ifdef LOCALISATION_CHALLENGE
    gps->CalcIntersections();
#endif    

    Profile::stop(PR_BE_VISLINK); 
    Py_XINCREF(Py_None);
    return Py_None;
}

/* Game Controller functions */
static PyObject * VisionLink_getTheCurrentMode(PyObject * /*self*/, PyObject *args) {
    Profile::start(PR_BE_VISLINK, 98); 
    
	PyObject *cm = PyInt_FromLong(Behaviours::gameData.state);
    
    Profile::stop(PR_BE_VISLINK); 
    return cm;
}

static PyObject * VisionLink_getPenalised(PyObject * /*self*/, PyObject *args) {
    Profile::start(PR_BE_VISLINK, 99); 
    
    PyObject *p = PyInt_FromLong(Behaviours::myTeam.players[Behaviours::PLAYER_NUMBER-1].penalty);
    
    Profile::stop(PR_BE_VISLINK); 
    return p;
}

static PyObject * VisionLink_getDogPenalised(PyObject * /*self*/, PyObject *args) {
    Profile::start(PR_BE_VISLINK, 100); 
    
    int dognum;
    PyArg_ParseTuple(args, "i", &dognum);
    PyObject *dp = PyInt_FromLong(Behaviours::myTeam.players[dognum].penalty);
    
    Profile::stop(PR_BE_VISLINK); 
    return dp;
}

static PyObject * VisionLink_getKickOffTeam(PyObject * /*self*/, PyObject *args) {
    Profile::start(PR_BE_VISLINK, 101); 
    
    // Blue = 0, Red = 1
	PyObject *kot = PyInt_FromLong(Behaviours::gameData.kickOffTeam);
    
    Profile::stop(PR_BE_VISLINK); 
    return kot;
}

/* from Constant.py, BLUE_TEAM = 0, RED_TEAM = 1 */
static PyObject * VisionLink_getDropInTeam(PyObject * /*self*/, PyObject *args) {
    Profile::start(PR_BE_VISLINK, 102);
    
    PyObject *dit = PyInt_FromLong(Behaviours::gameData.dropInTeam);       
    
    Profile::stop(PR_BE_VISLINK); 
    return dit;
}

static PyObject * VisionLink_getDropInTime(PyObject * /*self*/, PyObject *args) {
    Profile::start(PR_BE_VISLINK, 103);
    
    PyObject *dit = PyInt_FromLong(Behaviours::gameData.dropInTime);
    
    Profile::stop(PR_BE_VISLINK);
    return dit;
}

static PyObject * VisionLink_getScore(PyObject * /*self*/, PyObject *args) {
    Profile::start(PR_BE_VISLINK, 104); 
    
    PyObject* s = PyTuple_New(2);
    TeamInfo *us, *them;
    if (Behaviours::gameData.teams[TEAM_RED].teamNumber
            == Behaviours::TEAM_NUMBER) {
        us = &Behaviours::gameData.teams[TEAM_RED];
        them = &Behaviours::gameData.teams[TEAM_BLUE];
    } else {
        us = &Behaviours::gameData.teams[TEAM_BLUE];
        them = &Behaviours::gameData.teams[TEAM_RED];
    }
   
    PyTuple_SetItem(s, 0, PyInt_FromLong(us->score));
    PyTuple_SetItem(s, 1, PyInt_FromLong(them->score));

    Profile::stop(PR_BE_VISLINK); 
    return s;
}

static PyObject * VisionLink_gameDataIsValid(PyObject * /*self*/,
                                            PyObject *args) {
    Profile::start(PR_BE_VISLINK, 105);           
                                  
//    FCOUT << "vframe " << vision->vFrame << ", gameDataTime "
//        << Behaviours::gameDataTime << endl;
    PyObject *div = PyInt_FromLong((vision->vFrame - Behaviours::gameDataTime) <= 60); // true if data <= 2 seconds old
    
    Profile::stop(PR_BE_VISLINK); 
    return div;
}



static PyObject * VisionLink_getTemperature(PyObject * /*self*/, PyObject *args) {
    Profile::start(PR_BE_VISLINK, 106); 
    
#ifndef OFFLINE
	OStatus result;
	OPowerStatus currentStatus;
	result = OPENR::GetPowerStatus(&currentStatus);
	
	if (result == oSUCCESS) {
        PyObject *t = PyInt_FromLong(currentStatus.temperature);
        Profile::stop(PR_BE_VISLINK); 
		return t;
	} else {
		// Return an invalid value.
        PyObject *inv = PyInt_FromLong(-3);
        Profile::stop(PR_BE_VISLINK); 
		return inv;
	}
#endif // OFFLINE	
    PyObject *inv = PyInt_FromLong(-3);
    Profile::stop(PR_BE_VISLINK); 
    return inv;
}

static PyObject * VisionLink_getCPUUsage(PyObject * /*self*/, PyObject *args) {
    Profile::start(PR_BE_VISLINK, 107); 
    
	PyObject *cu = PyFloat_FromDouble(Behaviours::cpuUsage);
    
    Profile::stop(PR_BE_VISLINK); 
    return cu;
}


static PyObject * VisionLink_srand(PyObject * /*self*/, PyObject *args) {
    Profile::start(PR_BE_VISLINK, 108); 
    
    int i;
    if (PyArg_ParseTuple(args,"i:srand",&i)) {
        srand((unsigned int)i); 
    }
    Py_INCREF(Py_None);
    
    Profile::stop(PR_BE_VISLINK); 
    return Py_None;   
}

static PyObject * VisionLink_random(PyObject * /*self*/, PyObject *args) {
    Profile::start(PR_BE_VISLINK, 109); 
    
    PyObject *rnd = PyInt_FromLong(rand());
    
    Profile::stop(PR_BE_VISLINK); 
    return rnd;
}

static PyObject * VisionLink_randrange(PyObject * /*self*/, PyObject *args) {
    Profile::start(PR_BE_VISLINK, 110); 
    
    PyObject *rnd = PyInt_FromLong(rand());
    
    Profile::stop(PR_BE_VISLINK); 
    return rnd;
}

/* Python profiling functions */
static PyObject * VisionLink_startProfile(PyObject * /* self */, PyObject *args) {
    char *fname;    
    
    if (!PyArg_ParseTuple(args, "s:startProfile", &fname)) 
    {
        FCOUT << "from C++, wrong input argument!" << endl;

        Py_XINCREF(Py_None);
        return Py_None; 
    }

    Profile::startPy(fname);
    
    Py_XINCREF(Py_None);
    return Py_None;
}

static PyObject * VisionLink_stopProfile(PyObject * /* self */, PyObject *args) {
    char *fname;    
    
    if (!PyArg_ParseTuple(args, "s:stopProfile", &fname)) 
    {
        FCOUT << "from C++, wrong input argument!" << endl;

        Py_XINCREF(Py_None);
        return Py_None; 
    }

    Profile::stopPy(fname);
    
    Py_XINCREF(Py_None);
    return Py_None;
}


static PyObject * VisionLink_profileTimeSoFar(PyObject * /* self */, PyObject *args) {
    char *fname;    
    
    if (!PyArg_ParseTuple(args, "s:profileTimeSoFar", &fname)) 
    {
        FCOUT << "from C++, wrong input argument!" << endl;

        Py_XINCREF(Py_None);
        return Py_None; 
    }

    PyObject *tsf = PyFloat_FromDouble(Profile::timeSoFarPy(fname));
    
    return tsf;
}

/**** Offline functions ****/
#ifdef OFFLINE
// Called by VisPolicy through PyEmbed to process a frame. This emulates
// resultCamera in Vision.cc. Offline only.
static PyObject *VisionLink_processFrame(PyObject * /*self*/, PyObject *args) {
    Profile::start(PR_BE_VISLINK, 111); 
    
	// Setting goal
	if (gps->goalNotSet) {
		gps->SetGoals(true); // be Red
		//sendPlayerNumber(Behaviours::PLAYER_NUMBER, goalDir);
        //vision.invalidateEdgeMap();
	}

    //if ((*PWalkInfo.currentWalkType) != lastWalkType) {
        vision->invalidateEdgeMap();
    //    lastWalkType = (*PWalkInfo.currentWalkType);
    //}
    
    gps->sendCPlane = false;
    vision->bodyMoving = (Behaviours::left != 0 ||
                         Behaviours::forward != 0 ||
                         Behaviours::turnCCW != 0);
    vision->headMovement = sensors->getHeadSpeed();

    /* Process image */
    // nothing to do
    
    /* GPS updates */
    double myx, myy, myh, ballx, bally, balldx, balldy;
    if (PyArg_ParseTuple(args, "ddddddd:processFrame",
                        &myx, &myy, &myh, &ballx, &bally, &balldx, &balldy)) {
        gps->resetGaussians(myx, myy, myh);
        gps->resetBall(ballx, bally, balldx, balldy);
    } else {
        Profile::stop(PR_BE_VISLINK); 
        return NULL;
    }

    /* Obstacle updates */
    obstacle->ObstacleUpdate(vision, gps);
    obstacle->setObstacleGPSValid();

    /* Run behaviours */
    // nothing to do (they will run later)

    Py_INCREF(Py_None);
    
    Profile::stop(PR_BE_VISLINK); 
    return Py_None;
}

// clear all obstacle information
static PyObject *VisionLink_clearObstacles(PyObject* /*self*/, PyObject *args) {
    Profile::start(PR_BE_VISLINK_OBS, 112); 
    
    obstacle->clearObstacles();
    Py_INCREF(Py_None);
    
    Profile::stop(PR_BE_VISLINK_OBS); 
    return Py_None;
}

// add an obstacle box at the specified (x, y)
static PyObject *VisionLink_setObstacle(PyObject * /*self*/, PyObject *args) {
    Profile::start(PR_BE_VISLINK_OBS, 113); 
    
    int bx, by;
    if (PyArg_ParseTuple(args, "ii:setObstacle", &bx, &by)) {
        //cerr << "setObstacle " << bx << ", " << by << endl;
        obstacle->addObstacleBox(bx, by, vision->vFrame, MIN_OBSTACLE * 10);
    } else {
        Profile::stop(PR_BE_VISLINK_OBS); 
        return NULL;
    }
    Py_INCREF(Py_None);
    
    Profile::stop(PR_BE_VISLINK_OBS); 
    return Py_None;
}

#endif // OFFLINE


/* With this function specifying all the implemented semi-Python/semi-C
** functions, the python codes can call the "bridging" functions implemented in
** this file.
 *
 * NOTE: CHANGE IN THIS TABLE SHOULD BE TYPED IN VisionLink.py as well!
 **/
static PyMethodDef Vision_methods[] = {

    /*
     * General functions
     */
    {"setCallbacks", VisionLink_setCallbacks, METH_VARARGS, "Set the PyEmbed callbacks."},

    /*
     * Actuator related functions
     */ 
    {"sendAtomicAction", VisionLink_sendAtomicAction, METH_VARARGS, "Set the atomic action."},
    {"sendIndicators", VisionLink_sendIndicators, METH_VARARGS, "Set the LEDs, tail and ears."},
    {"sendMessageToBase", VisionLink_sendMessageToBase, METH_VARARGS, "Send wireless message back to base station, message is prefixed by '****'"},
    {"getBatteryLevel", VisionLink_getBatteryLevel, METH_NOARGS, "Get the robot's battery level."},
    {"getBatteryCurrent",VisionLink_getBatteryCurrent, METH_NOARGS, "Get the robots's battery current"},
    {"getFallenState", VisionLink_getFallenState, METH_NOARGS, "Have we fallen over?"},
    {"getPressSensorCount", VisionLink_getPressSensorCount, METH_O, "Get the state of the robot's touch sensors."},
    {"getSwitchSensorCount", VisionLink_getSwitchSensorCount, METH_O, "Get the state of the robot's switch sensors."},
    {"getJointSensor", VisionLink_getJointSensor, METH_VARARGS, "Get joint sensor from CommonSense."},
    {"getAnySensor", VisionLink_getAnySensor, METH_VARARGS, "Get joint sensor from CommonSense."},
    {"getLastMaxStepPWM", VisionLink_getLastMaxStepPWM, METH_VARARGS, "Get the max pwm value in the last step"},
    {"getJointPWMDuty", VisionLink_getJointPWMDuty, METH_VARARGS, "Get joint pwmDuty (or force sensor) from CommonSense."},
    {"getPWalkInfo", VisionLink_getPWalkInfo, METH_NOARGS, "getPWalkInfo() --> (walkType, step, PG, leadingLeg, stepComplete, desiredPan, desiredTilt, desiredCrane, isHighGain) ."},

    /*
     * Communication functions
     */
    {"sendEnvironmentTeamMates", VisionLink_sendData, METH_VARARGS, "Send debug information"},
    {"sendEnvironmentTeamMates", VisionLink_sendEnvironmentTeamMates, METH_NOARGS, "Send our world model to our teammates.[not available]"},
    {"sendCompressedCPlane", VisionLink_sendCompressedCPlane, METH_NOARGS, "Send the CPlane to robolink."},
    {"sendOPlane", VisionLink_sendOPlane, METH_NOARGS, "Send the OPlane to the base station.[not available]"},
    {"sendEnvironmentBaseStation", VisionLink_sendEnvironmentBaseStation, METH_NOARGS, "Send the world model to the base station.[not available]"},
    {"sendYUVPlane", VisionLink_sendYUVPlane, METH_NOARGS, "Send the YUV plane to the base station.[not available]"},
{"sendWalkInfo", VisionLink_sendWalkInfo, METH_VARARGS, "Send the PWalkInfo and atomic action to the base station"},
    {"sendObjects", VisionLink_sendObjects, METH_NOARGS, "Send Objects to robolink"},
    {"sendMyBehaviourInfo", VisionLink_sendMyBehaviourInfo, METH_VARARGS,"sendMyBehaviourInfo( ballDist, isSupporter, isStriker, isGoalie, haveBall, kickedOff, isDefending, isBackingOff) --> send my behaviours information to our teammates, such as ballDist, isGoalie, isForward... ."},
    {"sendTeamTalk", VisionLink_sendTeamTalk, METH_VARARGS, "Broadcast a behaviour coordination message."},

    /*
     * GPS functions
     */
    {"setGPSGoal", VisionLink_setGPSGoal, METH_O, "Tell GPS which goal is ours."},
    {"setGPSPaused", VisionLink_setGPSPaused, METH_O, "Pause GPS updates.  Useful when penalised."},
    {"setIfGPSMotionUpdate", VisionLink_setIfGPSMotionUpdate, METH_VARARGS, "Set gps updating with motion."},
    {"setIfGPSVisionUpdate", VisionLink_setIfGPSVisionUpdate, METH_VARARGS, "Set gps updating with vision."},
    {"setIfGPSOpponentUpdate", VisionLink_setIfGPSOpponentUpdate, METH_VARARGS, "Set gps opponent updating."},
    {"resetEdgeMap", VisionLink_resetEdgeMap, METH_NOARGS, "Resets the edge pixel stationary map."},
    {"setAllowPinkMapping", VisionLink_setAllowPinkMapping, METH_VARARGS, "Set recording of pink blobs."},  
    {"setGlobalMaxEdge", VisionLink_setGlobalMaxEdge, METH_NOARGS, "Sets gps location for maximum edge match."},
    {"getUsePinkUpdate", VisionLink_getUsePinkUpdate, METH_NOARGS, "Did gps use a pink blob to map/localise off"},
    {"getIfGPSMotionUpdate", VisionLink_getIfGPSMotionUpdate, METH_NOARGS, "Is gps updating with motion?"},
    {"getIfGPSVisionUpdate", VisionLink_getIfGPSVisionUpdate, METH_NOARGS, "Is gps updating with vision?"},    
    {"getGPSSelfInfo", VisionLink_getGPSSelfInfo, METH_NOARGS, "Get our location info."},
    {"getGPSSelfCovariance", VisionLink_getGPSSelfCovariance, METH_NOARGS, "Get information about our own goal.\n return 3x3 tuple"},
    {"getGPSBallInfo", VisionLink_getGPSBallInfo, METH_VARARGS, "Get information on the ball."},
    {"getGPSBallMaxVar", VisionLink_getGPSBallMaxVar, METH_NOARGS, "see gps::getBallMaxVar() ."},
    {"getGPSBallVInfo", VisionLink_getGPSBallVInfo, METH_VARARGS, "Get information on the ball's velocity."},
//    {"getGPSVBallMaxVar", VisionLink_getGPSVBallMaxVar, METH_NOARGS, "see gps::getVBallMaxVar() ."},
    {"getGPSOpponentInfo", VisionLink_getGPSOpponentInfo, METH_VARARGS, "Get information on an opponent.\n getGPSOpponentInfo(oppNum, context) ---> Vector where the opponent number oppNum (0..3) is, in context (GLOBAL,LOCAL)"},
    {"getGPSOpponentCovMax", VisionLink_getGPSOpponentCoxMax, METH_VARARGS, "Get covariance max of an opponent.\n getGPSOpponentCovMax(oppNum) --> double, oppNum = 0..3"},
    {"getGPSTeammateInfo", VisionLink_getGPSTeammateInfo, METH_VARARGS, "Get information about a teammates.\n getGPSTeammateInfo(teammate, context) --> Return WMObj tuple about that teammate [1..4] and context [LOCAL|GLOBAL] "},
    {"getOppGoalInfo", VisionLink_getOppGoalInfo, METH_NOARGS, "Get information about the opponent's goal."},
    {"getOwnGoalInfo", VisionLink_getOwnGoalInfo, METH_NOARGS, "Get information about our own goal."},
    {"getGPSCoordArray", VisionLink_getGPSCoordArray, METH_NOARGS, "Get information about our own goal."},
    {"getSelfLocation", VisionLink_getSelfLocation, METH_NOARGS, "Get self location.\n returns (confidence, x,y, heading) "},
    {"getBallLocation", VisionLink_getBallLocation, METH_NOARGS, "Get ball location in absolute coordinate(global).\n returns (confidence,x,y,distance) "},
    {"getTeamColor", VisionLink_getTeamColor, METH_NOARGS, "Get team color.\n returns 0 : blue, 1 : red "},
    {"getGPSTeammateBallInfo", VisionLink_getGPSTeammateBallInfo, METH_VARARGS, "Get ball info from teammate.\n getGPSTeammateBallInfo(tm) , tm = 0..3, returns pseudo WMObj"},

    /* 
     * Vision functions
     */ 
    {"getVisualObject", VisionLink_getVisualObject, METH_VARARGS, "Get visual information. index matches vob[] array "},
    {"getColouredPixelCount", VisionLink_dummy, METH_NOARGS, "Count the number of pixels of a given colour.[not available]"},
    {"getBallDistance", VisionLink_getBallDistance, METH_NOARGS, "Get distance from the ball to the robot. "},
    {"getBallHeading", VisionLink_getBallHeading, METH_NOARGS, "Get heading of the ball relative to the robot. "},
    {"getBallConfidence", VisionLink_getBallConfidence, METH_NOARGS, "Get confidence of the ball . "},
    {"reloadNNMC", VisionLink_reloadNNMC, METH_NOARGS, "Get confidence of the ball . "},
    {"setUseBeacons", VisionLink_setUseBeacons, METH_VARARGS, "Sets whether visualcortex looks for beacons and goals."},
    {"setGrabbed", VisionLink_setGrabbed, METH_NOARGS, "Tell vision we are currently in a grab."},
    {"setGrabbing", VisionLink_setGrabbing, METH_NOARGS, "Tell vision we are currently trying to grab."},
    
    /* 
     * Wireless functions
     */ 
    {"getSharedBallInfo", VisionLink_getSharedBallInfo, METH_NOARGS, "Get information about the ball sent by our teammates."},
    {"getWirelessTeammateInfo", VisionLink_getWirelessTeammateInfo, METH_NOARGS, "Get information of wireless teammates."},
	
	/*
	 * GameController functions
	 */
    {"getTheCurrentMode", VisionLink_getTheCurrentMode, METH_NOARGS, "Get my current robot mode."},
    {"getPenalised", VisionLink_getPenalised, METH_NOARGS, "Get my current robot penalty state."},
    {"getDogPenalised", VisionLink_getDogPenalised, METH_NOARGS, "Get the robot penalty state of a specified dog. Usage: getDogPenalised(dognum)"},
    {"getKickOffTeam", VisionLink_getKickOffTeam, METH_NOARGS, "Get the team color of which team is kicking off"},
    {"getDropInTeam", VisionLink_getDropInTeam, METH_NOARGS, "Gets the last team that kicked the ball out."},
    {"getDropInTime", VisionLink_getDropInTime, METH_NOARGS, "Gets the time since the last drop in, in seconds."},
    {"getScore", VisionLink_getScore, METH_NOARGS, "Gets the game score as a tuple (us, them)."},
    {"gameDataIsValid", VisionLink_gameDataIsValid, METH_NOARGS, "Returns 1 if the game controller data is recent, else 0."},
	 
    /*
     * Other functions
     */
    {"doBasicBehaviour", VisionLink_doBasicBehaviour, METH_VARARGS, "Perform basic actions that are implemented using C++ codes in 2003."},
    {"getMyPlayerNum", VisionLink_getMyPlayerNum, METH_NOARGS, "Get my own player number."},
    {"getTemperature", VisionLink_getTemperature, METH_NOARGS, "Get the robot's temperature."},
    {"getCPUUsage", VisionLink_getCPUUsage, METH_NOARGS, "Get the cpu usage."},
    {"getCameraFrame",VisionLink_getCameraFrame,METH_NOARGS,"Get the current camera frame."},
    {"getCurrentPreviousFrameID", VisionLink_getCurrentPreviousFrameID, METH_NOARGS, "Get the current and previous OPENR frame ID."},
    {"getCurrentTime",VisionLink_getCurrentTime,METH_NOARGS,"getCurrentTime() -> (seconds, useconds). Get the system time"},
    {"setTimeCritical",VisionLink_setTimeCritical,METH_NOARGS,"Sets time-critical mode; some computations will not be performed"},
    {"clearTimeCritical",VisionLink_clearTimeCritical,METH_NOARGS,"Clears time-critical mode"},
    {"isTimeCritical",VisionLink_isTimeCritical,METH_NOARGS,"Returns 1 if behaviour is currently in time-critical mode, else 0"},

    /* Offline functions */
#ifdef OFFLINE
    {"processFrame", VisionLink_processFrame, METH_VARARGS, "Setup frame information for offline behaviours."},
    {"clearObstacles", VisionLink_clearObstacles, METH_NOARGS, "Clear all obstacle information."},
    {"setObstacle", VisionLink_setObstacle, METH_VARARGS, "Add an obstacle box."},
#endif

    /*
     * Some helper functions that are already availble in C++. We don't want to re-implement it in Python.
     */
    {"setWalkLearningParameters", VisionLink_setWalkLearningParameters, METH_VARARGS, "Send walk learining parameters to actuator control."},
    {"getProjectedBall", VisionLink_getProjectedBall, METH_NOARGS, "Get the ball project point."},
    {"getProjectedPoint", VisionLink_getProjectedPoint, METH_VARARGS, "Project a point to the ground plane."},
//    {"getRedBlobInfo", VisionLink_getRedBlobInfo, METH_NOARGS, "."},
    {"getTestingInfo", VisionLink_getTestingInfo, METH_NOARGS, "."},
    {"printCPlusPlus", VisionLink_printCPlusPlus, METH_VARARGS, "Print C++ variables."},
    {"PointToElevation", VisionLink_PointToElevation, METH_VARARGS, "."},
    {"PointToHeading", VisionLink_PointToHeading, METH_VARARGS, "."},
    {"getHeadingToBestGap", VisionLink_getHeadingToBestGap, METH_NOARGS, "Get the best heading to the goal considering obstacles, see voak."},
    
    {"srand", VisionLink_srand, METH_VARARGS, "."},
    {"random", VisionLink_random, METH_NOARGS, "."},
    {"randrange", VisionLink_randrange, METH_VARARGS, "."},
    
    {"startProfile", VisionLink_startProfile, METH_VARARGS, "Start profiling, filename as string should be given as argument."},
    {"stopProfile", VisionLink_stopProfile, METH_VARARGS, "Stop profiling, filename as string should be given as argument."},
    
    {"ObstacleGPSValid", VisionLink_ObstacleGPSValid, METH_NOARGS, "Will return true (1) if local obstacles were transfered to GPS this frame."},
    {"getNoObstacleInBox",VisionLink_getNoObstacleInBox,METH_VARARGS, "Get the number of obstacles in box area."},    
    {"getNoObstacleInHeading",VisionLink_getNoObstacleInHeading,METH_VARARGS,"Get the number of obstacles in heading region"},
    {"getPointToNearestObstacleInBox",VisionLink_getPointToNearestObstacleInBox,METH_VARARGS, "Get the point to nearest obstacles in box area."},
    {"getBestGap", VisionLink_getBestGap, METH_VARARGS, "Get the best gap to destination considering obstacles."}, 
    {"getNoObstacleBetween", VisionLink_getNoObstacleBetween, METH_VARARGS, "Get the number of obstacle points in the corridor between two points."},  
    {"getFieldFeatureCount", VisionLink_getFieldFeatureCount, METH_NOARGS, "Get the number of field features."},
    {"resetPinkMapping",VisionLink_resetPinkMapping,METH_NOARGS, "reset the pink mapping for localisation challenge"},
    {"initPinkObject",VisionLink_initPinkObject,METH_NOARGS, "initialise the pink landmark, will cull down any low confidence landmark"},
    {"resetGPS",VisionLink_resetGPS,METH_NOARGS, "Explicitly reset the gaussian"},
    {"setIfObstacleUpdate",VisionLink_setIfObstacleUpdate,METH_VARARGS,"Set to turn off the obstacle update"},
    {"resetBall",VisionLink_resetBall,METH_VARARGS, " Reset the position of the ball to a particular place"},
    {"resetPinkUpdate",VisionLink_resetPinkUpdate,METH_NOARGS, " Reset pink position"},    
    {NULL,    NULL, 0, NULL}    /* sentinel */
};

/** Initialisation as VisionLink for online PyEmbed **/
PyObject * initVisionLinkModule(void) {
    PyObject *module;
    if ((module = PyImport_AddModule(VisionLinkModuleName)) == NULL) {
        cout << "error creating VisionLink Module" << endl;
        return NULL;
    } else if (Py_InitModule(VisionLinkModuleName, Vision_methods) == NULL) {
        cout << "error linking VisionLink Module" << endl;
        return NULL;
    }
    return module;
}

/** Initialisation as VisionLink for offline PyEmbed **/
PyMODINIT_FUNC
initVisionLink(void)
{

    (void) Py_InitModule("VisionLink", Vision_methods);
#ifdef OFFLINE
    initOfflineVision();
#endif
}


