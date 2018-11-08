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
 * $Id: OffsetWalk.cc 4364 2004-09-22 07:04:12Z kcph007 $
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 *
**/



#include "OffsetWalk.h"
#include <fstream>

using namespace std;

void OffsetWalk::calibrate(double fwd, double lft, double trn) {
            /* Calibration of OffsetWalkWT
             *
             * NOTE: this calibration is NOT ACCURATE - it has specifically been set for easiest behaviour level
             * integration(for replacing ZoidalWalk)
             * In particular, walking sideways is awful(most likely caused by the offsets being calibrated for
             * walking forwards)
             */
            trn *= 1.2;
            turnCCW = trn;
            fc = (fwd / 8.6666*6 + .04) * 6.0;
            sc = lft * 6.0;
            tc = trn / 18.75 * 10 * 1.5 + sc / 6.0 + fc / 100;
}

void OffsetWalk::frontLeft(double &f, double &s, double &h)
{
	double temp1, temp2, dd;
	double fstart, fend, sstart, send, fstep, sstep, hstep;

	/* work out start and end of paw in f-s plane */
	fstart = fc - tc * sin5 + ffo;
	fend   = -fc + tc * sin5 + ffo;
	sstart = sc + tc * cos5 + fso;
	send   = -sc - tc * cos5 + fso;

	/* work out locus length */
	temp1 = fstart - fend;
	temp2 = sstart - send;
	dd = sqrt(temp1 * temp1 + temp2 * temp2);

			double af, as, ah, bf, bs, bh, cf, cs, ch, df, ds, dh, ef, es, eh;
			double d1, d2, d3, d4, d5, dtotal;
			double t1, t2, t3, t4, t5;

			if (dd != 0) {
				double locusRotate = atan2(fstart-fend,sstart-send) - (pi/2.0);
				bf = fend + bsl1*sin(locusRotate) + bfl1*cos(locusRotate);
				bs = send + bsl1*cos(locusRotate) - bfl1*sin(locusRotate);
				cf = fend + csl1*sin(locusRotate) + cfl1*cos(locusRotate);
				cs = send + csl1*cos(locusRotate) - cfl1*sin(locusRotate);
				df = fstart + dsl1*sin(locusRotate) + dfl1*cos(locusRotate);
				ds = sstart + dsl1*cos(locusRotate) - dfl1*sin(locusRotate);
				ef = fstart + esl1*sin(locusRotate) + efl1*cos(locusRotate);
				es = sstart + esl1*cos(locusRotate) - efl1*sin(locusRotate);
				af = (ef + bf)/2.0;
				as = (es + bs)/2.0;
			}
			else {
				bf = cf = df = ef = af = (fstart + fend)/2.0;
				bs = cs = ds = es = as = (sstart + send)/2.0;
			}

			if (hdf != 0) {
				bh = hf + bhl1;
				ch = hf + chl1 - hdf;
				dh = hf + dhl1 - hdf;
				eh = hf + ehl1;
				ah = (eh + bh)/2.0;
			}
			else {
				bh = ch = dh = eh = ah = hf;
			}

			d1 = sqrt((af-bf)*(af-bf) + (as-bs)*(as-bs) + (ah-bh)*(ah-bh));
			d2 = sqrt((bf-cf)*(bf-cf) + (bs-cs)*(bs-cs) + (bh-ch)*(bh-ch));
			d3 = sqrt((cf-df)*(cf-df) + (cs-ds)*(cs-ds) + (ch-dh)*(ch-dh));
			d4 = sqrt((df-ef)*(df-ef) + (ds-es)*(ds-es) + (dh-eh)*(dh-eh));
			d5 = sqrt((ef-af)*(ef-af) + (es-as)*(es-as) + (eh-ah)*(eh-ah));
			dtotal = d1 + d2 + d3 + d4 + d5;

			if (dtotal != 0) {
				t1 = d1/dtotal * 2 * PG;
				t2 = d2/dtotal * 2 * PG;
				t3 = d3/dtotal * 2 * PG;
				t4 = d4/dtotal * 2 * PG;
				t5 = d5/dtotal * 2 * PG;
			}
			else {
				t1 = t2 = t3 = t4 = 0;
				t5 = 2 * PG;
			}

			if (step_ < t1) {
				fstep = (af - bf)/t1;
				sstep = (as - bs)/t1;
				hstep = (ah - bh)/t1;
				f     = af - step_*fstep;
				s     = as - step_*sstep;
				h     = ah - step_*hstep;
			} 
			else if (step_ < t1 + t2) {
				fstep = (bf - cf)/t2;
				sstep = (bs - cs)/t2;
				hstep = (bh - ch)/t2;
				f     = bf - (step_-t1)*fstep;
				s     = bs - (step_-t1)*sstep;
				h     = bh - (step_-t1)*hstep;
			} 
			else if (step_ < t1 + t2 + t3) {
				fstep = (cf - df)/t3;
				sstep = (cs - ds)/t3;
				hstep = (ch - dh)/t3;
				f     = cf - (step_-t1-t2)*fstep;
				s     = cs - (step_-t1-t2)*sstep;
				h     = ch - (step_-t1-t2)*hstep;
			} 
			else if (step_ < t1 + t2 + t3 + t4) {
				fstep = (df - ef)/t4;
				sstep = (ds - es)/t4;
				hstep = (dh - eh)/t4;
				f     = df - (step_-t1-t2-t3)*fstep;
				s     = ds - (step_-t1-t2-t3)*sstep;
				h     = dh - (step_-t1-t2-t3)*hstep;
			} 
			else {
				fstep = (ef - af)/t5;
				sstep = (es - as)/t5;
				hstep = (eh - ah)/t5;
				f     = ef - (step_-t1-t2-t3-t4)*fstep;
				s     = es - (step_-t1-t2-t3-t4)*sstep;
				h     = eh - (step_-t1-t2-t3-t4)*hstep;
			}
}

