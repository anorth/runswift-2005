/*
   Copyright 2005 The University of New South Wales (UNSW) and National  
   ICT Australia (NICTA).

   This file is part of the 2005 team rUNSWift RoboCup entry.  You may  
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
   should be included in that publication.

   This rUNSWift source is distributed in the hope that it will be useful,  
   but WITHOUT ANY WARRANTY; without even the implied warranty of  
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  
   General Public License for more details.

   You should have received a copy of the GNU General Public License along  
   with this source code; if not, write to the Free Software Foundation,  
   Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 */


#include "../../robot/share/minimalMatrix.h"

// very loosely based on the Numerical Recipies in C Simplex minimizer

template <std::size_t Cols>
class simplexMin {

	typedef MMatrix<double, 1, Cols> vec_type;
	typedef double (*evalFuncType)(vec_type& pt, void* additional);
	
	static const double ftol = 1e-3;
	static const double xtol = 0.1;
	void *addInfo;
	evalFuncType eval;

	vec_type p[Cols+1];
	vec_type psum;
	double y[Cols+1];


	void swapPt(int i, int j) {
		double spareVal = y[i];
		y[i] = y[j];
		y[j] = spareVal;
		
		vec_type sparePt = p[i];
		p[i] = p[j];
		p[j] = sparePt;
	}

	void getPSum() {
		psum.reset();
		for (unsigned int i=0; i<=Cols; i++) {
			psum += p[i];
		}
	}

	double amotry(int ihi, double fac) {
		double fac1, fac2, ytry;
		
		fac1 = (1.0-fac)/Cols;
		fac2 = fac1-fac;
		
		vec_type x = psum;
		x *= fac1;
		
		x.addMult(-fac2, p[ihi]);
		
		ytry = eval(x,addInfo);
		
		if (ytry < y[ihi]) {
			y[ihi] = ytry;
			psum -= p[ihi];
			psum += x;
			p[ihi] = x;
		}
		
		return ytry;
	}

	public:

	vec_type min(evalFuncType ef, vec_type initial, void *additional) {
	
		const int mpts = Cols+1;
		
		//const double nonzdelt = 0.05;
		//const double zdelt = 0.00025;
		
		eval = ef;
		addInfo = additional;
		psum.reset();
		
		p[0] = initial;
		y[0] = eval(initial, addInfo);
		psum += initial;
		
		for (unsigned int k=0; k<Cols; k++) {
			vec_type pt = initial;
/*
			if (pt(0,k) != 0) {
				pt(0,k) = (1+nonzdelt)*pt(0,k);
			} else {
				pt(0,k) = zdelt;
			}
*/
			pt(0,k) += 2;
			p[k+1] = pt;
			y[k+1] = eval(pt,addInfo);
			psum += pt;
		}
		
		for (int iter = 0; iter < 100; iter++) {
			int ilo = 0;
			int ihi, inhi;
			double ytry;
			
			// find the worst, second-worst, and best points (ihi, inhi, ilo)
			
			if (y[0]>y[1]) {
				inhi = 1;
				ihi = 0;
			} else {
				inhi = 0;
				ihi = 1;
			}
			
			for (int i=0; i<mpts; i++) {
				if (y[i] <= y[ilo])
					ilo = i;
				if (y[i] > y[ihi]) {
					inhi = ihi;
					ihi = i;
				} else if (y[i] > y[inhi] && i != ihi) {
					inhi = i;
				}
			}
			
			// check to see if we're done
			
			bool xbreakout = true;
			bool ybreakout = true;
			for (unsigned int i=0; (xbreakout || ybreakout) && i<=Cols; i++) {
				for (unsigned int j=0; xbreakout && j<Cols; j++) {
					double diff = fabs(p[ilo](0,j) - p[i](0,j));
					if (diff > xtol) {
						xbreakout = false;
					}
				}
				if (fabs(y[ilo] - y[i]) > ftol) {
					ybreakout = false;
				}
			}
			
			if (xbreakout || ybreakout) {
				if (ilo != 0)
					swapPt(0, ilo);
				return p[0];
			}
/*			
			double rtol = 2.0*fabs(y[ihi]-y[ilo])/(fabs(y[ihi])+fabs(y[ilo]));
			if (rtol < ftol) {
				if (ilo != 0)
					swapPt(0, ilo);
				return p[0];
			}
*/
			// do the iteration
			
			ytry = amotry(ihi, -1.0);
			
			if (ytry <= y[ilo]) {
				// gives a great result - try extrapolating further in this direction
				ytry = amotry(ihi, 2.0);
			} else if (ytry >= y[inhi]) {
				// the reflected point is worse than the second-worst point.  Look for an intermediate lower point, i.e., do a 1D contraction.
				double ysave = y[ihi];
				ytry = amotry(ihi, 0.5);
				if (ytry >= ysave) {
					// can't seem to get rid of this high point.  Contract around lowest point.
					for (int i=0; i<mpts; i++) {
						if (i != ilo) {
							vec_type x = p[i];
							x *= 0.5;
							x.addMult(0.5, p[ilo]);
							p[i] = x;
							y[i] = eval(x,addInfo);
						}
					}
					getPSum();
				}
			}
		}
		
		return p[0];
	}
};

/*
template <std::size_t Cols>
double testFunc(const MMatrix<double, 1, Cols> &pt) {
	cout << "eval at: ";
	pt.printOut();
	cout << " is: ";
	double result = pt(0,0)*pt(0,0) + pt(0,1)*pt(0,1) + pt(0,2)*pt(0,2);
	cout << result << "\n";
	return result;
}

int main() {
	MMatrix<double, 1, 3> initial;
	simplexMin<3> simp;
	
	initial.reset();
	initial(0,0)=5; initial(0,1)=4; initial(0,2)=3;
	
	initial = simp.min(testFunc, initial);
	cout << "result: " << endl;
	initial.printOut();
}
*/
