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
 * $Id: BackWalk.cc 4364 2004-09-22 07:04:12Z kcph007 $
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 *
**/



#include "BackWalk.h"

void BackWalk::calibrate(double fwd, double lft, double trn) {
    /* Calibration of normal walk */
    /* last term is to correct turning on sideways movements BH 3/8/00 */
    if (fwd > 0.2) fc = (fwd + 0.5) * 5.1;
    else fc = fwd * 5.2;

    sc = lft * 4.8;
    tc = pow(fabs(trn), 0.75) * 1.97;
    if (trn < 0.0) tc = -tc;

    if (fwd > 1.6) tc -= 0.20 * (fwd - 1.6);
    else if (fwd < -0.2) tc -= 0.07 * (fwd + 0.2);

}

void BackWalk::getFLStartEnd(double &fstart, double &sstart, double &fend, double &send){
    /* work out start and end of paw in f-s plane */
    fstart = ffo;
    fend   = ffo;
    sstart = fso;
    send   = fso;
}

void BackWalk::getFRStartEnd(double &fstart, double &sstart, double &fend, double &send){
    /* Work out start and end of paw positions in f-s plane, or the limits of the walk locus. */
    fstart = ffo;
    fend   = ffo;
    sstart = fso;
    send   = fso;
}

void BackWalk::getBLStartEnd(double &fstart, double &sstart, double &fend, double &send){
    /* work out start and end of paw in f-s plane */
    fstart = fc - tc * sin5 + bfo;
    fend   = -fc + tc * sin5 + bfo;
    sstart = sc - tc * cos5 + bso;
    send   = -sc + tc * cos5 + bso;
}

void BackWalk::getBRStartEnd(double &fstart, double &sstart, double &fend, double &send){
    /* work out start and end of paw in f-s plane */
    fstart = fc + tc * sin5 + bfo;
    fend   = -fc - tc * sin5 + bfo;
    sstart = -sc + tc * cos5 + bso;
    send   = sc - tc * cos5 + bso;
}


void BackWalk::getFLPG(double fstart, double sstart, double fend, double send, int &PGR, int &PGH){
    double temp1, temp2, dd;
    /* work out locus length */
    temp1 = fstart - fend;
    temp2 = sstart - send;
    dd = sqrt(temp1 * temp1 + temp2 * temp2);

    PGR = dd == 0 ? PG :(int)((double) PG /(1 + hdf / dd));
    PGH = (PG - PGR) / 2;
}

void BackWalk::getFRPG(double fstart, double sstart, double fend, double send, int &PGR, int &PGH){
    double temp1, temp2, dd;
    /* Work out the projected locus length on the f-s plane. */
    temp1 = fstart - fend;
    temp2 = sstart - send;
    dd = sqrt(temp1 * temp1 + temp2 * temp2);

    PGR = dd == 0 ? PG :(int)((double) PG /(1 + hdf / dd));
    PGH = (PG - PGR) / 2;
}

void BackWalk::getBLPG(double fstart, double sstart, double fend, double send, int &PGR, int &PGH){
    double temp1, temp2, dd;
    /* work out locus length */
    temp1 = fstart - fend;
    temp2 = sstart - send;
    dd = sqrt(temp1 * temp1 + temp2 * temp2);

    PGR = dd == 0 ? PG :(int)((double) PG /(1 + hdb / dd));
    PGH = (PG - PGR) / 2;
}

void BackWalk::getBRPG(double fstart, double sstart, double fend, double send, int &PGR, int &PGH){
    double temp1, temp2, dd;
    /* work out locus length */
    temp1 = fstart - fend;
    temp2 = sstart - send;
    dd = sqrt(temp1 * temp1 + temp2 * temp2);

    PGR = dd == 0 ? PG :(int)((double) PG /(1 + hdb / dd));
    PGH = (PG - PGR) / 2;
}

void BackWalk::getFLCornerPoint(int pointID, double &f, double &s, double &h) {
    switch (pointID) {
        case 1:
        case 2:
        case 3:
        case 4:
            f = ffo;
            s = fso;
            h = hfG ;
            break;
        default:
            f = s = h = 0;
            break;
    }
}

void BackWalk::getFRCornerPoint(int pointID, double &f, double &s, double &h) {
    switch (pointID) {
        case 1:
        case 2:
        case 3:
        case 4:
            f = ffo;
            s = fso;
            h = hfG ;
            break;
        default:
            f = s = h = 0;
            break;
    }
}

void BackWalk::getBLCornerPoint(int pointID, double &f, double &s, double &h) {
    double fstart, fend, sstart, send;

    getBLStartEnd(fstart,sstart,fend,send);

    double cH = (walktype == CanterWalkWT) ? canterHeight : 0;

    switch (pointID) {
        case 1:
            f = fstart;
            s = sstart;
            h = hbG + cH;
            break;
        case 2:
            f = fstart;
            s = sstart;
            h = hbG - hdb - cH;
            break;
        case 3:
            f = fend;
            s = send;
            h = hbG - hdb - cH;
            break;
        case 4:
            f = fend;
            s = send;
            h = hbG + cH;
            break;
        default:
            f = s = h = 0;
            break;
    }

}