void OffsetWalk::getFLCornerPoint(int pointID, double &f, double &s, double &h) {
	double temp1, temp2, dd;
	double fstart, fend, sstart, send;

	/* work out start and end of paw in f-s plane */
	fstart = fc - tc * sin5 + ffo;
	fend   = -fc + tc * sin5 + ffo;
	sstart = sc + tc * cos5 + fso;
	send   = -sc - tc * cos5 + fso;

	/* work out locus length */
	temp1 = fstart - fend;
	temp2 = sstart - send;
	dd = sqrt(temp1 * temp1 + temp2 * temp2);

			double af, as, ah, bf, bs, bh, cf, cs, ch, df, ds, dh, ef, es, eh;

			if (dd != 0) {
				double locusRotate = atan2(fstart-fend,sstart-send) - (pi/2.0);
				bf = fend + bsl1*sin(locusRotate) + bfl1*cos(locusRotate);
				bs = send + bsl1*cos(locusRotate) - bfl1*sin(locusRotate);
				cf = fend + csl1*sin(locusRotate) + cfl1*cos(locusRotate);
				cs = send + csl1*cos(locusRotate) - cfl1*sin(locusRotate);
				df = fstart + dsl1*sin(locusRotate) + dfl1*cos(locusRotate);
				ds = sstart + dsl1*cos(locusRotate) - dfl1*sin(locusRotate);
				ef = fstart + esl1*sin(locusRotate) + efl1*cos(locusRotate);
				es = sstart + esl1*cos(locusRotate) - efl1*sin(locusRotate);
				af = (ef + bf)/2.0;
				as = (es + bs)/2.0;
			}
			else {
				bf = cf = df = ef = af = (fstart + fend)/2.0;
				bs = cs = ds = es = as = (sstart + send)/2.0;
			}

			if (hdf != 0) {
				bh = hf + bhl1;
				ch = hf + chl1 - hdf;
				dh = hf + dhl1 - hdf;
				eh = hf + ehl1;
				ah = (eh + bh)/2.0;
			}
			else {
				bh = ch = dh = eh = ah = hf;
			}

	switch (pointID) {
		case 1:
			f = bf;
			s = bs;
			h = bh;
			break;
		case 2:
			f = cf;
			s = cs;
			h = ch;
			break;
		case 3:
			f = df;
			s = ds;
			h = dh;
			break;
		case 4:
			f = ef;
			s = es;
			h = eh;
			break;
		default:
			f = s = h = 0;
			break;
	}

}

