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
 * $Id: SpecialAction.h 7490 2005-07-08 11:47:55Z weiming $
 *
 * Copyright(c) 2003 UNSW
 * All Rights Reserved.
 *
 **/

#ifndef _SpecialAction_h_DEFINED
#define _SpecialAction_h_DEFINED

#include <fstream>
#include <iostream>
#include "../share/PWalkDef.h"
#include "JointCommand.h"
#include "BaseAction.h"

using namespace std;

/* posa = position array, format (double):
   duration,
   head tilt, pan, roll,
   right front joint, shoulder, knee,
   left front joint, shoulder, knee,
   right hind joint, shoulder, knee,
   left hind joint, shoulder, knee
   */


class SpecialAction : public BaseAction {
protected:
    int numOfSteps;
    int duration;
    int step;
    int currIndex;

    // how much odometry changes for this action
    // units: cm & degrees
    double dFwd;
    double dLeft;
    double dTurn;

    bool actionCompleted;

    double *posa;
    const char *filename;

    bool isUsingHead;

public:
    SpecialAction() {
        numOfSteps = 0;
        duration = 0;
        posa = NULL;
	dFwd = dLeft = dTurn = 0;
        reset();
    }

    virtual ~SpecialAction() {
        delete[](posa);
    }

    virtual void reset(){
        actionCompleted = false;
        currIndex = 0;
        step = 0;
    }

    virtual void load(const string &str){
        load(str.c_str());
    }

    virtual void load(const char *fname) {
      this->filename = fname;
      isUsingHead = true; // assume is using the head
      
      numOfSteps = 0;
      duration = 0;
      if (posa) {
	delete[](posa);
	posa = NULL;
        }
        ifstream in(fname);
        if (in) {
            in >> numOfSteps;
            //FFCOUT << fname << " steps: " << numOfSteps << endl;

            if (numOfSteps > 0) {
                posa = (double *)(new double[numOfSteps*posaLength]);

                for (int i = 0; i < numOfSteps; ++i) {
                    while (isspace(in.peek())) // read to end of line
                        in.get();
                    //FFCOUT << fname << " line at " << in.tellg() << endl;
                    // ignore lines beginning with '#'
                    if (in.peek() == '#') {
                        while (in.peek() != '\n') // skip line
                            in.get();
                        --i;
                        continue;
                    }

                    if (! in.good()) {
                        FFCOUT << "Error in pos file " << fname
                        << " - unexpected EOF" << endl;
                    }
                    for (int j = 0; j < posaLength; ++j) {
                        while (isspace(in.peek())) // read to non-space char
                            in.get();
                        // check head params
                        if ((j>0 && j<4) && in.peek() == '?') {
                            in.get();
                            isUsingHead = false;
                        } else {
                            in >> posa[i*posaLength+j];
                        }
                    }
                    duration += (int) posa[i*posaLength];
                    posa[i*posaLength] = duration;
                }
            }
            FFCOUT << fname << " : step duration:" << duration 
                << (isUsingHead?"":" is NOT using head.") << endl;
            in.close();
        } else {
            FFCOUT << "cannot open file: " << fname << endl;
        }
    }

    // setting the odometry change for this action
    void setOdometry(double fwd, double left, double turn) {
      if (duration == 0) {
	FFCERR << "Duration is not set yet!!" << endl;
	dFwd = dLeft = dTurn = 0;
	return;
      }
      dFwd = fwd / duration; 
      dLeft = left / duration; 
      dTurn = turn / duration;
    }

    void reload(){
        load(this->filename);
    }

    virtual double getCurrentJointValue(int jointIndex){
        return posa[currIndex * posaLength + jointIndex];
    }

    virtual void setCurrentJointCommand(JointCommand &j) {
        for (int i=1; i<posaLength; ++i) {
            j.setJointValue(i-1,DEG2MICRO(getCurrentJointValue(i)) );
        }
    }

    virtual void getOdometry(double &delta_forward, double &delta_left, double &delta_turn, bool highGain) {
      delta_forward = dFwd;
      delta_left = dLeft;      
      delta_turn = dTurn;
      highGain = false; // we force high gain anyways
    }

    virtual void goNextFrame(){
        step ++;
        if (step >= duration) {
            step = 0;
            currIndex = 0;
            actionCompleted = true;
        }
        else{
            if (step >= posa[currIndex * posaLength] ){
                currIndex ++;
            }
        }
        //cout << "now in step " << step << " ind = " << currIndex << endl;
    }

    virtual bool isCompleted(){
        return actionCompleted;
    }

    virtual int getNumPoints() {
        return (int)duration;
    }

    virtual void setPointNum(int number) {
        step = number;
        currIndex = 0;
        while (step >= posa[currIndex * posaLength] ){
            currIndex ++;
        }
        //cout << "got set number " << step << " currIndex " << endl;
    }

    virtual int getPointNum() {
        return step;
    }

    virtual bool usingHead() {
        return isUsingHead;
    }

    virtual bool isSpecialAction(){
        return true;
    }

};


#endif //_SpecialAction_h_DEFINED