void BackWalk::getBRCornerPoint(int pointID, double &f, double &s, double &h) {
	double fstart, fend, sstart, send;

    getBRStartEnd(fstart,sstart,fend,send);
	
	double cH = (walktype == CanterWalkWT) ? canterHeight : 0;
	
	switch (pointID) {
		case 1:
			f = fstart;
			s = sstart;
			h = hbG + cH;
			break;
		case 2:
			f = fstart;
			s = sstart;
			h = hbG - hdb - cH;
			break;
		case 3:
			f = fend;
			s = send;
			h = hbG - hdb - cH;
			break;
		case 4:
			f = fend;
			s = send;
			h = hbG + cH;
			break;
		default:
			f = s = h = 0;
			break;
	}
}

void BackWalk::frontLeft(double &f, double &s, double &h)
{
    f = ffo;
    s = fso;
    h = hfG;
}

void BackWalk::frontRight(double &f, double &s, double &h)
{
    f = ffo;
    s = fso;
    h = hfG;
}

void BackWalk::backLeft(double &f, double &s, double &h)
{
    double fstart, fend, sstart, send, fstep, sstep;
    int PGH, PGR;

    getBLStartEnd(fstart, sstart, fend, send);
    getBLPG(fstart, sstart, fend, send, PGR, PGH);
    /* work out points in rectangular locus */
    if (step < PG / 2) {
        fstep = (fstart - fend) / PG;
        sstep = (sstart - send) / PG;
        f = (fstart + fend) / 2 - step * fstep;
        s = (sstart + send) / 2 - step * sstep;
        h = hb;
    }
    if (step < PG / 2 + PGH && step >= PG / 2 && PGH != 0) {

        f = fend;
        s = send;
        h = hb - (step - PG / 2) * hdb / PGH;
    }
    if (step < PG / 2 + PGH + PGR && step >= PG / 2 + PGH) {

        fstep = (fstart - fend) / PGR;
        sstep = (sstart - send) / PGR;
        f = fend + (step - PG / 2 - PGH) * fstep;
        s = send + (step - PG / 2 - PGH) * sstep;
        h = hb - hdb;
    }
    if (step < PG / 2 + 2 * PGH + PGR
            && step >= PG / 2 + PGH + PGR && PGH != 0) {

        f = fstart;
        s = sstart;
        h = hb - hdb + hdb / PGH *(step - PG / 2 - PGH - PGR);
    }
    if (step < 2 * PG && step >= PG / 2 + 2 * PGH + PGR) {

        fstep = (fstart - fend) / PG;
        sstep = (sstart - send) / PG;
        f = fstart - fstep *(step - PG / 2 - 2 * PGH - PGR);
        s = sstart - sstep *(step - PG / 2 - 2 * PGH - PGR);
        h = hb;
    }
}

void BackWalk::backRight(double &f, double &s, double &h)
{
    double fstart, fend, sstart, send, fstep, sstep;
    int PGH, PGR;

    getBRStartEnd(fstart, sstart, fend, send);
    getBRPG(fstart, sstart, fend, send, PGR, PGH);
    /* work out points in rectangular locus */
    if (step_ < PG / 2) {

        fstep = (fstart - fend) / PG;
        sstep = (sstart - send) / PG;
        f = (fstart + fend) / 2 - step_ * fstep;
        s = (sstart + send) / 2 - step_ * sstep;
        h = hb;
    }
    if (step_ < PG / 2 + PGH && step_ >= PG / 2 && PGH != 0) {

        f = fend;
        s = send;
        h = hb - (step_ - PG / 2) * hdb / PGH;
    }
    if (step_ < PG / 2 + PGH + PGR && step_ >= PG / 2 + PGH) {

        fstep = (fstart - fend) / PGR;
        sstep = (sstart - send) / PGR;
        f = fend + (step_ - PG / 2 - PGH) * fstep;
        s = send + (step_ - PG / 2 - PGH) * sstep;
        h = hb - hdb;
    }
    if (step_ < PG / 2 + 2 * PGH + PGR
            && step_ >= PG / 2 + PGH + PGR && PGH != 0) {

        f = fstart;
        s = sstart;
        h = hb - hdb + hdb / PGH *(step_ - PG / 2 - PGH - PGR);
    }
    if (step_ < 2 * PG && step_ >= PG / 2 + 2 * PGH + PGR) {

        fstep = (fstart - fend) / PG;
        sstep = (sstart - send) / PG;
        f = fstart - fstep *(step_ - PG / 2 - 2 * PGH - PGR);
        s = sstart - sstep *(step_ - PG / 2 - 2 * PGH - PGR);
        h = hb;
    }
}

