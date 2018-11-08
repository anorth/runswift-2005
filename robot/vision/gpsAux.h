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

#ifndef _gpsAUX_h
#define _gpsAUX_h

/*
 * Last modification background information
 * $Id: gpsAux.h 1953 2003-08-21 03:51:39Z eileenm $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 *
 * Auxillery structures for localisation
 *
 **/

#undef MAVERICK_WM_STUFF

//used to store a cut down copy of VisualObject
//used as an interface between visualobject and what we need
//used because it looks like visual object is going to change soon
struct SlimVisOb {
	int vobType;     //number that represents what type of visual object it is
	//see typedef VobEnum in VisualCortex.h
	double d;        //distance to the object
	double var;      //variance on that distance
	double h;        //heading of that object      in degrees forward 0
	double angleVar; //variance on that heading
	double cf;       // confidence factor, mainly used for ball
};

struct WMObj {
	int vobType;     //number that represents what type of visual object it is
	//see typedef VobEnum in VisualCortex.h
	Vector pos;      //x and y coords in world model of where object is
	double posVar;   //variance of location.. summarises the covariance matrix

	//these only applicable if used to store your own location
	double h;        //heading of that object (if applicable)
	double hVar;     //variance on that heading

	// only applicable for friendly robots, represents validity
	int counter;

	unsigned int behavioursVal;
	//only maverick should use this.. get rid of it when you can
#ifdef MAVERICK_WM_STUFF
	unsigned int ballDist : 29;
	unsigned int amThirdPlayer : 1;
	unsigned int playerType : 2;
#endif

	WMObj() {
		vobType = 1;
		pos.setVector(vCART, 0.0, 0.0);
		posVar = get95CF(200);
		h = 0;
		hVar = get95CF(HALF_CIRCLE);
		counter = 0;
		behavioursVal = 0;
#ifdef MAVERICK_WM_STUFF
		ballDist = 0;
		playerType = 0;
		amThirdPlayer = 0;
#endif
	}

	WMObj & operator =(WMObj other) {
		this->pos.setVector(vCART, other.pos.x, other.pos.y);
		this->posVar = other.posVar;
		this->h = other.h;
		this->hVar = other.hVar;
		this->counter = other.counter;
		this->behavioursVal = other.behavioursVal;
#ifdef MAVERICK_WM_STUFF
		this->ballDist = other.ballDist;
		this->playerType = other.playerType;
		this->amThirdPlayer = other.amThirdPlayer;
#endif
		return *this;
	}



	//given the covariance of the objects position (and heading if applicable)
	//generate the variances posVar (and hVar) that summarises the covariance
	void updateVar(const MMatrix3 *cov) {
		//make the variance on the position
		//the longest axis of the ellipse
		//ie solve for eigen in
		//(varx-eigen)(vary-eigen)-(varxy)^2 = 0
		//

		double b = -((*cov) (0, 0) + (*cov) (1, 1));       //-(varx+vary)
		double c = (*cov) (0, 0) * (*cov) (1, 1)
			- (*cov) (0, 1) * (*cov) (0, 1); //varx*vary-varxy^2
		//Raymond: I thought we fixed this ages ago ...
		if (b * b - 4 * c < 1E-5) {
			posVar = (-b + 0) / 2;
			if (b * b - 4 * c < -1E-5) {
				cout << __func__ << "WARNING: WMObj MM3 b*b-4*c < 0 : " << (b*b-4*c)
					<< endl;
			}
		}
		else {
			posVar = (-b + sqrt(b * b - 4 * c)) / 2;
		}
		hVar = (*cov) (2, 2);
	}

	void updateVar(const MMatrix4 *cov) {
		//make the variance on the position
		//the longest axis of the ellipse
		//ie solve for eigen in
		//(varx-eigen)(vary-eigen)-(varxy)^2 = 0
		//

		double b = -((*cov) (0, 0) + (*cov) (1, 1));       //-(varx+vary)
		double c = (*cov) (0, 0) * (*cov) (1, 1)
			- (*cov) (0, 1) * (*cov) (0, 1); //varx*vary-varxy^2
		if (b * b - 4 * c < 1E-5) {
			posVar = (-b + 0) / 2;
			if (b * b - 4 * c < -1E-5) {
				cout << __func__ << "WARNING: WMObj MM4 b*b-4*c < 0 : " << (b*b-4*c) << endl;
			}
		}
		else {
			posVar = (-b + sqrt(b * b - 4 * c)) / 2;
		}
		/*if (cov->RowNo()>2) {
		  hVar = (*cov)(2,2);
		  }*/
	}

