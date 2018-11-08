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
 * $Id: NormalWalk.cc 6618 2005-06-14 10:01:24Z nmor250 $
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 *
**/



#include "NormalWalk.h"
#include "../share/CanterCalib.h"

static bool calibrateFromScratch = false;
static bool calibrateNew2005 = true;

double inversePositiveQuadratic(double x, double a, double b, double c) {
  return (-b + sqrt(b*b - 4*a*(c-x))) / 2 / a;
}

/*
 * __05wtcal__99 0 7 0 0 40 90 120 10 25 55 10 -55 5
 */
void NormalWalk::calibrate(double fwd, double lft, double trn) {
  if (PG <= 5) { // if the robot stands still
    fc = sc = tc = 0.0;
    return;
  }

  /* for everything else */
  static const double forward_a = -0.00246348;
  static const double forward_b = 0.570087;
  static const double forward_c = -1.12313;

  static const double left_a = -0.00752726;
  static const double left_b = 0.627452;
  static const double left_c = -0.977274;

  static const double right_a = 0.0112198; 
  static const double right_b = 0.760032;
  static const double right_c = 1.24505; 

  static const double turnCCW_a = -0.031694;
  static const double turnCCW_b = 3.83103;
  static const double turnCCW_c = -11.3907;

  static const double turnCW_a = 0.0220573;
  static const double turnCW_b = 3.19358;
  static const double turnCW_c = 6.42082;

  static const double FORWARD_MAX = 19.0;    // max RAW forward command is about 45 to achieve 19cm/step
  static const double BACKWARD_MAX = -FORWARD_MAX;
  static const double LEFT_MAX = 12.0;       // max RAW left/right command is about 35...
  static const double RIGHT_MAX = -11.0;  
  static const double TURNCCW_MAX = 83.0;    // max RAW turnCCW/CW command is about 35...
  static const double TURNCW_MAX = -78.0;

  if (calibrateNew2005) {
    // capping the values
    if (fwd > FORWARD_MAX)
      fwd = FORWARD_MAX;
    else if (fwd < BACKWARD_MAX)
      fwd = BACKWARD_MAX;
    if (lft > LEFT_MAX)
      lft = LEFT_MAX;
    else if (lft < RIGHT_MAX)
      lft = RIGHT_MAX;
    if (trn > TURNCCW_MAX)
      trn = TURNCCW_MAX;
    else if (trn < TURNCW_MAX)
      trn = TURNCW_MAX;
    
    // calibrate
    if (fwd == 0.0)
      fc = 0;
    else if (fwd > 0)
      fc = inversePositiveQuadratic(fwd, forward_a, forward_b, forward_c);
    else { // fwd < 0
      fwd = -fwd;
      fc = -inversePositiveQuadratic(fwd, forward_a, forward_b, forward_c);
    }

    if (lft == 0.0)
      sc = 0;
    else if (lft > 0)
      sc = inversePositiveQuadratic(lft, left_a, left_b, left_c);
    else // lft < 0
      sc = inversePositiveQuadratic(lft, right_a, right_b, right_c);
    
    if (PG == 25) { // grab turning
      static const double GRAB_TURNCCW = 59.6142;
      static const double GRAB_TURNCW = -45.0449;
      if (trn > GRAB_TURNCCW)
	trn = GRAB_TURNCCW;
      else if (trn < GRAB_TURNCW)
	trn = GRAB_TURNCW;
    
      static const double grabTurnCCW_a = -0.0348883;//-0.0150275;
      static const double grabTurnCCW_b = 3.60197;//2.6725;
      static const double grabTurnCCW_c = -16.9292;//-8.80528;
      
      static const double grabTurnCW_a = 0.017061;//0.0130286;
      static const double grabTurnCW_b = 2.04933;//1.76826;
      static const double grabTurnCW_c = 5.83846;//5.22543;
      
      if (trn == 0.0)
	tc = 0;
      else if (trn > 0)
	tc = inversePositiveQuadratic(trn, grabTurnCCW_a, grabTurnCCW_b, grabTurnCCW_c);
      else // trn < 0
	tc = inversePositiveQuadratic(trn, grabTurnCW_a, grabTurnCW_b, grabTurnCW_c);
    } else { // normal turning
      if (trn == 0.0)
	tc = 0;
      else if (trn > 0)
	tc = inversePositiveQuadratic(trn, turnCCW_a, turnCCW_b, turnCCW_c);
      else // trn < 0
	tc = inversePositiveQuadratic(trn, turnCW_a, turnCW_b, turnCW_c);      
    }
  }
  else if (calibrateFromScratch){
    fc = fwd;
    sc = lft;
    tc = trn;
  }
  else{
    if (fwd > 0.2) fc = (fwd + 0.5) * 5.1;
    else fc = fwd * 5.2;
    
    sc = lft * 4.8;
    
    tc = pow(fabs(trn), 0.75) * 1.97;
    if (trn < 0.0) tc = -tc;
    
    if (fwd > 1.6) tc -= 0.20 * (fwd - 1.6);
    else if (fwd < -0.2) tc -= 0.07 * (fwd + 0.2);
  }
}

