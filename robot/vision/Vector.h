/*

   Copyright 2003 The University of New South Wales (UNSW) and National  
   ICT Australia (NICTA).

   This file is part of the 2003 team rUNSWift RoboCup entry.  You may  
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
 * $Id: Vector.h 6092 2005-05-20 01:03:14Z alexn $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 * 
 **/

#ifndef VECTOR_H
#define VECTOR_H

#include "../share/Common.h"
#include <iostream>
//#include "Wireless.h"

#define vCART   0        /* cartesian */
#define vPOLAR  1        /* polar */

using std::cout;
using std::endl;

class Vector {
	public:
		double x, y; //cartesian coordinates of the vector
		double d, theta; //polar coordinates of the vector
		// all angles are in degrees

		double head;

		double angleNorm; //angle it can get to before wrapping around to 0 again


		/**
		 * A unit vector
		 */
		Vector();

		/**
         * @vtype is either cartesion (vCART) or polar (vPOLAR).  It sets the
         * type fo values sent in the next two parameters.  @value1 x or d
		 * @value2 y or theta
         * @normaliser, usually a vector is normailesd between PI and -PI.  THe
         * other valid setting is 2*PI [0..2PI]
		 */
		Vector(int vtype,
				double value1,
				double value2,
				double normaliser = HALF_CIRCLE);
		Vector(const Vector &v);
		~Vector() {
		}

		/* these methods returns states information */
		bool setVector(int vtype, double value1, double value2);
		void setAngleNorm(double degs);
		double getX() const;
		double getY() const;
		double getR() const;
		double getTheta() const;
		double getHead() const;

		double getHackedTheta();
		double getHackedHead();
		/**
		 * Returns the difference in the theta values between the two vectors
		 */
		double diff_theta(const Vector &v) const;

		/* these methods modify states */
		/* Should have he same origin object */
		inline void Vector::add(const Vector &v) {
			x += v.x;
			y += v.y;
			calcR();
			calcTheta();
		}

		inline void Vector::sub(const Vector &v) {
			x -= v.x;
			y -= v.y;
			calcR();
			calcTheta();
		}

		void norm();
		void scale(double scale);
		void rotate(double epsilon);    // rad

		void reset();

		/* print utility */
		void dump() const;

		/* operator utility */
		void operator =(const Vector &v) {
			x = v.x;
			y = v.y;
			d = v.d;
			theta = v.theta;
			head = v.head;
		}
		Vector operator +(const Vector &v) {
			Vector res(*this);
			res.add(v);
			return res;
		}
		Vector operator -(const Vector &v) {
			Vector res(*this);
			res.sub(v);
			return res;
		}

	protected:

		inline double Vector::principle(double phi) const {
			int n = 3;
			if (phi > angleNorm) {
				while (phi > angleNorm) {
					phi -= FULL_CIRCLE;
					if (--n < 0) {
						cout << "problem - " << phi << "," << angleNorm << endl;
						break;
					}
				}
			}
			else {

            //vel(0, 0) = bx - lastx;
            //vel(1, 0) = by - lasty;
				while (phi <= angleNorm - FULL_CIRCLE) {
					phi += FULL_CIRCLE;
					if (--n < 0) {
						cout << "problem - " << phi << "," << angleNorm << endl;
						break;
					}
				}
			}
			return phi;
		}

		void calcX() {
			x = d * cos(DEG2RAD(theta));
		}
		void calcY() {
			y = d * sin(DEG2RAD(theta));
		}
		void calcR() {
			d = sqrt(SQUARE(x) + SQUARE(y));
		}
		void calcHead() {
			head = principle(theta - HALF_CIRCLE / 2);
		}
		void calcTheta() {
			theta = (x == 0 && y == 0) ? 0 : RAD2DEG(atan2(y, x));
			calcHead();
		}
};

// the constructors are declared below this function because this function is
// inline and used by one of the constructors
inline bool Vector::setVector(int vtype, double value1, double value2) {
	if (vtype == vCART) {
		x = value1;
		y = value2;
		calcR();
		calcTheta();
		//calcHead ();
	}
	else if (vtype == vPOLAR) {
		d = value1;
		theta = value2;
		calcHead();
		calcX();
		calcY();
	}
	else {
		return false;
	}
	return true;
}

inline Vector::Vector() {
	/* unit vector */
	x = 1;
	y = 0;
	d = 1;
	theta = 0;
	head = -HALF_CIRCLE / 2;
	angleNorm = HALF_CIRCLE;
}

inline Vector::Vector(int vtype, double value1, double value2, double value3) {
	angleNorm = value3;
	setVector(vtype, value1, value2);
}

inline Vector::Vector(const Vector &v) {
	x = v.x;
	y = v.y;
	d = v.d;
	theta = v.theta;
	head = v.head;
	angleNorm = v.angleNorm;
}

inline void Vector::reset() {
	x = y = d = theta = 0;
	head = HALF_CIRCLE / 2;
}
inline void Vector::setAngleNorm(double degs) {
	angleNorm = degs;
	theta = principle(theta);
	calcHead();
}
inline double Vector::getX() const {
	return x;
}
inline double Vector::getY() const {
	return y;
}
inline double Vector::getR() const {
	return d;
}
inline double Vector::getTheta() const {
	return theta;
}

inline double Vector::getHackedTheta() {
	return theta;
}

inline double Vector::getHackedHead() {
	return head;
}

inline double Vector::getHead() const {
	return head;
}
inline double Vector::diff_theta(const Vector &v) const {
	return principle(theta - v.theta);
}

inline void Vector::norm() {
	if (d == 0) {
		return;
	}

	x /= d;
	y /= d;
	calcR();
	calcTheta();
}

inline void Vector::scale(double scaleFactor) {
	/* theta unchanged */
	x *= scaleFactor;
	y *= scaleFactor;
	calcR();
	calcTheta();
}
// ivy
inline void Vector::rotate(double epsilon) {
	/* d unchanged */
	theta = principle(theta + epsilon);
	calcHead();
	calcX();
	calcY();
}

/* private utility methods */
inline void Vector::dump() const {
	double angle;
	angle = theta;
	cout << "x: " << x << " y: " << y << " d: " << d << " t: " << angle << endl;
}

#endif
