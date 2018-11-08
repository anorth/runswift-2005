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

#include "Stats.h"
//Constructor
Stats::Stats(DataSet& ds, int n) : dimension(n), dat(ds)
{
}
// Destructor
Stats::~Stats()
{
}

/*****************************
 * double mean( const int)
 *
 * Calculates and returns the mean
 * restrictions: index < dimension
 ********************************/
double Stats::mean(const unsigned int index)
{
    DataIter i;
    double total=0;
    if (index>=dimension)
        return INT_MAX;
    for (i=dat.begin();i!=dat.end();i++)
        total+= (*(*i))[index];
    return total/dat.getLength();
}

/*****************************
 * int compareDouble(const void* d1, const void* d2)
 * Simple comparison function for scalars of the data set
 * @return -1 if d1<d2, 
 *          0 if equal and 
 *          1 if d2>d1
 ****************************/

int compareDouble(const void* d1, const void* d2) {
  double diff = *((double*)d2)- *((double*)d1);
  if (diff>0)
    return -1;

  else if (diff==0)
    return 0;
  else
    return 1;
  
}

/****************************
 * void print(const unsigned int index)
 *
 * Prints all scalars of the dataset at the specified vector index
 ***************************/
void Stats::print(const unsigned int index) {
  DataIter i;
  for (i=dat.begin();i!=dat.end();i++) {
    cout << (**i)[index] << endl;
  }
}

/****************************
 * void print(const unsigned int index)
 *
 * Prints all the vectors of the dataset
 ***************************/
void Stats::print() {
  DataIter i;
  for (i=dat.begin();i!=dat.end();i++) {
    cout << "{";
    for (int j=0;j<dimension;j++) {
      cout << (**i)[j];
      if (j!=dimension-1)
	cout << ", ";
    }
    cout << "}" << endl;
  } 
}

/****************************
 * void print(const unsigned int index)
 *
 * Prints all the vectors of the dataset in SORTED order
 ***************************/
void Stats::printSorted(const unsigned int index) {
  double* base = new double[dat.getLength()];
  DataIter i;
  int j;
  for (i=dat.begin(),j=0;i!=dat.end();i++,j++)
    base[j]=(**i)[index];
  qsort(base, dat.getLength(), sizeof(double), &compareDouble);
  cout << "Sorted:" << endl;
  for (j=0;j<dat.getLength(); j++) 
    cout << base[j] << endl;
}  

double Stats::median(const unsigned int index) {
  double* base = new double[dat.getLength()];
  DataIter i;
  int j;
  for (i=dat.begin(),j=0;i!=dat.end();i++,j++)
    base[j]=(**i)[index];
  qsort(base, dat.getLength(), sizeof(double), &compareDouble);
  return base[dat.getLength()/2];
}

double Stats::midpoint(const unsigned int index)
{
  DataIter i=dat.begin();
  double max=(**i)[index], min=(**i)[index];
  for(;i!=dat.end();i++) {
    double val = (**i)[index];
    if (val<min) {
      min=val;
    }
    else if (val>max) {
      max=val;
    }
  }
  return min + (max-min)/2;
}

/*****************************
 * double dist( const int)
 *
 * Calculates and returns the sum of distances from the average
 ********************************/
double Stats::dist(const unsigned int index)
{
    DataIter i;
    double xDiff=0, xAvg=mean(index);
    if (index>=dimension)
        return INT_MAX;
    for(i=dat.begin();i!=dat.end();i++)
        xDiff+=pow((**i)[0]-xAvg,2);
    return xDiff;
}


/*****************************
 * double mean( const int)
 * Calculates the beta regression values
 * restrictions: dim=2
 ********************************/
void Stats::getBetas(double& beta0, double& beta1)
{
    double xavg=mean(0);
    double yavg=mean(1);
    double sum1, sum2;
    DataIter i;
    
    for (i=dat.begin();i!=dat.end();i++)
    {
        sum1+= (**i)[0] * (**i)[1];
        sum2+= pow((**i)[0],2);
    }
    beta1=(sum1- dat.getLength()*xavg*yavg) / (sum2 - dat.getLength() * pow(xavg,2));
    beta0=yavg-beta1*xavg;
}