void OffsetWalk::frontRight(double &f, double &s, double &h)
{
	double temp1, temp2, dd;
	double fstart, fend, sstart, send, fstep, sstep, hstep;

	/* Work out start and end of paw positions in f-s plane, or the limits of the walk locus. */
	fstart = fc + tc * sin5 + ffo;
	fend   = -fc - tc * sin5 + ffo;
	sstart = -sc - tc * cos5 + fso;
	send   = sc + tc * cos5 + fso;

	/* Work out the projected locus length on the f-s plane. */
	temp1 = fstart - fend;
	temp2 = sstart - send;
	dd = sqrt(temp1 * temp1 + temp2 * temp2);

			double af, as, ah, bf, bs, bh, cf, cs, ch, df, ds, dh, ef, es, eh;
			double d1, d2, d3, d4, d5, dtotal;
			double t1, t2, t3, t4, t5;

			if (dd != 0) {
				double locusRotate = atan2(fstart-fend,sstart-send) - (pi/2.0);
				bf = fend + bsl1*sin(locusRotate) + bfl1*cos(locusRotate);
				bs = send + bsl1*cos(locusRotate) - bfl1*sin(locusRotate);
				cf = fend + csl1*sin(locusRotate) + cfl1*cos(locusRotate);
				cs = send + csl1*cos(locusRotate) - cfl1*sin(locusRotate);
				df = fstart + dsl1*sin(locusRotate) + dfl1*cos(locusRotate);
				ds = sstart + dsl1*cos(locusRotate) - dfl1*sin(locusRotate);
				ef = fstart + esl1*sin(locusRotate) + efl1*cos(locusRotate);
				es = sstart + esl1*cos(locusRotate) - efl1*sin(locusRotate);
				af = (ef + bf)/2.0;
				as = (es + bs)/2.0;
			}
			else {
				bf = cf = df = ef = af = (fstart + fend)/2.0;
				bs = cs = ds = es = as = (sstart + send)/2.0;
			}

			if (hdf != 0) {
				bh = hf + bhl1;
				ch = hf + chl1 - hdf;
				dh = hf + dhl1 - hdf;
				eh = hf + ehl1;
				ah = (eh + bh)/2.0;
			}
			else {
				bh = ch = dh = eh = ah = hf;
			}

			d1 = sqrt((af-bf)*(af-bf) + (as-bs)*(as-bs) + (ah-bh)*(ah-bh));
			d2 = sqrt((bf-cf)*(bf-cf) + (bs-cs)*(bs-cs) + (bh-ch)*(bh-ch));
			d3 = sqrt((cf-df)*(cf-df) + (cs-ds)*(cs-ds) + (ch-dh)*(ch-dh));
			d4 = sqrt((df-ef)*(df-ef) + (ds-es)*(ds-es) + (dh-eh)*(dh-eh));
			d5 = sqrt((ef-af)*(ef-af) + (es-as)*(es-as) + (eh-ah)*(eh-ah));
			dtotal = d1 + d2 + d3 + d4 + d5;

			if (dtotal != 0) {
				t1 = d1/dtotal * 2 * PG;
				t2 = d2/dtotal * 2 * PG;
				t3 = d3/dtotal * 2 * PG;
				t4 = d4/dtotal * 2 * PG;
				t5 = d5/dtotal * 2 * PG;
			}
			else {
				t1 = t2 = t3 = t4 = 0;
				t5 = 2 * PG;
			}

			if (step < t1) {
				fstep = (af - bf)/t1;
				sstep = (as - bs)/t1;
				hstep = (ah - bh)/t1;
				f     = af - step*fstep;
				s     = as - step*sstep;
				h     = ah - step*hstep;
			} 
			else if (step < t1 + t2) {
				fstep = (bf - cf)/t2;
				sstep = (bs - cs)/t2;
				hstep = (bh - ch)/t2;
				f     = bf - (step-t1)*fstep;
				s     = bs - (step-t1)*sstep;
				h     = bh - (step-t1)*hstep;
			} 
			else if (step < t1 + t2 + t3) {
				fstep = (cf - df)/t3;
				sstep = (cs - ds)/t3;
				hstep = (ch - dh)/t3;
				f     = cf - (step-t1-t2)*fstep;
				s     = cs - (step-t1-t2)*sstep;
				h     = ch - (step-t1-t2)*hstep;
			} 
			else if (step < t1 + t2 + t3 + t4) {
				fstep = (df - ef)/t4;
				sstep = (ds - es)/t4;
				hstep = (dh - eh)/t4;
				f     = df - (step-t1-t2-t3)*fstep;
				s     = ds - (step-t1-t2-t3)*sstep;
				h     = dh - (step-t1-t2-t3)*hstep;
			}
			else {
				fstep = (ef - af)/t5;
				sstep = (es - as)/t5;
				hstep = (eh - ah)/t5;
				f     = ef - (step-t1-t2-t3-t4)*fstep;
				s     = es - (step-t1-t2-t3-t4)*sstep;
				h     = eh - (step-t1-t2-t3-t4)*hstep;
			}
}


