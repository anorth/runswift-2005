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
 * $Id: BackWalk.h 4364 2004-09-22 07:04:12Z kcph007 $
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 *
**/



#ifndef _BackWalk_h
#define _BackWalk_h

#include "QuadWalk.h"

class BackWalk : public QuadWalk {
	public:

	BackWalk() : QuadWalk() {
        useBinarySearchCalibration = false;
	}
	
	virtual ~BackWalk() {
	}
	
	protected:

	virtual void calibrate(double f, double l, double t);

    virtual void getFLStartEnd(double &fstart, double &sstart, double &fend, double &send);
    virtual void getFLPG(double fstart, double sstart, double fend, double send, int &PGR, int &PGH);
    virtual void frontLeft(double &f, double &s, double &h);
    virtual void getFLCornerPoint(int pointID, double &f, double &s, double &h);

    virtual void getFRStartEnd(double &fstart, double &sstart, double &fend, double &send);
    virtual void getFRPG(double fstart, double sstart, double fend, double send, int &PGR, int &PGH);
    virtual void frontRight(double &f, double &s, double &h);
    virtual void getFRCornerPoint(int pointID, double &f, double &s, double &h);

    virtual void getBLStartEnd(double &fstart, double &sstart, double &fend, double &send);
    virtual void getBLPG(double fstart, double sstart, double fend, double send, int &PGR, int &PGH);
    virtual void backLeft(double &f, double &s, double &h);
    virtual void getBLCornerPoint(int pointID, double &f, double &s, double &h);

    virtual void getBRStartEnd(double &fstart, double &sstart, double &fend, double &send);
    virtual void getBRPG(double fstart, double sstart, double fend, double send, int &PGR, int &PGH);
    virtual void backRight(double &f, double &s, double &h);
    virtual void getBRCornerPoint(int pointID, double &f, double &s, double &h);

	
};

#endif // _BackWalk_h
