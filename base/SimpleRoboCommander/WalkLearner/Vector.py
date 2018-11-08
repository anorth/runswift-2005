##  
##     Copyright 2004 The University of New South Wales (UNSW) and National  
##     ICT Australia (NICTA).
##  
##     This file is part of the 2004 team rUNSWift RoboCup entry.  You may  
##     redistribute it and/or modify it under the terms of the GNU General  
##     Public License as published by the Free Software Foundation; either  
##     version 2 of the License, or (at your option) any later version as  
##     modified below.  As the original licensors, we add the following  
##     conditions to that license:
##  
##     In paragraph 2.b), the phrase "distribute or publish" should be  
##     interpreted to include entry into a competition, and hence the source  
##     of any derived work entered into a competition must be made available  
##     to all parties involved in that competition under the terms of this  
##     license.
##  
##     In addition, if the authors of a derived work publish any conference  
##     proceedings, journal articles or other academic papers describing that  
##     derived work, then appropriate academic citations to the original work  
##     must be included in that publication.
##  
##     This rUNSWift source is distributed in the hope that it will be useful,  
##     but WITHOUT ANY WARRANTY; without even the implied warranty of  
##     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  
##     General Public License for more details.
##  
##     You should have received a copy of the GNU General Public License along  
##     with this source code; if not, write to the Free Software Foundation,  
##     Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
##  
##  
##  
##   Last modification background information
##   $Id$
##  
##   Copyright (c) 2004 UNSW
##   All Rights Reserved.
##  
##  



import math

def toDouble(list): #convert everything to float
    return map(float , list)
    
EPSILON = 0.00001

class Vector:
    def __init__(self,list):
        self.vector = toDouble(list) #make sure every element is double
        
    def length(self):
        total = 0.0
        for x in self.vector:
            total += x * x
        return math.sqrt(total)
        
    def standardize(self):
        l = self.length()
        if abs(l) < EPSILON: return
        
        self.vector = [ x / l for x in self.vector ]
        
    def multiply(self, constant):
        self.vector = [x * constant for x in self.vector]
    
    def multiplyVector(self, otherVec):
        self.vector = [x * y for x,y in zip (self.vector, otherVec) ]
            
    def add(self,otherVec):
        self.vector = [x + y for x,y in zip (self.vector, otherVec) ]
        
    def toList(self):
        return self.vector
