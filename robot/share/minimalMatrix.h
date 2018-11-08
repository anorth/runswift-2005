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

#ifndef MINIMAL_MATRIX_H
#define MINIMAL_MATRIX_H

#include <cstdlib>
#include <math.h>
#include <iostream>

//#define MM_DEBUG_OUT

using namespace std;

static const double MM_LOW_VALUE = 1E-9;

template <class T, std::size_t Rows, std::size_t Cols>
class MMatrix {
	T myData[Rows][Cols];
	public:

	typedef T value_type;
	typedef MMatrix<T, Rows, Cols> this_type;

    // can't inline the constructors (don't know why)
    MMatrix(const this_type &prev);
	MMatrix(T val);

	MMatrix() { }

	inline void reset() {
		for (size_t row = 0 ; row < Rows ; row++) {
			for (size_t col = 0 ; col < Cols ; col++) {
				myData[row][col] = (T) 0;
			}
		}
	}
    
    inline bool equals(const this_type &other) {
		for (size_t i = 0; i < Rows; i++) {
			for (size_t j = 0; j < Cols; j++) {
				if (myData[i][j] != other(i, j)) return false;
			}
		}
        
        return true;
    }

//    inline __attribute__((always_inline))
    value_type & operator()(std::size_t i, std::size_t j) {
#ifdef MM_DEBUG_OUT
		if ((i >= Rows) || (j >= Cols)) {
			std::cout << "MMatrix size violation(ref) <" << Rows;
			std::cout << "," << Cols << "> - tried (";
			std::cout << i << "," << j << ")" << std::endl;
			return myData[0][0];
		}
#endif
		return myData[i][j];
	}

//    inline __attribute__((always_inline))
	value_type operator()(std::size_t i, std::size_t j) const {
#ifdef MM_DEBUG_OUT
		if ((i >= Rows) || (j >= Cols)) {
			std::cout << "MMatrix size violation(val) <" << Rows;
			std::cout << "," << Cols << "> - tried (";
			std::cout << i << "," << j << ")" << std::endl;
		}
#endif
		return myData[i][j];
	}

	// assignment
	inline this_type & operator=(const this_type &newVals) {
		for (size_t i = 0; i < Rows; i++) {
			for (size_t j = 0; j < Cols; j++) {
				myData[i][j] = newVals(i, j);
			}
		}

		return *this;
	}

	// MMatrix operations (in place)

	inline this_type & operator+=(const this_type &newVals) {
		for (size_t i = 0; i < Rows; i++) {
			for (size_t j = 0; j < Cols; j++) {
				myData[i][j] += newVals(i, j);
			}
		}

		return *this;
	}

	inline this_type & operator-=(const this_type &newVals) {
		for (size_t i = 0; i < Rows; i++) {
			for (size_t j = 0; j < Cols; j++) {
				myData[i][j] -= newVals(i, j);
			}
		}

		return *this;
	}

	// Matrix operations (not in place)

    //this never normally gets inlined
	template <std::size_t intSize>
#ifndef OFFLINE        
    __attribute__((always_inline))
#endif    
    this_type & isMult(const MMatrix<T, Rows, intSize> &A,
    const MMatrix<T, intSize, Cols> &B) {
        for (size_t i = 0; i < Rows; i++) {
            for (size_t j = 0; j < Cols; j++) {
                myData[i][j] = 0;
            }
        }

        for (size_t i = 0; i < Rows; i++) {
            for (size_t j = 0; j < intSize; j++) {
                for (size_t k = 0; k < Cols; k++) {
                    myData[i][k] += A(i, j) * B(j, k);
                }
            }
        }

        return *this;
    }
    
