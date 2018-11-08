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



import random #psedo-random generator
from Vector import * #my module 
import math

def sum(aList):
    total = 0
    for v in aList:
        total += v
    return total
        
def getMean(values):
    return float(sum(values)) / len(values)

def getStandardDeviation(values):
    m = getMean(values)
    sq = map( lambda x: (x-m)**2 , values)
    sumSQ = sum(sq) / len(values)
    return math.sqrt(sumSQ)

class LearnError:   
    def __init__(self, startFromScratch = True, logFileName = "learnError.log"):
        self.parameters = None
        self.deltaParams = None
        self.evaluation = []
        self.logFileName = logFileName
        self.fd = open(logFileName,"w")
        self.fd.write("Evaluation of the same\n")
        self.fd.close()
        
    def setParameters(self,parameters):
        self.parameters = parameters
    
    def setDeltaParameters(self,deltaParams):
        self.deltaParams = deltaParams
        
    def getCurrentParameters(self):
        return self.parameters
        
    def getNextEvaluation(self):
        return (0,self.parameters)
        
    def setEvaluation(self,id,value):
        self.evaluation.append(value)
        self.fd = open(self.logFileName,"a")
        self.fd.write("%.2f\n"%value)
        self.fd.close()
        
    def cancelEvaluation(self,policyID):
        pass
            
    def getBestParameter(self):
        return ( "n = %d , mean = %.2f , sd = %.2f" % 
            (len(self.evaluation), getMean(self.evaluation), getStandardDeviation(self.evaluation)), 
            "%.2f"%max(self.evaluation)  )
        
    