	double getHackedH() const {
		double lala = h;
		while (lala >= 180) {
			lala -= 360;
		}
		while (lala < -180) {
			lala += 360;
		}
		return lala;
	}


	int serialize(unsigned char *add, int base) {
		// note - only adds stuff that is important regarding to wireless
		int ret = base;
		float temp = (float) pos.x;
		memcpy(add + ret, &temp, sizeof(float));
		ret += (int)sizeof(float);
		temp = (float) pos.y;
		memcpy(add + ret, &temp, sizeof(float));
		ret += (int)sizeof(float);

		temp = (float) h;
		memcpy(add + ret, &temp, sizeof(float));
		ret += (int)sizeof(float);
		temp = (float) posVar;
		memcpy(add + ret, &temp, sizeof(float));
		ret += (int)sizeof(float);
		temp = (float) hVar;
		memcpy(add + ret, &temp, sizeof(float));
		ret += (int)sizeof(float);

		memcpy(add + ret, &behavioursVal, sizeof(unsigned int));
		ret += (int)sizeof(unsigned int);
#ifdef MAVERICK_WM_STUFF
		unsigned int temp2 = (unsigned int) ballDist;
		memcpy(add + ret, &temp2, sizeof(unsigned int));
		ret += (int)sizeof(unsigned int);
		temp2 = (unsigned int) playerType;
		memcpy(add + ret, &temp2, sizeof(unsigned int));
		ret += (int)sizeof(unsigned int);
		temp2 = (unsigned int) amThirdPlayer;
		memcpy(add + ret, &temp2, sizeof(unsigned int));
		ret += (int)sizeof(unsigned int);
#endif
		return ret;
	}

	int unserialize(unsigned char *add, int base) {
		int ret = base;
		float temp;

		double x, y;
		memcpy(&temp, add + ret, sizeof(float));
		ret += (int)sizeof(float);
		x = (double) temp;
		memcpy(&temp, add + ret, sizeof(float));
		ret += (int)sizeof(float);
		y = (double) temp;
		pos.setVector(vCART, x, y);

		memcpy(&temp, add + ret, sizeof(float));
		ret += (int)sizeof(float);
		h = (double) temp;
		memcpy(&temp, add + ret, sizeof(float));
		ret += (int)sizeof(float);
		posVar = (double) temp;
		memcpy(&temp, add + ret, sizeof(float));
		ret += (int)sizeof(float);
		hVar = (double) temp;

		memcpy(&behavioursVal, add + ret, sizeof(unsigned int));
		ret += (int)sizeof(unsigned int);
#ifdef MAVERICK_WM_STUFF
		unsigned int temp2;
		memcpy(&temp2, add + ret, sizeof(unsigned int));
		ret += (int)sizeof(unsigned int);
		ballDist = temp2;
		memcpy(&temp2, add + ret, sizeof(unsigned int));
		ret += (int)sizeof(unsigned int);
		playerType = temp2;
		memcpy(&temp2, add + ret, sizeof(unsigned int));
		ret += (int)sizeof(unsigned int);
		amThirdPlayer = temp2;
#endif

		return ret;
	}
}; 

struct SingleGaussian {
    WMObj mean;
    MMatrix3 cov;
    double weight;
    SingleGaussian& operator = (SingleGaussian other) {
        mean = other.mean;
        cov = other.cov;
        weight = other.weight;
        
        return *this;   
    }
    void printOut()
    {
        cout << "Gaussian has weight " << weight << "\n";
        cout << "Mean: x " << mean.pos.x << "   y " << mean.pos.y;
        cout << "   h " << mean.h << "   Covariance:\n";
        cov.printOut();        
    }
};
#endif // _gpsAUX_h