    //this never normally gets inlined
#ifndef OFFLINE        
    __attribute__((always_inline))
#endif    
    this_type& preMult( const MMatrix<T, Rows, Rows> &M) {
		T temp[Rows][Cols];
		for (size_t r = 0 ; r < Rows ; r++) {
			for (size_t c = 0 ; c < Cols ; c++) {
				temp[r][c] = myData[r][c];
			}
		}

		for (size_t i = 0; i < Rows; i++) {
			for (size_t j = 0; j < Cols; j++) {
				myData[i][j] = 0;
			}
		}

		for (size_t r = 0 ; r < Rows ; r++) {
			for (size_t c = 0 ; c < Cols ; c++) {
				for (size_t i = 0 ; i < Rows ; i++) {
					myData[r][c] += M(r, i) * temp[i][c];
				}
			}
		}

		return *this;
	}

	inline this_type & isTranspose(const MMatrix<T, Cols, Rows> &M) {
		for (size_t i = 0; i < Rows; i++) {
			for (size_t j = 0; j < Cols; j++) {
				myData[i][j] = M(j, i);
			}
		}
		return *this;
	}

	// scalar operations

	inline this_type & operator+=(const value_type val) {
		for (size_t i = 0; i < Rows; i++) {
			for (size_t j = 0; j < Cols; j++) {
				myData[i][j] += val;
			}
		}

		return *this;
	}

	inline this_type & operator-=(const value_type val) {
		for (size_t i = 0; i < Rows; i++) {
			for (size_t j = 0; j < Cols; j++) {
				myData[i][j] -= val;
			}
		}

		return *this;
	}

	inline this_type & operator*=(const value_type val) {
		for (size_t i = 0; i < Rows; i++) {
			for (size_t j = 0; j < Cols; j++) {
				myData[i][j] *= val;
			}
		}

		return *this;
	}

	inline this_type & operator/=(const value_type val) {
		for (size_t i = 0; i < Rows; i++) {
			for (size_t j = 0; j < Cols; j++) {
				myData[i][j] /= val;
			}
		}

		return *this;
	}

	inline this_type & operator^=(int power) {
		this_type temp = *this;

		for (int i = 1 ; i < power ; i++) {
			this->preMult(temp);
		}

		return *this;
	}

	inline this_type & addMult(const value_type k, const this_type &m) {
		for (size_t i = 0; i < Rows; i++) {
			for (size_t j = 0; j < Cols; j++) {
				myData[i][j] += k*m(i,j);
			}
		}

		return *this;
	}

	// partial pivoting method
    int pivot(size_t row);
   
    void isInverse(const this_type &m);

	// calculate the determinant of a matrix
	inline value_type Det() {
		size_t i,j,k;
		T piv,detVal = T(1);

		if (Rows != Cols) {
#ifdef MM_DEBUG_OUT
			cout << "MatInv: Determinant of a non-square matrix" << endl;
#endif // MM_DEBUG_OUT
		}   

		this_type temp(*this);

		for (k=0; k < Rows; k++) {
			int indx = temp.pivot(k);
			if (indx == -1) return 0;
			if (indx != 0) detVal = - detVal;
			detVal = detVal * temp(k,k);
			
			for (i=k+1; i < Rows; i++) {
				piv = temp(i,k) / temp(k,k);

				for (j=k+1; j < Rows; j++)
					temp(i,j) -= piv * temp(k,j);
			}
		}
	
		return detVal;
	}

	inline void const printOut() {
		for (size_t row = 0 ; row < Rows ; row++) {
			cout << "(";
			for (size_t col = 0 ; col < Cols ; col++) {
				cout << "\t" << myData[row][col];
			}
			cout << "\t)" << endl;
		}
	}
	
    inline void const printOut(ostream * output) {
        if (output == NULL) 
            return;
        for (size_t row = 0 ; row < Rows ; row++) {
            *output << "(";
            for (size_t col = 0 ; col < Cols ; col++) {
                *output << "\t" << myData[row][col];
            }
            *output << "\t)" << endl;
        }
    }

	inline this_type& dot(const this_type &newVals) {
		for (size_t row = 0 ; row < Rows ; row++) {
			for (size_t col = 0 ; col < Cols ; col++) {
				myData[row][col] *= newVals[row][col];
			}
		}
                return *this;
	}
        