void 
NormalWalk::setWalkParams(const struct normWalkParms &newParms) {
    PG = newParms.pg;
    hdb = newParms.hdb;
    hdf = newParms.hdf;
    hfG = newParms.hf;
    hbG = newParms.hb;
    ffo = newParms.ffo;
    fso = newParms.fso;
    bfo = newParms.bfo;
    bso = newParms.bso;    
}

#ifdef CALWALK_OFFLINE
int main() {
  NormalWalk* walk = new NormalWalk();

  AtomicAction a;
  a.walkType = NormalWalkWT;
  a.Forward = 0;
  a.Left = 0;
  a.turnCCW = 0.0;

  a.speed = 40;
  a.hF = 90;
  a.hB = 110;
  a.hdF = 20;
  a.hdB = 20;
  a.ffO = 59;
  a.fsO = 10;
  a.bfO = -50;
  a.bsO = 5;

  double fwd, lft, trn;
  // maximum fwd just will get capped is b/t 57-58
  // maximum lft just will get capped is b/t 89-90
  // maximum trn just will get capped is b/t 33-34
  /*  
  cout << "#finding the limit" << endl;
  for (fwd = 0; fwd < 50; fwd +=1)
    for(lft = 0; lft < 80; lft+=1)
      for (trn = 0; trn < 60; trn+=1) {
	a.Forward = fwd; a.Left = lft; a.turnCCW = trn;
	walk->setParameters(a);
	if (walk->alpha < 1 && walk->alpha > 0.95)
	  cout << fwd << " " << lft << " " << trn << endl;
      }
  */
  while(1) {
    cout << "please enter fwd & lft & trn: ";
    cin >> fwd >> lft >> trn;
  
    double fwd2 = fwd - 12.74;
    double lft2 = lft - 7.253;
    double trn2 = trn - 44.8;
    double temp_fwd, temp_lft, temp_trn;
    temp_fwd = 7.545*fwd2 + 2.058*trn2;
    temp_lft = 0; //3.508*fwd2 + 1.637*lft2 + 1.277*trn2;
    temp_trn = 24.863*fwd2 + 9.054*trn2;
    
    fwd = (fwd > temp_fwd) ? fwd : temp_fwd;
    lft = (lft > temp_lft) ? lft : temp_lft;
    trn = (trn > temp_trn) ? trn : temp_trn;
    
    cout << "before calibration, fwd / lft / trn is " << fwd << " " << lft << " " << trn << endl;
    walk->calibrate(fwd, lft, trn);
    cout << "after calibration, fc / sc / tc is " << walk->fc << " " << walk->sc << " " << walk->tc << endl;
  }
}
#endif //CALWALK_OFFLINE


