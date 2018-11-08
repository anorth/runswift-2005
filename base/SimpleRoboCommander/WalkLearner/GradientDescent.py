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

bDebugLoading = False

def parseListString(s ):
    return map(float, s.strip(" []\n").split(",") )

def parse2TupleFloatString(s):
    s = s.strip(" \n")
    ( t1, t2, f) = s.split(";")
    return parseListString(t1), parseListString(t2), float(f)
    
def formatFloat(fl):
    return "%.5f" % fl
    
def formatList(l):
    return "[%s]" % ",".join(map( formatFloat, l))
    
#todo : multiple runs for each params?
#bodyTile problem?
class GradientDescent:
    NUMBER_OF_POLICIES_PER_ITERATION = 50
    
    def __init__(self, startFromScratch = True, logFileName = "gradientDesc.log"):
        self.logFileName = logFileName
        self.evalList = []
        self.parameters = None
        self.deltaParams = None
        self.logging = False
        self.bestParams,self.bestValue = [], 100000000
        self.cachingEvaluation = {}
        
        
        if startFromScratch:            
            self.logFile = open(self.logFileName,"w")
            self.logFile.close()
            self.logging = True
            print "Starting GradientDescent from scratch..."
            self.writeToLog("GradientDescent starting up...")
        else:
            print "Loading GradientDescent from log file..."
            self.loadFromFile(self.logFileName)
        
    def writeToLog(self, msg):
        self.logFile = open(self.logFileName,"a")   
        self.logFile.write(msg + "\n")
        self.logFile.close()
        
    def setParameters(self,parameters):
        self.parameters = parameters
        if self.logging:
            self.writeToLog("Starting parameters : " + formatList(self.parameters))
        if self.deltaParams is not None:
            self.updatePlans()
        
    def setDeltaParameters(self,deltaParams):
        self.deltaParams = deltaParams
        if self.logging:
            self.writeToLog("Delta parameters : " + formatList(self.deltaParams))
        if self.parameters is not None:
            self.updatePlans()
        
    def getCurrentParameters(self):
        return self.parameters
    
    def getNextEvaluation(self):
        i = 0
        print "Get Next Evaluation ---- ",
        for (policy, _ , cost) in self.evalList:
            if cost == -1:
                self.evalList[i][2] = 0 #cost set to 0 - waiting for answer
                print "Return ",i," -  ", policy
                return (i,policy)
            i += 1
        print "What? returning none?"
        return (None,None)
        
    def setEvaluation(self,id,value):
        print "python: setEvaluation: id: ", id
        if id > len(self.evalList):
            print "-------  Error: setEvaluation : evaluation id is invalid"
        else:
            params = self.evalList[id][0]
            if self.cachingEvaluation.setdefault(params,value) != value:
                print "WARNING: cached value is different from this value"
                print "Param =", params, "  old :", self.cachingEvaluation[params], "  new:",value
                
            self.evalList[id][2] = value
            if value < self.bestValue:
                self.bestValue = value
                self.bestParams = self.evalList[id][0] 
            
            print "-------  GradientDescent : got evaluation : [", id, "]", \
                " policy = " , self.evalList[id]
            if self.logging:
                self.writeToLog( "id %d : %s ; %s ; %.2f" % (id, \
                    formatList( self.evalList[id][0]), formatList(self.evalList[id][1]),self.evalList[id][2] ) )
                
            if self.checkAllPoliciesDone(self.evalList):
                self.updateNextParameters()
                self.updatePlans()
                
    def cancelEvaluation(self,policyID):
        try:
            self.evalList[policyID][2] = -1 # 2 = cost 
        except:
            print "This policyID is wrong" , policyID
            return None
            
    def getBestParameter(self):
        return (self.bestParams, self.bestValue)
        
    ##################### private function #####################    
    
    def updatePlans(self):
        self.evalList = [[tuple(self.parameters), [0]*len(self.parameters), -1]]
        for _ in range(self.NUMBER_OF_POLICIES_PER_ITERATION):
            r = self.randomDirectionVector( len(self.parameters) )
            policy = map( lambda para, delta, dir : para + delta * dir , self.parameters, self.deltaParams, r)
            policyTup = tuple(policy)
            if self.cachingEvaluation.get(policyTup,None) is not None:
                cost = self.cachingEvaluation[policyTup]
            else:
                cost = -1
            self.evalList.append( [policyTup,r,cost] ) # r : random direction
            
        if self.checkAllPoliciesDone(self.evalList):
            print "Recursive call: all generated policy has been evaluated"
            self.updateNextParameters()
            self.updatePlans()
    
    def checkAllPoliciesDone(self,policyList):
        for (_, _ , cost) in policyList:
            if cost == -1 or cost == 0: #there one undone policy left
                return False
        return True
    
    def updateNextParameters(self):
        
        gradientList = []
        for i in range(len(self.parameters)):
            minusCount, minusTotal = 0,0
            zeroCount, zeroTotal = 0,0
            plusCount, plusTotal = 0,0
            for ( _ , dirVector, cost) in self.evalList:
                if dirVector[i] == -1:
                    minusTotal += cost
                    minusCount += 1
                elif dirVector[i] == 0:
                    zeroTotal += cost
                    zeroCount += 1
                else:
                    plusTotal += cost
                    plusCount += 1
                
            grad = 0
            if minusCount > 0 and zeroCount > 0 and plusCount > 0:
                minusAver = 1.0 * minusTotal / minusCount
                zeroAver = 1.0* zeroTotal / zeroCount
                plusAver = 1.0* plusTotal / plusCount
                if zeroAver > minusAver and zeroAver > plusAver:
                    grad = 0
                else:
                    grad = (plusAver - minusAver)
            gradientList.append(grad)
            
        v = Vector(gradientList)
        v.standardize()
        if self.logging:
            self.writeToLog("Gradient " + `v.toList()`)
        deltaLength = Vector(self.deltaParams).length()
        v.multiply(deltaLength)