	// note - casts type as float
	inline int serialize(unsigned char *add, int base) {
		int ret = base;
		for (size_t row = 0 ; row < Rows ; row++) {
			for (size_t col = 0 ; col < Cols ; col++) {
				float temp = (float) myData[row][col];
				memcpy(add + ret, &temp, sizeof(float));
				ret += (int)sizeof(float);
			}
		}

		return ret;
	}

	// note - casts float as type
	inline int unserialize(unsigned char *add, int base) {
		int ret = base;
		for (size_t row = 0 ; row < Rows ; row++) {
			for (size_t col = 0 ; col < Cols ; col++) {
				float temp;
				memcpy(&temp, add + ret, sizeof(float));
				myData[row][col] = (T) temp;
				ret += (int)sizeof(float);
			}
		}

		return ret;
	}

	// serializes assuming matrix is symmetric
	// note - casts type as float
	inline int serializeSym(unsigned char *add, int base) {
		if (Rows != Cols) {
			cout << "Error! : asked to assume non-square matrix is symmetric"
				<< endl;
			cout << "         Performing normal serialize" << endl;
			return serialize(add, base);
		}

		int ret = base;
		for (size_t row = 0 ; row < Rows ; row++) {
			for (size_t col = 0 ; col < Cols ; col++) {
				// just copy top half
				if (col >= row) {
					float temp = (float) myData[row][col];
					memcpy(add + ret, &temp, sizeof(float));
					ret += (int)sizeof(float);
				}
			}
		}

		return ret;
	}

	// unserializes assuming matrix is symmetric
	// note - casts float as type
	inline int unserializeSym(unsigned char *add, int base) {
		if (Rows != Cols) {
			cout << "Error! : asked to assume non-square matrix is symmetric"
				<< endl;
			cout << "         Performing normal unserialize" << endl;
			return unserialize(add, base);
		}

		int ret = base;
		for (size_t row = 0 ; row < Rows ; row++) {
			for (size_t col = 0 ; col < Cols ; col++) {
				if (col >= row) {
					float temp;
					memcpy(&temp, add + ret, sizeof(float));
					myData[row][col] = (T) temp;
					myData[col][row] = (T) temp;
					ret += (int)sizeof(float);
				}
			}
		}

		return ret;
	}
};

typedef MMatrix<double, 2, 1> MVec2;
typedef MMatrix<double, 3, 1> MVec3;
typedef MMatrix<double, 1, 2> MRowVec2;
typedef MMatrix<double, 1, 1> MMatrix1;
typedef MMatrix<double, 2, 2> MMatrix2;
typedef MMatrix<double, 3, 3> MMatrix3;
typedef MMatrix<double, 4, 4> MMatrix4;
typedef MMatrix<double, 2, 3> MMatrix23;
typedef MMatrix<double, 3, 2> MMatrix32;

// Some complex functions can't be inlined so they are defined here to avoid
// compiler warnings.
template <class T, std::size_t Rows, std::size_t Cols>
MMatrix<T, Rows, Cols>::MMatrix(const this_type &prev) {
    for (size_t i = 0; i < Rows; i++) {
        for (size_t j = 0; j < Cols; j++) {
            myData[i][j] = prev(i, j);
        }
    }
}

template <class T, std::size_t Rows, std::size_t Cols>
MMatrix<T, Rows, Cols>::MMatrix(T val) {
    for (size_t i = 0 ; i < Rows; i++) {
        for (size_t j = 0; j < Cols; j++) {
            myData[i][j] = (i == j) ? val : (T) 0.0;
        }
    }
}

