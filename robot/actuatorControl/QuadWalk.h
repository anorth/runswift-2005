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
 * $Id: QuadWalk.h 6046 2005-05-18 03:41:49Z weiming $
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 *
**/



#ifndef _QuadWalk_h
#define _QuadWalk_h

#include "BaseAction.h"
#include "../share/ActionProDef.h"

class QuadWalk : public BaseAction {
 public:
  double alpha; // for capping forward/left/turn
#ifndef OFFLINE
  protected:
#endif
    bool useBinarySearchCalibration;
    /* Forward, sideways and turn components in mm. */
    double fc, sc, tc;
    int step_;  /* one step_ per frame */
    int step;   /* step = step_ 180 deg out of phase */

    /* height forward, the vertical height in mm from the shoulder joint to the ground
     * The point on the ground vertically below the shoulder joint is called "shoulder ground"
     **/
    double hf;

    /* height back, the vertical height in mm from the hip joint to the ground
     * The point on the ground vertically below the hip joint is called "hip ground"
     **/
    double hb;

    /* height delta forward, the height in mm that the robot lifts its front paws when walking,
     * that is, the height of the walk locus for the front paws.
     **/
    double hdf;

    /* height delta back, the height in mm that the robot lifts its back paws when walking,
     * that is, the height of the walk locus for the back paws.
     **/
    double hdb;

    /* front forward offset, home position of the front paws in mm forward from shoulder ground
     **/
    double ffo;

    /* front sideways offset, home position of the front paws in mm sideways from shoulder ground.
     * Larger values spread the legs outwards, smaller values bring them in closer
     **/
    double fso;

    /* back forward offset, home position of the back paws in mm forward from hip ground 
     **/
    double bfo;

    /* back sideways offset, home position of the back paws in mm sideways from hip ground.
     * Larger values spread the legs outwards, smaller values bring them in closer
     **/
    double bso;

    double hfG;
    double hbG;

    double forward, left, turnCCW;

    /* period ground, the time taken to complete a half-step 
     *(for the robot to move along the ground, or the bottom edge of the walk locus
     * PG = numOfFrames * number of camera frames per half-step
     **/
    int PG;                        /* steps per ground stroke */

    /* Robot body length constants. */
    double theta5, sin5, cos5;

    /* Latest calculated joint angles in microrads. */
    double jfl1, jfl2, jfl3, jfr1, jfr2, jfr3, jbl1, jbl2, jbl3, jbr1, jbr2, jbr3;

    
	double canterHeight;

    /* specifies the actual type of locomotion action to execute. 
     * Walktypes are definitions used to decide whether to execute a particular walking style or kicking action.
     *(exception: aimable forward kick is a kicking action that ignore all other parameter values)
     **/
    int walktype;

    bool noSolution;



	public:

		QuadWalk();

		virtual ~QuadWalk() {
		}

        // called each time a half-step completes (when it should do nothing)
        // and when forceComplete is set in PWalk
		virtual void reset() {
            // move legs back to the nearest home position
            if (step_ < PG/2)
                step_ = 0;
            else if (step_ < 3*PG / 2)
                step_ = PG;
            else
                step_ = 2 * PG;
		}

		virtual void setParameters(const AtomicAction &aa);

		virtual void setCurrentJointCommand(JointCommand &j);

		virtual void getOdometry(double &delta_forward, double &delta_left, double &delta_turn, bool highGain);

		virtual void goNextFrame();

		virtual bool isCompleted();

		virtual int getNumPoints();

		virtual int getPointNum();
		virtual void setPointNum(int step);
		
		virtual bool usingHead() {
			return false;
		}

        bool isSpecialAction(){ return false;}
#ifndef OFFLINE
	protected:
#endif
		virtual void calibrate(double f, double l, double t) = 0;

		virtual void frontLeft(double &f, double &s, double &h) = 0;
		virtual void getFLCornerPoint(int pointID, double &f, double &s, double &h) = 0;
    	virtual void frontRight(double &f, double &s, double &h) = 0;
		virtual void getFRCornerPoint(int pointID, double &f, double &s, double &h) = 0;
    	virtual void backLeft(double &f, double &s, double &h) = 0;
		virtual void getBLCornerPoint(int pointID, double &f, double &s, double &h) = 0;
    	virtual void backRight(double &f, double &s, double &h) = 0;
		virtual void getBRCornerPoint(int pointID, double &f, double &s, double &h) = 0;
    	void fsh2xyz(double f, double s, double h, double theta4, double &x, double &y, double &z);
        public: //for offline testing
    	void frontJointAngles(double x, double y, double z, double &theta1, double &theta2, double &theta3);
    	void backJointAngles(double x, double y, double z, double &theta1, double &theta2, double &theta3);
		bool testParams(double alpha, double fwd, double lft, double trn);
		double binSearch(double fwd, double lft, double trn);
};


#endif	// _QuadWalk_h