void OffsetWalk::getFRCornerPoint(int pointID, double &f, double &s, double &h) {
	double temp1, temp2, dd;
	double fstart, fend, sstart, send;

	/* Work out start and end of paw positions in f-s plane, or the limits of the walk locus. */
	fstart = fc + tc * sin5 + ffo;
	fend   = -fc - tc * sin5 + ffo;
	sstart = -sc - tc * cos5 + fso;
	send   = sc + tc * cos5 + fso;

	/* Work out the projected locus length on the f-s plane. */
	temp1 = fstart - fend;
	temp2 = sstart - send;
	dd = sqrt(temp1 * temp1 + temp2 * temp2);

			double af, as, ah, bf, bs, bh, cf, cs, ch, df, ds, dh, ef, es, eh;

			if (dd != 0) {
				double locusRotate = atan2(fstart-fend,sstart-send) - (pi/2.0);
				bf = fend + bsl1*sin(locusRotate) + bfl1*cos(locusRotate);
				bs = send + bsl1*cos(locusRotate) - bfl1*sin(locusRotate);
				cf = fend + csl1*sin(locusRotate) + cfl1*cos(locusRotate);
				cs = send + csl1*cos(locusRotate) - cfl1*sin(locusRotate);
				df = fstart + dsl1*sin(locusRotate) + dfl1*cos(locusRotate);
				ds = sstart + dsl1*cos(locusRotate) - dfl1*sin(locusRotate);
				ef = fstart + esl1*sin(locusRotate) + efl1*cos(locusRotate);
				es = sstart + esl1*cos(locusRotate) - efl1*sin(locusRotate);
				af = (ef + bf)/2.0;
				as = (es + bs)/2.0;
			}
			else {
				bf = cf = df = ef = af = (fstart + fend)/2.0;
				bs = cs = ds = es = as = (sstart + send)/2.0;
			}

			if (hdf != 0) {
				bh = hf + bhl1;
				ch = hf + chl1 - hdf;
				dh = hf + dhl1 - hdf;
				eh = hf + ehl1;
				ah = (eh + bh)/2.0;
			}
			else {
				bh = ch = dh = eh = ah = hf;
			}

	switch (pointID) {
		case 1:
			f = bf;
			s = bs;
			h = bh;
			break;
		case 2:
			f = cf;
			s = cs;
			h = ch;
			break;
		case 3:
			f = df;
			s = ds;
			h = dh;
			break;
		case 4:
			f = ef;
			s = es;
			h = eh;
			break;
		default:
			f = s = h = 0;
			break;
	}

}