template <class T, std::size_t Rows, std::size_t Cols>
int MMatrix<T, Rows, Cols>::pivot(size_t row) {
    int k = int(row);
    double amax, temp;

    if (myData[row][row] != T(0)) {
        return 0;
    }
    amax = -1;
    for (size_t i = row; i < Rows; i++) {
#ifdef OFFLINE //template doesn't know if the value is int, double or float
        if ((temp = ABS(myData[i][row])) > amax && temp != 0.0) {
#else        
        if ((temp = std::abs(myData[i][row])) > amax && temp != 0.0) {
#endif        
            amax = temp;
            k = i;
        }
    }
    if (myData[k][row] == T(0)) {
        return -1;
    }

    if (k != int(row)) {
        T buf;
        for (size_t i = 0; i < Cols; i++) {
            buf = myData[k][i];
            myData[k][i] = myData[row][i];
            myData[row][i] = buf;
        }
        return k;
    }
    return 0;
}

template <class T, std::size_t Rows, std::size_t Cols>
void MMatrix<T, Rows, Cols>::isInverse(const this_type &m) {
    if (Rows != Cols) {
        std::cout << "MatInv: Inversion of a non-square matrix" << std::endl;
    }

    if (Rows == 2) {
        // special formula case for 2x2 matrix
        T det = m(0, 0) * m(1, 1) - m(0, 1) * m(1, 0);
#ifdef OFFLINE //template doesn't know if the value is int, double or float
        if (ABS(det) < MM_LOW_VALUE) {
#else        
        if (std::abs(det) < MM_LOW_VALUE) {
#endif        
#ifdef MM_DEBUG_OUT
            std::cout << "MatInv(2): Inversion of singular matrix, det=" << det
                << std::endl;
#endif // MM_DEBUG_OUT
            det = MM_LOW_VALUE;
        }
        // will assume matrix was non-singular
        T detInv = 1 / det; 
        myData[0][0] = detInv * m(1, 1);
        myData[0][1] = -(detInv * m(0, 1));
        myData[1][0] = -(detInv * m(1, 0));
        myData[1][1] = detInv * m(0, 0);
        return;
    }
    else if (Rows == 3) {
        // special formula case for 3x3 matrix
        T det = m(0, 0) * (m(1, 1) * m(2, 2) - m(1, 2) * m(2, 1))
            - m(0, 1) * (m(1, 0) * m(2, 2) - m(1, 2) * m(2, 0))
            + m(0,
                    2) * (m(1,
                            0) * m(2,
                                1)
                        - m(1,
                            1) * m(2,
                                0));
#ifdef OFFLINE //template doesn't know if the value is int, double or float
                    if (ABS(det) < MM_LOW_VALUE) {
#else        
                    if (std::abs(det) < MM_LOW_VALUE) {
#endif
#ifdef MM_DEBUG_OUT
                        std::cout << "MatInv(3): Inversion of singular matrix, det=" << det
                            << std::endl;
#endif // MM_DEBUG_OUT
                        det = MM_LOW_VALUE;
                    }
                    // will assume matrix was non-singular
                    T detInv = 1 / det; 
                    myData[0][0] = detInv * (m(1, 1) * m(2, 2) - m(1, 2) * m(2, 1));
                    myData[0][1] = detInv * (m(0, 2) * m(2, 1) - m(0, 1) * m(2, 2));
                    myData[0][2] = detInv * (m(0, 1) * m(1, 2) - m(0, 2) * m(1, 1));
                    myData[1][0] = detInv * (m(1, 2) * m(2, 0) - m(1, 0) * m(2, 2));
                    myData[1][1] = detInv * (m(0, 0) * m(2, 2) - m(0, 2) * m(2, 0));
                    myData[1][2] = detInv * (m(0, 2) * m(1, 0) - m(0, 0) * m(1, 2));
                    myData[2][0] = detInv * (m(1, 0) * m(2, 1) - m(1, 1) * m(2, 0));
                    myData[2][1] = detInv * (m(0, 1) * m(2, 0) - m(0, 0) * m(2, 1));
                    myData[2][2] = detInv * (m(0, 0) * m(1, 1) - m(0, 1) * m(1, 0));
                    return;
    }
    else if (Rows == 4) {
        // partial determinants, missing one row and column
        T pdet00 = (m(1, 1) * (m(2, 2) * m(3, 3) - m(2, 3) * m(3, 2)))
            - (m(1, 2) * (m(2, 1) * m(3, 3) - m(2, 3) * m(3, 1)))
            + (m(1, 3) * (m(2, 1) * m(3, 2) - m(2, 2) * m(3, 1)));
        T pdet01 = (m(1, 0) * (m(2, 2) * m(3, 3) - m(2, 3) * m(3, 2)))
            - (m(1, 2) * (m(2, 0) * m(3, 3) - m(2, 3) * m(3, 0)))
            + (m(1, 3) * (m(2, 0) * m(3, 2) - m(2, 2) * m(3, 0)));
        T pdet02 = (m(1, 0) * (m(2, 1) * m(3, 3) - m(2, 3) * m(3, 1)))
            - (m(1, 1) * (m(2, 0) * m(3, 3) - m(2, 3) * m(3, 0)))
            + (m(1, 3) * (m(2, 0) * m(3, 1) - m(2, 1) * m(3, 0)));
        T pdet03 = (m(1, 0) * (m(2, 1) * m(3, 2) - m(2, 2) * m(3, 1)))
            - (m(1, 1) * (m(2, 0) * m(3, 2) - m(2, 2) * m(3, 0)))
            + (m(1, 2) * (m(2, 0) * m(3, 1) - m(2, 1) * m(3, 0)));
        T pdet10 = (m(0, 1) * (m(2, 2) * m(3, 3) - m(2, 3) * m(3, 2)))
            - (m(0, 2) * (m(2, 1) * m(3, 3) - m(2, 3) * m(3, 1)))
            + (m(0, 3) * (m(2, 1) * m(3, 2) - m(2, 2) * m(3, 1)));
        T pdet11 = (m(0, 0) * (m(2, 2) * m(3, 3) - m(2, 3) * m(3, 2)))
            - (m(0, 2) * (m(2, 0) * m(3, 3) - m(2, 3) * m(3, 0)))
            + (m(0, 3) * (m(2, 0) * m(3, 2) - m(2, 2) * m(3, 0)));
        T pdet12 = (m(0, 0) * (m(2, 1) * m(3, 3) - m(2, 3) * m(3, 1)))
            - (m(0, 1) * (m(2, 0) * m(3, 3) - m(2, 3) * m(3, 0)))
            + (m(0, 3) * (m(2, 0) * m(3, 1) - m(2, 1) * m(3, 0)));
        T pdet13 = (m(0, 0) * (m(2, 1) * m(3, 2) - m(2, 2) * m(3, 1)))
            - (m(0, 1) * (m(2, 0) * m(3, 2) - m(2, 2) * m(3, 0)))
            + (m(0, 2) * (m(2, 0) * m(3, 1) - m(2, 1) * m(3, 0)));
        T pdet20 = (m(0, 1) * (m(1, 2) * m(3, 3) - m(1, 3) * m(3, 2)))
            - (m(0, 2) * (m(1, 1) * m(3, 3) - m(1, 3) * m(3, 1)))
            + (m(0, 3) * (m(1, 1) * m(3, 2) - m(1, 2) * m(3, 1)));
        T pdet21 = (m(0, 0) * (m(1, 2) * m(3, 3) - m(1, 3) * m(3, 2)))
            - (m(0, 2) * (m(1, 0) * m(3, 3) - m(1, 3) * m(3, 0)))
            + (m(0, 3) * (m(1, 0) * m(3, 2) - m(1, 2) * m(3, 0)));
        T pdet22 = (m(0, 0) * (m(1, 1) * m(3, 3) - m(1, 3) * m(3, 1)))
            - (m(0, 1) * (m(1, 0) * m(3, 3) - m(1, 3) * m(3, 0)))
            + (m(0, 3) * (m(1, 0) * m(3, 1) - m(1, 1) * m(3, 0)));
        T pdet23 = (m(0, 0) * (m(1, 1) * m(3, 2) - m(1, 2) * m(3, 1)))
            - (m(0, 1) * (m(1, 0) * m(3, 2) - m(1, 2) * m(3, 0)))
            + (m(0, 2) * (m(1, 0) * m(3, 1) - m(1, 1) * m(3, 0)));
        T pdet30 = (m(0, 1) * (m(1, 2) * m(2, 3) - m(1, 3) * m(2, 2)))
            - (m(0, 2) * (m(1, 1) * m(2, 3) - m(1, 3) * m(2, 1)))
            + (m(0, 3) * (m(1, 1) * m(2, 2) - m(1, 2) * m(2, 1)));
        T pdet31 = (m(0, 0) * (m(1, 2) * m(2, 3) - m(1, 3) * m(2, 2)))
            - (m(0, 2) * (m(1, 0) * m(2, 3) - m(1, 3) * m(2, 0)))
            + (m(0, 3) * (m(1, 0) * m(2, 2) - m(1, 2) * m(2, 0)));
        T pdet32 = (m(0, 0) * (m(1, 1) * m(2, 3) - m(1, 3) * m(2, 1)))
            - (m(0, 1) * (m(1, 0) * m(2, 3) - m(1, 3) * m(2, 0)))
            + (m(0, 3) * (m(1, 0) * m(2, 1) - m(1, 1) * m(2, 0)));
        T pdet33 = (m(0, 0) * (m(1, 1) * m(2, 2) - m(1, 2) * m(2, 1)))
            - (m(0, 1) * (m(1, 0) * m(2, 2) - m(1, 2) * m(2, 0)))
            + (m(0, 2) * (m(1, 0) * m(2, 1) - m(1, 1) * m(2, 0)));

        T det = m(0, 0) * pdet00
            - m(0, 1) * pdet01
            + m(0, 2) * pdet02
            - m(0,
                    3) * pdet03;

#ifdef OFFLINE //template doesn't know if the value is int, double or float
        if (ABS(det) < MM_LOW_VALUE) {
#else        
        if (std::abs(det) < MM_LOW_VALUE) {
#endif        
            cout << "MatInv: Inversion of singular matrix, det="
                << det << endl;
            det = MM_LOW_VALUE;
        }
        // will assume matrix was non-singular
        T detInv = 1 / det; 
        myData[0][0] = detInv * pdet00;
        myData[0][1] = -detInv * pdet10;
        myData[0][2] = detInv * pdet20;
        myData[0][3] = -detInv * pdet30;
        myData[1][0] = -detInv * pdet01;
        myData[1][1] = detInv * pdet11;
        myData[1][2] = -detInv * pdet21;
        myData[1][3] = detInv * pdet31;
        myData[2][0] = detInv * pdet02;
        myData[2][1] = -detInv * pdet12;
        myData[2][2] = detInv * pdet22;
        myData[2][3] = -detInv * pdet32;
        myData[3][0] = -detInv * pdet03;
        myData[3][1] = detInv * pdet13;
        myData[3][2] = -detInv * pdet23;
        myData[3][3] = detInv * pdet33;
        return;
    }


    for (size_t i = 0 ; i < Rows; i++) {
        for (size_t j = 0; j < Cols; j++) {
            myData[i][j] = (i == j) ? (T) 1.0 : (T) 0.0;
        }
    }

    this_type m_copy(m);

    for (size_t k = 0; k < Rows; k++) {
        int indx = m_copy.pivot(k);
        if (indx == -1) {
#ifdef MM_DEBUG_OUT
            cout << "MatInv: Inversion of a non-square matrix" << endl;
#endif // MM_DEBUG_OUT
        }

        if (indx != 0) {
            for (size_t i = 0; i < Cols; i++) {
                T tempElt = myData[k][i];
                myData[k][i] = myData[indx][i];
                myData[indx][i] = tempElt;
            }
        }
        T a1 = m_copy(k, k);
        for (size_t j = 0; j < Rows; j++) {
            m_copy(k, j) /= a1;
            myData[k][j] /= a1;
        }
        for (size_t i = 0; i < Rows; i++) {
            if (i != k) {
                T a2 = m_copy(i, k);
                for (size_t j = 0; j < Rows; j++) {
                    m_copy(i, j) -= a2 * m_copy(k, j);
                    myData[i][j] -= a2 * myData[k][j];
                }
            }
        }
    }
    return;
}
#endif // MINIMAL_MATRIX_H