void NormalWalk::getOdometry(double &delta_forward, double &delta_left, double
&delta_turn, bool highGain) {
  if (calibrateNew2005) {
    if (PG <= 5) { // if the dog stand still
      delta_forward = delta_left = delta_turn = 0;
    }
    delta_forward = forward / 2.0 / PG;
    delta_left = left / 2.0 / PG;
    delta_turn = turnCCW / 2.0 / PG;
    return;
  }

    if (PG == 0) {
        cout << "PG is Zero!" << endl;
        delta_forward = 0.0;
        delta_left = 0.0;
        delta_turn = 0.0;

    }
    else if (PG == 32){ //hack: normal side way walk
        //Andrew: Not correct.
        //delta_forward = forward / 2.0 / PG * 2.7647999999999997;
        //delta_left = left / 2.0 / PG * 0.76800000000000002;
        //delta_turn = turnCCW / 2.0 / PG * 2.91072 ;
        if (highGain){
            delta_forward = forward / 2.0 / PG * 2.3341648582666039; // 1.952;
            delta_left = left / 2.0 / PG * 0.84422566910467889 ; // 1.012363;
            delta_turn = turnCCW / 2.0 / PG * 3.9519725557461407; //4.2496;
        }
        else{
            delta_forward = forward / 2.0 / PG * 1.6209901112941338;// 2.3341648582666039; // 1.952;
            delta_left = left / 2.0 / PG * 0.28244989758042549; //0.84422566910467889 ; // 1.012363;
            delta_turn = turnCCW / 2.0 / PG * 2.7494033412887826; //3.9519725557461407; //4.2496;
        }
        //cout << "Hacked " << delta_forward << " " << delta_left << " " << delta_turn << endl;
    }
/* TODO:Weiming Can you please make it general? Don't hardcoded forward to 0 and left to 0!
    If a dog have the same setting but not used for grab turning this will hardcode the forward and left to 0
    Makes GPS more mixed up, when doing moving with this dog setting.
*/    
    else if (PG == 25 && hf == 85 && hb == 100 && hdf == 18) {
      // hack: for Action.GrabTurnFast 17/03/2005     
      delta_forward = 0;
      delta_left = 0;
      delta_turn = turnCCW / 2.0 / PG * 0.708288223;//should changed ? to a bit less so it turn more to check the goal, 0.708288223; // 1.0;
      //mean 3.46375 seconds for 360 degree
    }     
    else if (PG == 25 && hf == 80 && hb == 100 && hdf == 18) {
      // hack: for Action.TurnKick for 22/04/2005
      delta_forward = 0;
      delta_left = 0;
      delta_turn = turnCCW / 2.0 / PG * 0.708288223;
    }  
    else {
        if (highGain) {
            delta_forward = forward / 2.0 / PG * 1.81;
            delta_left = left / 2.0 / PG * 1.2416;
            delta_turn = turnCCW / 2.0 / PG * 1.7926;
            /*
            if (abs(turnCCW) < 5 && left != 0){
                if (left > 0) 
                    delta_turn -= 0.0126 * left ;
                else if (left < 0) 
                    delta_turn += 0.0036 * left ;
            }
            */
        } else {
            delta_forward = forward / 2.0 / PG * 1.79;
            delta_left = left / 2.0 / PG * 0.78;
            delta_turn = turnCCW / 2.0 / PG * 1.14;        
        }
    }


    if (delta_forward != delta_forward) {
        cout << "delta_forward NaN" << endl;
    }
    if (delta_left != delta_left) {
        cout << "delta_left NaN" << endl;
    }
    if (delta_turn != delta_turn) {
        cout << "delta_turn NaN" << endl;
    }

}



void NormalWalk::getFLStartEnd(double &fstart, double &sstart, double &fend, double &send){
    /* work out start and end of paw in f-s plane */
    fstart = fc - tc * sin5 + ffo;
    fend   = -fc + tc * sin5 + ffo;
    sstart = sc + tc * cos5 + fso;
    send   = -sc - tc * cos5 + fso;
}

void NormalWalk::getFRStartEnd(double &fstart, double &sstart, double &fend, double &send){
    /* Work out start and end of paw positions in f-s plane, or the limits of the walk locus. */
    fstart = fc + tc * sin5 + ffo;
    fend   = -fc - tc * sin5 + ffo;
    sstart = -sc - tc * cos5 + fso;
    send   = sc + tc * cos5 + fso;
}

void NormalWalk::getBLStartEnd(double &fstart, double &sstart, double &fend, double &send){
    /* work out start and end of paw in f-s plane */
    fstart = fc - tc * sin5 + bfo;
    fend   = -fc + tc * sin5 + bfo;
    sstart = sc - tc * cos5 + bso;
    send   = -sc + tc * cos5 + bso;
}

