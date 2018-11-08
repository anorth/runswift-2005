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
 * $Id: Odometer.cc 4362 2004-09-22 06:54:12Z kcph007 $
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 *
**/



#include "Odometer.h"
#include <list>
#include "../share/Common.h"

#include <fstream>
#include <stdio.h>

struct fsh {
	double f, s, h;
};

struct xyz {
	double x, y, z;
};

namespace Odometer {
	std::list<xyz> flxyz, frxyz, rlxyz, rrxyz;
	std::list<fsh> flfsh, frfsh, rlfsh, rrfsh;

	void xyz2fsh(double x,
			double y,
			double z,
			double theta4,
			double &f,
			double &s,
			double &h);
	void inverseFront(double theta1,
			double theta2,
			double theta3,
			double &x,
			double &y,
			double &z);
	void inverseBack(double theta1,
			double theta2,
			double theta3,
			double &x,
			double &y,
			double &z);
}

void Odometer::storeData(long jfl1,
		long jfl2,
		long jfl3,
		long jfr1,
		long jfr2,
		long jfr3,
		long jbl1,
		long jbl2,
		long jbl3,
		long jbr1,
		long jbr2,
		long jbr3) {
	xyz entry;

	inverseFront(MICRO2RAD(jfl1),
			MICRO2RAD(jfl2),
			MICRO2RAD(jfl3),
			entry.x,
			entry.y,
			entry.z);
	flxyz.push_back(entry);

	inverseFront(MICRO2RAD(jfr1),
			MICRO2RAD(jfr2),
			MICRO2RAD(jfr3),
			entry.x,
			entry.y,
			entry.z);
	frxyz.push_back(entry);

	inverseBack(MICRO2RAD(jbl1),
			MICRO2RAD(jbl2),
			MICRO2RAD(jbl3),
			entry.x,
			entry.y,
			entry.z);
	rlxyz.push_back(entry);

	inverseBack(MICRO2RAD(jbr1),
			MICRO2RAD(jbr2),
			MICRO2RAD(jbr3),
			entry.x,
			entry.y,
			entry.z);
	rrxyz.push_back(entry);
}

void Odometer::inverse(const OdometerData *data, bool store) {
	if (store) {
		double theta4 = asin((data->hb - data->hf) / lsh);
		fsh entry;

		for (std::list<xyz>::iterator i = flxyz.begin(); i != flxyz.end(); ++i) {
			xyz2fsh((*i).x, (*i).y, (*i).z, theta4, entry.f, entry.s, entry.h);
			flfsh.push_back(entry);
		}
		for (std::list<xyz>::iterator i = frxyz.begin(); i != frxyz.end(); ++i) {
			xyz2fsh((*i).x, (*i).y, (*i).z, theta4, entry.f, entry.s, entry.h);
			frfsh.push_back(entry);
		}
		for (std::list<xyz>::iterator i = rlxyz.begin(); i != rlxyz.end(); ++i) {
			xyz2fsh((*i).x, (*i).y, (*i).z, theta4, entry.f, entry.s, entry.h);
			rlfsh.push_back(entry);
		}
		for (std::list<xyz>::iterator i = rrxyz.begin(); i != rrxyz.end(); ++i) {
			xyz2fsh((*i).x, (*i).y, (*i).z, theta4, entry.f, entry.s, entry.h);
			rrfsh.push_back(entry);
		}

		static int count = 0;

		/* Store data to be plotted with Mathematica. */
		char name[256];
		sprintf(name, "data.%d", count);
		std::fstream ofile(name);

		if (ofile) {
			for (std::list<fsh>::iterator i = flfsh.begin(); i != flfsh.end(); ++i)
				ofile << "{-" << (*i).f << "," << (*i).s << ",-" << (*i).h << "},";
			ofile << std::endl;
			for (std::list<fsh>::iterator i = frfsh.begin(); i != frfsh.end(); ++i)
				ofile << "{" << (*i).f << "," << (*i).s << ",-" << (*i).h << "},";
			ofile << std::endl;
			for (std::list<fsh>::iterator i = rlfsh.begin(); i != rlfsh.end(); ++i)
				ofile << "{-" << (*i).f << "," << (*i).s << ",-" << (*i).h << "},";
			ofile << std::endl;
			for (std::list<fsh>::iterator i = rrfsh.begin(); i != rrfsh.end(); ++i)
				ofile << "{" << (*i).f << "," << (*i).s << ",-" << (*i).h << "},";
			ofile << std::endl;
		}

		ofile.close();
		++count;
	}

	flfsh.clear();
	frfsh.clear();
	rlfsh.clear();
	rrfsh.clear();

	flxyz.clear();
	frxyz.clear();
	rlxyz.clear();
	rrxyz.clear();
}

void Odometer::xyz2fsh(double x,
		double y,
		double z,
		double theta4,
		double &f,
		double &s,
		double &h) {
	f = x * cos(theta4) - y * sin(theta4);
	s = z;
	h = y * cos(theta4) + x * sin(theta4);
}

void Odometer::inverseFront(double theta1,
		double theta2,
		double theta3,
		double &x,
		double &y,
		double &z) {
	double u = 2 * l3 *sin(theta3 / 2) * sin(theta3 / 2) + l2 *sin(theta3);
	double v = l1 + l3 *sin(theta3) + l2 *cos(theta3);    
	x = v * cos(theta1) * sin(theta2) + u * cos(theta2);
	y = v * cos(theta1) * cos(theta2) - u * sin(theta2);
	z = v * sin(theta1);
}

void Odometer::inverseBack(double theta1,
		double theta2,
		double theta3,
		double &x,
		double &y,
		double &z) {
	double u = 2 * l3 *sin(theta3 / 2) * sin(theta3 / 2) + l4 *sin(theta3);
	double v = l1 + l3 *sin(theta3) + l4 *cos(theta3);
	x = -v * cos(theta1) * sin(theta2) - u * cos(theta2);
	y = v * cos(theta1) * cos(theta2) - u * sin(theta2);
	z = v * sin(theta1);
}

