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
 * $Id: NormalWalk.h 6401 2005-06-03 09:07:29Z nmor250 $
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 *
**/



#ifndef _NormalWalk_h
#define _NormalWalk_h

#include "QuadWalk.h"

class normWalkParms { 
    public:
    int pg; 
    double hdb;
    double hdf;
    double hf;
    double hb;
    double ffo;
    double fso;
    double bfo;
    double bso;
    
    void init() { 
	pg = 40;
        hf = 90;
	hb = 110;
	hdb = 20;
	hdf = 20;
	ffo = 59;
	fso = 10;
	bfo = -50;
	bso = 5;        
    }
    
    void read(std::istream &in) {
        double tempPG;
        in >> tempPG;  // 1
        pg = (int)rint(tempPG); //PG
        in >> hf; //hF
        in >> hb;  //hB
        in >> hdf; //hdF
        in >> hdb; //hdB //5
        in >> ffo;  //ffo
        in >> fso;  //fso
        in >> bfo;  //bfo
        in >> bso;  //bso
    }
    
    void print(std::ostream &out) const {
        out << "[";
        out << pg << ", ";
        out << hf << ", ";
        out << hb << ", ";
        out << hdf << ", ";
        out << hdb << ", ";
        out << ffo << ", ";
        out << fso << ", ";
        out << bfo << ", ";
        out << bso;
        out << "]";
    }

};

class NormalWalk : public QuadWalk {
	public:

	NormalWalk() {
	}
	
	virtual ~NormalWalk() {
	}
	void NormalWalk::getOdometry(double &delta_forward, double &delta_left, double &delta_turn, bool highGain);

        void setWalkParams(const struct normWalkParms &newParms); 

#ifndef OFFLINE
	protected:
#endif
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

#endif // _NormalWalk_h