void OffsetWalk::backLeft(double &f, double &s, double &h)
{
	double temp1, temp2, dd;
	double fstart, fend, sstart, send, fstep, sstep, hstep;

	/* work out start and end of paw in f-s plane */
	fstart = fc - tc * sin5 + bfo;
	fend   = -fc + tc * sin5 + bfo;
	sstart = sc - tc * cos5 + bso;
	send   = -sc + tc * cos5 + bso;

	/* work out locus length */
	temp1 = fstart - fend;
	temp2 = sstart - send;
	dd = sqrt(temp1 * temp1 + temp2 * temp2);

			double af, as, ah, bf, bs, bh, cf, cs, ch, df, ds, dh, ef, es, eh;
			double d1, d2, d3, d4, d5, dtotal;
			double t1, t2, t3, t4, t5;

			if (dd != 0) {
				double locusRotate = atan2(fstart-fend,sstart-send) - (pi/2.0);
				bf = fend + bsl2*sin(locusRotate) + bfl2*cos(locusRotate);
				bs = send + bsl2*cos(locusRotate) - bfl2*sin(locusRotate);
				cf = fend + csl2*sin(locusRotate) + cfl2*cos(locusRotate);
				cs = send + csl2*cos(locusRotate) - cfl2*sin(locusRotate);
				df = fstart + dsl2*sin(locusRotate) + dfl2*cos(locusRotate);
				ds = sstart + dsl2*cos(locusRotate) - dfl2*sin(locusRotate);
				ef = fstart + esl2*sin(locusRotate) + efl2*cos(locusRotate);
				es = sstart + esl2*cos(locusRotate) - efl2*sin(locusRotate);
				af = (ef + bf)/2.0;
				as = (es + bs)/2.0;
			}
			else {
				bf = cf = df = ef = af = (fstart + fend)/2.0;
				bs = cs = ds = es = as = (sstart + send)/2.0;
			}

			if (hdb != 0) {
				bh = hb + bhl2;
				ch = hb + chl2 - hdb;
				dh = hb + dhl2 - hdb;
				eh = hb + ehl2;
				ah = (eh + bh)/2.0;
			}
			else {
				bh = ch = dh = eh = ah = hb;
			}

			d1 = sqrt((af-bf)*(af-bf) + (as-bs)*(as-bs) + (ah-bh)*(ah-bh));
			d2 = sqrt((bf-cf)*(bf-cf) + (bs-cs)*(bs-cs) + (bh-ch)*(bh-ch));
			d3 = sqrt((cf-df)*(cf-df) + (cs-ds)*(cs-ds) + (ch-dh)*(ch-dh));
			d4 = sqrt((df-ef)*(df-ef) + (ds-es)*(ds-es) + (dh-eh)*(dh-eh));
			d5 = sqrt((ef-af)*(ef-af) + (es-as)*(es-as) + (eh-ah)*(eh-ah));
			dtotal = d1 + d2 + d3 + d4 + d5;

			if (dtotal != 0) {
				t1 = d1/dtotal * 2 * PG;
				t2 = d2/dtotal * 2 * PG;
				t3 = d3/dtotal * 2 * PG;
				t4 = d4/dtotal * 2 * PG;
				t5 = d5/dtotal * 2 * PG;
			}
			else {
				t1 = t2 = t3 = t4 = 0;
				t5 = 2 * PG;
			}

			if (step < t1) {
				fstep = (af - bf)/t1;
				sstep = (as - bs)/t1;
				hstep = (ah - bh)/t1;
				f     = af - step*fstep;
				s     = as - step*sstep;
				h     = ah - step*hstep;
			} 
			else if (step < t1 + t2) {
				fstep = (bf - cf)/t2;
				sstep = (bs - cs)/t2;
				hstep = (bh - ch)/t2;
				f     = bf - (step-t1)*fstep;
				s     = bs - (step-t1)*sstep;
				h     = bh - (step-t1)*hstep;
			}
			else if (step < t1 + t2 + t3) {
				fstep = (cf - df)/t3;
				sstep = (cs - ds)/t3;
				hstep = (ch - dh)/t3;
				f     = cf - (step-t1-t2)*fstep;
				s     = cs - (step-t1-t2)*sstep;
				h     = ch - (step-t1-t2)*hstep;
			} 
			else if (step < t1 + t2 + t3 + t4) {
				fstep = (df - ef)/t4;
				sstep = (ds - es)/t4;
				hstep = (dh - eh)/t4;
				f     = df - (step-t1-t2-t3)*fstep;
				s     = ds - (step-t1-t2-t3)*sstep;
				h     = dh - (step-t1-t2-t3)*hstep;
			}
			else {
				fstep = (ef - af)/t5;
				sstep = (es - as)/t5;
				hstep = (eh - ah)/t5;
				f     = ef - (step-t1-t2-t3-t4)*fstep;
				s     = es - (step-t1-t2-t3-t4)*sstep;
				h     = eh - (step-t1-t2-t3-t4)*hstep;
			}
}


