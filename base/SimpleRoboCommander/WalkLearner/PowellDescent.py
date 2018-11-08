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



from powell import *
from threading import Event
import thread #low-level threads
import optimize

dummyID = -1
bDebugThread = True

def debug(*arg):
    if bDebugThread:
        print arg

def checkLogFile(logFileName):
    try:
        f = open(logFileName,"r")
        f.readline()
        line = f.readline()
        while True:
            if line[:4] != MARKING:
                return False
            line = f.readline() #base
            line = f.readline() #dirVec
            line = f.readline() #x,fx,fcalls
            line = f.readline() 
            if line == "": break
        f.close()
        return True
    except:
        return False
        
#todo : multiple runs for each params?
class PowellDescent:
    def __init__(self, startFromScratch = True, logFileName = "powell.log"):
##~         sys.setcheckinterval(1)
        if not startFromScratch and checkLogFile(logFileName):
            self.startFromScratch = False
            print "Resuming from previous run..."
        else:
            self.startFromScratch = True
            print "Starting from scratch..."
        self.logFileName = logFileName
        self.running = False
        self.eventDog = Event()
        self.eventPowell = Event()
        self.nextParameters = None
        self.lowerLimits = None
        self.upperLimits = None
        self.evaluation = None
        self.bestParams,self.bestValue = [], 10000000
        self.isStopped = False
        self.cachingEvaluation = {}
        
        #2 variables to ensure only one param is evaluated and powell 
        #thread is resumed only when the evaluation is done
        self.paramsIsDone = False
        self.paramsInUse = False 

    def startOptimization(self):
        #need to stop the dog thread
        self.running = True
        self.eventDog.clear()
        #thread.start_new_thread(fminPowellNew,(self.walkingFunction, self.nextParameters, (), self.startFromScratch, \
        #                                       False, 0.2, 50)) # shouldBreak, xtol, ftol
        thread.start_new_thread(optimize.fmin, (self.walkingFunction, self.nextParameters, (), 0.2, 50))

        
        debug( "Sleeping..")
        self.eventDog.wait()
        debug( "..done" )
    
    #note:this is run in a separate thread - powell thread
    def walkingFunction(self,params):
        if params is None: #powell tells to stop
            print "Powell is stoped"
            self.isStopped = True
            self.eventDog.set()
            return None
        
        rawParams = tuple(params)
        tempList = []
        for p in rawParams:
            tempList.append( round(p, 1) ) # round into 1 decimal precision
        self.nextParameters = tuple(tempList)
        if self.cachingEvaluation.get(self.nextParameters, None) is not None:
            # check cache first
            self.evaluation = self.cachingEvaluation[self.nextParameters]
            print __name__, "Got already evaluated policy:", self.nextParameters
            print __name__, "Evaluation value:", self.evaluation
            return self.evaluation
        elif self.isOutOfLimits():
            # check for limit
            print __name__, "Parameters are out of limits!", self.nextParameters
            print __name__, "Setting evaluation value to:", 100000.0 # 100 sec
            return 100000.0 # 100 sec
        else:            
            self.evaluation = None
            self.paramsIsDone = False
            self.eventPowell.clear()
            self.eventDog.set()
            debug( "Powell Waiting...", self.nextParameters)
            self.eventPowell.wait()
            debug( "Powell executing...")
            if self.evaluation is None:
                raise Exception("Invalid evaluation")
            return self.evaluation #minimization

    def isOutOfLimits(self):
        n = len(self.nextParameters)
        for i in xrange(n):
            if self.lowerLimits and self.nextParameters[i] < self.lowerLimits[i]:
                return True
            if self.upperLimits and self.nextParameters[i] > self.upperLimits[i]:
                return True
        return False
        
    def writeToLog(self, msg):
        self.logFile = open(self.logFileName,"a")   
        self.logFile.write(msg + "\n")
        self.logFile.close()
        
    def setParameters(self,parameters):
        self.nextParameters = parameters
        
    def setDeltaParameters(self,deltaParams):
        pass

    def setLowerLimits(self, limits):
        self.lowerLimits = limits

    def setUpperLimits(self, limits):
        self.upperLimits = limits


    def getCurrentParameters(self):
        return self.nextParameters
    
    def getNextEvaluation(self):
        if self.isStopped :
            print "Already stop"
            return (None,None)
        debug( "getNextEvaluation")
        if not self.running:
            debug( "Starting up optimization")
            self.startOptimization()
           
        if self.paramsIsDone:
            self.paramsIsDone = False
            debug( "Evaluated, resume powell")
            self.eventDog.clear()
            self.eventPowell.set()
            debug( "Dog Waiting...")
            self.eventDog.wait()
            if self.isStopped:
                print "Already stoped"
                return (None,None)
            debug( "Dog executing...")
        elif self.paramsInUse:
            print "current Params is being used, got nothing more to do"
            return (None,None) 
        self.paramsInUse = True
        global dummyID
        dummyID += 1
        print __name__, "getNextEvaluation:", dummyID, self.nextParameters
        return (dummyID, self.nextParameters)

    def setEvaluation(self,_,value):
        debug( "Setting ", value)
        self.evaluation = value
        if value < self.bestValue:
            self.bestValue = value
            self.bestParams = self.nextParameters
        self.paramsInUse = False
        self.paramsIsDone = True
        
        if self.cachingEvaluation.setdefault(self.nextParameters, value) != value:
            print "WARNING: cached value is different from this value",
            print "Param =", self.nextParameters, "  old :",
            print self.cachingEvaluation[self.nextParameters], "  new:",value        
##~         self.eventPowell.set()
        
                
    def cancelEvaluation(self,_):
        self.paramsInUse = False
        
    def getBestParameter(self):
        return (self.bestParams,self.bestValue)
        
    ##################### private function #####################    
    
    def loadFromFile(self,fileName):
        pass
        
