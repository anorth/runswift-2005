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



from GradientDescent import *
from PowellDescent import *
from LearnError import LearnError
from sys import *

class ErrorFunctionBase:
    def __init__(self):
        self.callCount = 0
    def getCallCount(self):
        return self.callCount
        
class HumanErrorFunction(ErrorFunctionBase):
    def __init__(self):
        ErrorFunctionBase.__init__(self)
        
    def getFirstParams(self):
        return [0.0, 0.0]
        
    def getDeltaParams(self):
        return [ 4,  2,  2]
        
    def evaluate(self,params):
        self.callCount += 1
        print " Cost " + `params` + " ? " ,
        cost = float(stdin.readline())
        print "Ok, got " , cost
        return cost

class SimpleErrorFunction(ErrorFunctionBase):
    def __init__(self):
        ErrorFunctionBase.__init__(self)
        
    def getFirstParams(self):
        return [0,0,0]
        
    def getDeltaParams(self):
        return [ 1,1,1 ]
        
    def evaluate(self,params):
        self.callCount += 1
        [x,y,z] = params
        cost = - x * y * z + - 100 * x * y + 5 * x * z + 9 * y * z
        return cost
        
class RosenbrockFunction(ErrorFunctionBase):
    def __init__(self):
        ErrorFunctionBase.__init__(self)

    def getFirstParams(self):
        return [0.8,1.2,0.7]
        
    def getDeltaParams(self):
        return [ 0.001,0.001,0.001 ]
        
    def evaluate(self,params):
        self.callCount += 1
        [x,y,z] = map(float,params)
        cost = 100 * (y - x ** 2) ** 2 + 100 * (z - y ** 2 ) ** 2 + ( 1 - x ) ** 2 + ( 1 - y ) ** 2
        return -cost
        
class TestLearning:
    def __init__(self, learningAlgo, errFun):
        self.learningAlgo = learningAlgo
        self.errFun = errFun
    
    def run(self):
        params = self.errFun.getFirstParams()
        self.learningAlgo.setParameters(params)
        deltaParams = self.errFun.getDeltaParams()
        self.learningAlgo.setDeltaParameters(deltaParams)
        while True:
            (rid, params) = self.learningAlgo.getNextEvaluation()
            if rid is None: break
            
            cost = self.errFun.evaluate( params )
            if cost == -1 : break
            if cost == -2: 
                self.learningAlgo.cancelEvaluation(rid)
            else:
                self.learningAlgo.setEvaluation(rid, cost)
            print "Param, cost = ", params, cost
        

if __name__ == "__main__":
    la = PowellDescent(True)
##~     la = GradientDescent()
##~     la = LearnError()
##~     errFun = HumanErrorFunction()
##~     errFun = SimpleErrorFunction()
    #errFun = RosenbrockFunction()
    errFun = HumanErrorFunction()
    test = TestLearning(la,errFun)
    try:
        test.run()
    except KeyboardInterrupt:
        pass
        
    print la.getBestParameter()
    print "Number of fucntion calls: ",errFun.getCallCount()