void NormalWalk::getBRStartEnd(double &fstart, double &sstart, double &fend, double &send){
    /* work out start and end of paw in f-s plane */
    fstart = fc + tc * sin5 + bfo;
    fend   = -fc - tc * sin5 + bfo;
    sstart = -sc + tc * cos5 + bso;
    send   = sc - tc * cos5 + bso;
}


void NormalWalk::getFLPG(double fstart, double sstart, double fend, double send, int &PGR, int &PGH){
    double temp1, temp2, dd;
    /* work out locus length */
    temp1 = fstart - fend;
    temp2 = sstart - send;
    dd = sqrt(temp1 * temp1 + temp2 * temp2);

    PGR = dd == 0 ? PG :(int)((double) PG /(1 + hdf / dd));
    PGH = (PG - PGR) / 2;
}

void NormalWalk::getFRPG(double fstart, double sstart, double fend, double send, int &PGR, int &PGH){
    double temp1, temp2, dd;
    /* Work out the projected locus length on the f-s plane. */
    temp1 = fstart - fend;
    temp2 = sstart - send;
    dd = sqrt(temp1 * temp1 + temp2 * temp2);

    PGR = dd == 0 ? PG :(int)((double) PG /(1 + hdf / dd));
    PGH = (PG - PGR) / 2;
}

void NormalWalk::getBLPG(double fstart, double sstart, double fend, double send, int &PGR, int &PGH){
    double temp1, temp2, dd;
    /* work out locus length */
    temp1 = fstart - fend;
    temp2 = sstart - send;
    dd = sqrt(temp1 * temp1 + temp2 * temp2);

    PGR = dd == 0 ? PG :(int)((double) PG /(1 + hdb / dd));
    PGH = (PG - PGR) / 2;
}

void NormalWalk::getBRPG(double fstart, double sstart, double fend, double send, int &PGR, int &PGH){
    double temp1, temp2, dd;
    /* work out locus length */
    temp1 = fstart - fend;
    temp2 = sstart - send;
    dd = sqrt(temp1 * temp1 + temp2 * temp2);

    PGR = dd == 0 ? PG :(int)((double) PG /(1 + hdb / dd));
    PGH = (PG - PGR) / 2;
}

void NormalWalk::getFLCornerPoint(int pointID, double &f, double &s, double &h) {
    double fstart, fend, sstart, send;

    fstart = fc - tc * sin5 + ffo;
    fend   = -fc + tc * sin5 + ffo;
    sstart = sc + tc * cos5 + fso;
    send   = -sc - tc * cos5 + fso;

    double cH = (walktype == CanterWalkWT) ? canterHeight : 0;

    switch (pointID) {
        case 1:
            f = fstart;
            s = sstart;
            h = hfG + cH;
            break;
        case 2:
            f = fstart;
            s = sstart;
            h = hfG - hdf - cH;
            break;
        case 3:
            f = fend;
            s = send;
            h = hfG - hdf - cH;
            break;
        case 4:
            f = fend;
            s = send;
            h = hfG + cH;
            break;
        default:
            f = s = h = 0;
            break;
    }
}

void NormalWalk::getFRCornerPoint(int pointID, double &f, double &s, double &h) {
    double fstart, fend, sstart, send;

    fstart = fc + tc * sin5 + ffo;
    fend   = -fc - tc * sin5 + ffo;
    sstart = -sc - tc * cos5 + fso;
    send   = sc + tc * cos5 + fso;

    double cH = (walktype == CanterWalkWT) ? canterHeight : 0;

    switch (pointID) {
        case 1:
            f = fstart;
            s = sstart;
            h = hfG + cH;
            break;
        case 2:
            f = fstart;
            s = sstart;
            h = hfG - hdf - cH;
            break;
        case 3:
            f = fend;
            s = send;
            h = hfG - hdf - cH;
            break;
        case 4:
            f = fend;
            s = send;
            h = hfG + cH;
            break;
        default:
            f = s = h = 0;
            break;
    }

}

