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
 * $Id: OffsetWalk.h 4364 2004-09-22 07:04:12Z kcph007 $
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 *
**/



#ifndef _OffsetWalk_h
#define _OffsetWalk_h

#include "QuadWalk.h"
#include "../share/OffsetDef.h"

class OffsetWalk : public QuadWalk {
	protected:
    /* Locus point offsets. */
    double bfl1, bsl1, bhl1, cfl1, csl1, chl1, dfl1, dsl1, dhl1, efl1, esl1, ehl1;
    double bfl2, bsl2, bhl2, cfl2, csl2, chl2, dfl2, dsl2, dhl2, efl2, esl2, ehl2;
	
	virtual void calibrate(double f, double l, double t);

    virtual void frontLeft(double &f, double &s, double &h);
    virtual void getFLCornerPoint(int pointID, double &f, double &s, double &h);
    virtual void frontRight(double &f, double &s, double &h);
    virtual void getFRCornerPoint(int pointID, double &f, double &s, double &h);
    virtual void backLeft(double &f, double &s, double &h);
    virtual void getBLCornerPoint(int pointID, double &f, double &s, double &h);
    virtual void backRight(double &f, double &s, double &h);
    virtual void getBRCornerPoint(int pointID, double &f, double &s, double &h);
	
	public:
	
	OffsetWalk() {
	}
	
	virtual ~OffsetWalk() {
	}
	
	void readLocusOffets(const char *filename);
	void setOffsetParams(const OffsetParams *nextOP);
};

#endif // _OffsetWalk_h
