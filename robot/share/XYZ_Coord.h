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
 * $Id: XYZ_Coord.h 4364 2004-09-22 07:04:12Z kcph007 $
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 *
**/



#ifndef XYZ_Coord_h
#define XYZ_Coord_h

#include <cmath>
#include <iostream>

class XYZ_Coord{
	public:
	double x,y,z;

	XYZ_Coord() {
		x = 0;
		y = 0;
		z = 0;
	}

	XYZ_Coord(double nx, double ny, double nz) {
		x = nx;
		y = ny;
		z = nz;
	}

	inline void rotateXY(double theta) {
		const double lx = x;
		const double ly = y;
	
		const double sint = sin(theta);
		const double cost = cos(theta);
	
		y = ly * cost - lx * sint;
		x = ly * sint + lx * cost;
	}
	
	inline void rotateYZ(double theta) {
		const double ly = y;
		const double lz = z;
	
		const double sint = sin(theta);
		const double cost = cos(theta);
	
		y = ly * cost - lz * sint;
		z = ly * sint + lz * cost;
	}
	
	inline void add(const XYZ_Coord &opt) {
		x += opt.x;
		y += opt.y;
		z += opt.z;
	}

	inline void sub(const XYZ_Coord &opt) {
		x -= opt.x;
		y -= opt.y;
		z -= opt.z;
	}

	inline void addMult(const XYZ_Coord &opt, double mult) {
		x += opt.x*mult;
		y += opt.y*mult;
		z += opt.z*mult;
	}

	inline void normalize() {
		double dist = sqrt(x*x + y*y + z*z);
		
		if (dist != 0) {		
			x = x/dist;
			y = y/dist;
			z = z/dist;
		}
	}

};

static inline std::ostream &operator<<(std::ostream &out , const XYZ_Coord &a){
	out << '[' << a.x << ", " << a.y << ", " << a.z << ']';
	return out;
}

static inline XYZ_Coord rotateCopyXY(const XYZ_Coord &xyz, double theta) {
	XYZ_Coord result = xyz;
	result.rotateXY(theta);
	return result;
}

static inline XYZ_Coord addCopy(const XYZ_Coord &A, const XYZ_Coord &B) {
	XYZ_Coord result = A;
	result.add(B);
	return result;
}

static inline XYZ_Coord subCopy(const XYZ_Coord &A, const XYZ_Coord &B) {
	XYZ_Coord result = A;
	result.sub(B);
	return result;
}

#endif