void OffsetWalk::getBLCornerPoint(int pointID, double &f, double &s, double &h) {
	double temp1, temp2, dd;
	double fstart, fend, sstart, send;

	/* work out start and end of paw in f-s plane */
	fstart = fc - tc * sin5 + bfo;
	fend   = -fc + tc * sin5 + bfo;
	sstart = sc - tc * cos5 + bso;
	send   = -sc + tc * cos5 + bso;

	/* work out locus length */
	temp1 = fstart - fend;
	temp2 = sstart - send;
	dd = sqrt(temp1 * temp1 + temp2 * temp2);

			double af, as, ah, bf, bs, bh, cf, cs, ch, df, ds, dh, ef, es, eh;

			if (dd != 0) {
				double locusRotate = atan2(fstart-fend,sstart-send) - (pi/2.0);
				bf = fend + bsl2*sin(locusRotate) + bfl2*cos(locusRotate);
				bs = send + bsl2*cos(locusRotate) - bfl2*sin(locusRotate);
				cf = fend + csl2*sin(locusRotate) + cfl2*cos(locusRotate);
				cs = send + csl2*cos(locusRotate) - cfl2*sin(locusRotate);
				df = fstart + dsl2*sin(locusRotate) + dfl2*cos(locusRotate);
				ds = sstart + dsl2*cos(locusRotate) - dfl2*sin(locusRotate);
				ef = fstart + esl2*sin(locusRotate) + efl2*cos(locusRotate);
				es = sstart + esl2*cos(locusRotate) - efl2*sin(locusRotate);
				af = (ef + bf)/2.0;
				as = (es + bs)/2.0;
			}
			else {
				bf = cf = df = ef = af = (fstart + fend)/2.0;
				bs = cs = ds = es = as = (sstart + send)/2.0;
			}

			if (hdb != 0) {
				bh = hb + bhl2;
				ch = hb + chl2 - hdb;
				dh = hb + dhl2 - hdb;
				eh = hb + ehl2;
				ah = (eh + bh)/2.0;
			}
			else {
				bh = ch = dh = eh = ah = hb;
			}

	switch (pointID) {
		case 1:
			f = bf;
			s = bs;
			h = bh;
			break;
		case 2:
			f = cf;
			s = cs;
			h = ch;
			break;
		case 3:
			f = df;
			s = ds;
			h = dh;
			break;
		case 4:
			f = ef;
			s = es;
			h = eh;
			break;
		default:
			f = s = h = 0;
			break;
	}

}