##~         v.multiplyVector(self.deltaParams)
        v.add(self.parameters)
        self.parameters = v.toList()
        if self.logging:
            self.writeToLog("Updated current parameters: " + formatList(self.parameters))
        
    
    def randomDirectionVector(self, length):
        vector = []
        for _ in range(length):
            vector.append( random.randint(-1,1) )
        return vector
    
    def loadFromFile(self,fileName):
        self.logging = False
        file = open(fileName,"r")
        
        print "Loading session from ", fileName, 
        print "Loading :", file.readline()
        self.parseStartingParameter(file.readline())
        self.parseDeltaParameters(file.readline())
        while True:
            line = file.readline()
            if line == "": break
            if bDebugLoading:
                print "Loading :", line
            self.parseALine(line)
        file.close()
        self.logging = True
        
    def parseStartingParameter(self,line):
        if (len(line) < 22) : return
        line = line[22:]
        print "Got params" , line
        self.setParameters(parseListString(line))
        
    def parseDeltaParameters(self,line):
        if (len(line) < 19) : return
        line = line[19:]
        print "Got delta" , line
        self.setDeltaParameters(parseListString(line))
    
    def parseALine(self,line):
        if line.startswith("id"):
            idStr, s = line.split(":")
            id = int(idStr[3:])
            if id > len(self.evalList) : return;
            policy, r, cost = parse2TupleFloatString(s)
            self.evalList[id] = (policy, r, cost)
            if cost > self.bestValue:
                self.bestValue = cost
                self.bestParams = policy
                
        elif line.startswith("Updated current"):
            line = line[28:]
            self.setParameters(parseListString(line))
        
        if self.checkAllPoliciesDone(self.evalList):
            self.updateNextParameters()
            self.updatePlans()
        