void NormalWalk::getBLCornerPoint(int pointID, double &f, double &s, double &h) {
    double fstart, fend, sstart, send;

    fstart = fc - tc * sin5 + bfo;
    fend   = -fc + tc * sin5 + bfo;
    sstart = sc - tc * cos5 + bso;
    send   = -sc + tc * cos5 + bso;

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

void NormalWalk::getBRCornerPoint(int pointID, double &f, double &s, double &h) {
	double fstart, fend, sstart, send;

	fstart = fc + tc * sin5 + bfo;
	fend   = -fc - tc * sin5 + bfo;
	sstart = -sc + tc * cos5 + bso;
	send   = sc - tc * cos5 + bso;
	
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



void NormalWalk::frontLeft(double &f, double &s, double &h)
{
    double fstart, fend, sstart, send, fstep, sstep;
    int PGH, PGR;

    getFLStartEnd(fstart, sstart, fend, send);
    getFLPG(fstart, sstart, fend, send, PGR, PGH);
    /* work out points on rectangular locus */
    if (step_ < PG / 2) {
        fstep = (fstart - fend) / PG;
        sstep = (sstart - send) / PG;
        f = (fstart + fend) / 2 - step_ * fstep;
        s = (sstart + send) / 2 - step_ * sstep;
        h = hf;
    }
    if (step_ < PG / 2 + PGH && step_ >= PG / 2 && PGH != 0) {
        f = fend;
        s = send;
        h = hf - (step_ - PG / 2) * hdf / PGH;
    }
    if (step_ < PG / 2 + PGH + PGR && step_ >= PG / 2 + PGH) {
        fstep = (fstart - fend) / PGR;
        sstep = (sstart - send) / PGR;
        f = fend + (step_ - PG / 2 - PGH) * fstep;
        s = send + (step_ - PG / 2 - PGH) * sstep;
        h = hf - hdf;
    }
    if (step_ < PG / 2 + 2 * PGH + PGR
            && step_ >= PG / 2 + PGH + PGR && PGH != 0) {
        f = fstart;
        s = sstart;
        h = hf - hdf + hdf / PGH *(step_ - PG / 2 - PGH - PGR);
    }
    if (step_ < 2 * PG && step_ >= PG / 2 + 2 * PGH + PGR) {
        fstep = (fstart - fend) / PG;
        sstep = (sstart - send) / PG;
        f = fstart - fstep *(step_ - PG / 2 - 2 * PGH - PGR);
        s = sstart - sstep *(step_ - PG / 2 - 2 * PGH - PGR);
        h = hf;
    }
}

void NormalWalk::frontRight(double &f, double &s, double &h)
{
    double fstart, fend, sstart, send, fstep, sstep;
    int PGH, PGR;

    getFRStartEnd(fstart, sstart, fend, send);
    getFRPG(fstart, sstart, fend, send, PGR, PGH);
    /* Calculate points in rectangular locus */
    if (step < PG / 2) {
        fstep = (fstart - fend) / PG;
        sstep = (sstart - send) / PG;
        f = (fstart + fend) / 2 - step * fstep;
        s = (sstart + send) / 2 - step * sstep;
        h = hf;
    }
    if (step < PG / 2 + PGH && step >= PG / 2 && PGH != 0) {
        f = fend;
        s = send;
        h = hf - (step - PG / 2) * hdf / PGH;
    }
    if (step < PG / 2 + PGH + PGR && step >= PG / 2 + PGH) {
        fstep = (fstart - fend) / PGR;
        sstep = (sstart - send) / PGR;
        f = fend + (step - PG / 2 - PGH) * fstep;
        s = send + (step - PG / 2 - PGH) * sstep;
        h = hf - hdf;
    }
    if (step < PG / 2 + 2 * PGH + PGR
            && step >= PG / 2 + PGH + PGR && PGH != 0) {
        f = fstart;
        s = sstart;
        h = hf - hdf + hdf / PGH *(step - PG / 2 - PGH - PGR);
    }
    if (step < 2 * PG && step >= PG / 2 + 2 * PGH + PGR) {
        fstep = (fstart - fend) / PG;
        sstep = (sstart - send) / PG;
        f = fstart - fstep *(step - PG / 2 - 2 * PGH - PGR);
        s = sstart - sstep *(step - PG / 2 - 2 * PGH - PGR);
        h = hf;
    }
}

void NormalWalk::backLeft(double &f, double &s, double &h)
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


void NormalWalk::backRight(double &f, double &s, double &h)
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