void OffsetWalk::backRight(double &f, double &s, double &h)
{
	double temp1, temp2, dd;
	double fstart, fend, sstart, send, fstep, sstep, hstep;

	/* work out start and end of paw in f-s plane */
	fstart = fc + tc * sin5 + bfo;
	fend   = -fc - tc * sin5 + bfo;
	sstart = -sc + tc * cos5 + bso;
	send   = sc - tc * cos5 + bso;

	/* work out locus length */
	temp1 = fstart - fend;
	temp2 = sstart - send;
	dd = sqrt(temp1 * temp1 + temp2 * temp2);

			double af, as, ah, bf, bs, bh, cf, cs, ch, df, ds, dh, ef, es, eh;
			double d1, d2, d3, d4, d5, dtotal;
			double t1, t2, t3, t4, t5;

			if (dd != 0) {
				double locusRotate = atan2(fstart-fend,sstart-send) - (pi/2.0);
				bf = fend + bsl2*sin(locusRotate) + bfl2*cos(locusRotate);
				bs = send + bsl2*cos(locusRotate) - bfl2*sin(locusRotate);
				cf = fend + csl2*sin(locusRotate) + cfl2*cos(locusRotate);
				cs = send + csl2*cos(locusRotate) - cfl2*sin(locusRotate);
				df = fstart + dsl2*sin(locusRotate) + dfl2*cos(locusRotate);
				ds = sstart + dsl2*cos(locusRotate) - dfl2*sin(locusRotate);
				ef = fstart + esl2*sin(locusRotate) + efl2*cos(locusRotate);
				es = sstart + esl2*cos(locusRotate) - efl2*sin(locusRotate);
				af = (ef + bf)/2.0;
				as = (es + bs)/2.0;
			}
			else {
				bf = cf = df = ef = af = (fstart + fend)/2.0;
				bs = cs = ds = es = as = (sstart + send)/2.0;
			}

			if (hdb != 0) {
				bh = hb + bhl2;
				ch = hb + chl2 - hdb;
				dh = hb + dhl2 - hdb;
				eh = hb + ehl2;
				ah = (eh + bh)/2.0;
			}
			else {
				bh = ch = dh = eh = ah = hb;
			}

			d1 = sqrt((af-bf)*(af-bf) + (as-bs)*(as-bs) + (ah-bh)*(ah-bh));
			d2 = sqrt((bf-cf)*(bf-cf) + (bs-cs)*(bs-cs) + (bh-ch)*(bh-ch));
			d3 = sqrt((cf-df)*(cf-df) + (cs-ds)*(cs-ds) + (ch-dh)*(ch-dh));
			d4 = sqrt((df-ef)*(df-ef) + (ds-es)*(ds-es) + (dh-eh)*(dh-eh));
			d5 = sqrt((ef-af)*(ef-af) + (es-as)*(es-as) + (eh-ah)*(eh-ah));
			dtotal = d1 + d2 + d3 + d4 + d5;

			if (dtotal != 0) {
				t1 = d1/dtotal * 2 * PG;
				t2 = d2/dtotal * 2 * PG;
				t3 = d3/dtotal * 2 * PG;
				t4 = d4/dtotal * 2 * PG;
				t5 = d5/dtotal * 2 * PG;
			}
			else {
				t1 = t2 = t3 = t4 = 0;
				t5 = 2 * PG;
			}

			if (step_ < t1) {
				fstep = (af - bf)/t1;
				sstep = (as - bs)/t1;
				hstep = (ah - bh)/t1;
				f     = af - step_*fstep;
				s     = as - step_*sstep;
				h     = ah - step_*hstep;
			} 
			else if (step_ < t1 + t2) {
				fstep = (bf - cf)/t2;
				sstep = (bs - cs)/t2;
				hstep = (bh - ch)/t2;
				f     = bf - (step_-t1)*fstep;
				s     = bs - (step_-t1)*sstep;
				h     = bh - (step_-t1)*hstep;
			} 
			else if (step_ < t1 + t2 + t3) {
				fstep = (cf - df)/t3;
				sstep = (cs - ds)/t3;
				hstep = (ch - dh)/t3;
				f     = cf - (step_-t1-t2)*fstep;
				s     = cs - (step_-t1-t2)*sstep;
				h     = ch - (step_-t1-t2)*hstep;
			} 
			else if (step_ < t1 + t2 + t3 + t4) {
				fstep = (df - ef)/t4;
				sstep = (ds - es)/t4;
				hstep = (dh - eh)/t4;
				f     = df - (step_-t1-t2-t3)*fstep;
				s     = ds - (step_-t1-t2-t3)*sstep;
				h     = dh - (step_-t1-t2-t3)*hstep;
			} 
			else {
				fstep = (ef - af)/t5;
				sstep = (es - as)/t5;
				hstep = (eh - ah)/t5;
				f     = ef - (step_-t1-t2-t3-t4)*fstep;
				s     = es - (step_-t1-t2-t3-t4)*sstep;
				h     = eh - (step_-t1-t2-t3-t4)*hstep;
			}
}


void OffsetWalk::getBRCornerPoint(int pointID, double &f, double &s, double &h) {
	double temp1, temp2, dd;
	double fstart, fend, sstart, send;

	/* work out start and end of paw in f-s plane */
	fstart = fc + tc * sin5 + bfo;
	fend   = -fc - tc * sin5 + bfo;
	sstart = -sc + tc * cos5 + bso;
	send   = sc - tc * cos5 + bso;

	/* work out locus length */
	temp1 = fstart - fend;
	temp2 = sstart - send;
	dd = sqrt(temp1 * temp1 + temp2 * temp2);

			double af, as, ah, bf, bs, bh, cf, cs, ch, df, ds, dh, ef, es, eh;

			if (dd != 0) {
				double locusRotate = atan2(fstart-fend,sstart-send) - (pi/2.0);
				bf = fend + bsl2*sin(locusRotate) + bfl2*cos(locusRotate);
				bs = send + bsl2*cos(locusRotate) - bfl2*sin(locusRotate);
				cf = fend + csl2*sin(locusRotate) + cfl2*cos(locusRotate);
				cs = send + csl2*cos(locusRotate) - cfl2*sin(locusRotate);
				df = fstart + dsl2*sin(locusRotate) + dfl2*cos(locusRotate);
				ds = sstart + dsl2*cos(locusRotate) - dfl2*sin(locusRotate);
				ef = fstart + esl2*sin(locusRotate) + efl2*cos(locusRotate);
				es = sstart + esl2*cos(locusRotate) - efl2*sin(locusRotate);
				af = (ef + bf)/2.0;
				as = (es + bs)/2.0;
			}
			else {
				bf = cf = df = ef = af = (fstart + fend)/2.0;
				bs = cs = ds = es = as = (sstart + send)/2.0;
			}

			if (hdb != 0) {
				bh = hb + bhl2;
				ch = hb + chl2 - hdb;
				dh = hb + dhl2 - hdb;
				eh = hb + ehl2;
				ah = (eh + bh)/2.0;
			}
			else {
				bh = ch = dh = eh = ah = hb;
			}

	switch (pointID) {
		case 1:
			f = bf;
			s = bs;
			h = bh;
			break;
		case 2:
			f = cf;
			s = cs;
			h = ch;
			break;
		case 3:
			f = df;
			s = ds;
			h = dh;
			break;
		case 4:
			f = ef;
			s = es;
			h = eh;
			break;
		default:
			f = s = h = 0;
			break;
	}
}


void OffsetWalk::readLocusOffets(const char *filename)
{
	ifstream in(filename);

	if (in) {
		in >> bfl1 >> bsl1 >> bhl1 
			>> cfl1 >> csl1 >> chl1 
			>> dfl1 >> dsl1 >> dhl1 
			>> efl1 >> esl1 >> ehl1 
			>> bfl2 >> bsl2 >> bhl2 
			>> cfl2 >> csl2 >> chl2 
			>> dfl2 >> dsl2 >> dhl2 
			>> efl2 >> esl2 >> ehl2;
	}
	else {
		/* unable to open file, default to 0 offsets(uniform
		 * speed rectangular locus
		 */
		cout << "Unable to read locus offsets, reverting to 0 offsets." << endl;
		bfl1 = bsl1 = bhl1 =
			cfl1 = csl1 = chl1 =
			dfl1 = dsl1 = dhl1 =
			efl1 = esl1 = ehl1 =
			bfl2 = bsl2 = bhl2 =
			cfl2 = csl2 = chl2 =
			dfl2 = dsl2 = dhl2 =
			efl2 = esl2 = ehl2 = 0;
	}
}

void OffsetWalk::setOffsetParams(const OffsetParams *nextOP) {
	bfl1 = nextOP->bfl1;
	bsl1 = nextOP->bsl1;
	bhl1 = nextOP->bhl1;
	cfl1 = nextOP->cfl1;
	csl1 = nextOP->csl1;
	chl1 = nextOP->chl1;
	dfl1 = nextOP->dfl1;
	dsl1 = nextOP->dsl1;
	dhl1 = nextOP->dhl1;
	efl1 = nextOP->efl1;
	esl1 = nextOP->esl1;
	ehl1 = nextOP->ehl1;
	bfl2 = nextOP->bfl2;
	bsl2 = nextOP->bsl2;
	bhl2 = nextOP->bhl2;
	cfl2 = nextOP->cfl2;
	csl2 = nextOP->csl2;
	chl2 = nextOP->chl2;
	dfl2 = nextOP->dfl2;
	dsl2 = nextOP->dsl2;
	dhl2 = nextOP->dhl2;
	efl2 = nextOP->efl2;
	esl2 = nextOP->esl2;
	ehl2 = nextOP->ehl2;
}

