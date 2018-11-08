#
#   Copyright 2004 The University of New South Wales (UNSW) and National  
#   ICT Australia (NICTA).
#
#   This file is part of the 2004 team rUNSWift RoboCup entry.  You may  
#   redistribute it and/or modify it under the terms of the GNU General  
#   Public License as published by the Free Software Foundation; either  
#   version 2 of the License, or (at your option) any later version as  
#   modified below.  As the original licensors, we add the following  
#   conditions to that license:
#
#   In paragraph 2.b), the phrase "distribute or publish" should be  
#   interpreted to include entry into a competition, and hence the source  
#   of any derived work entered into a competition must be made available  
#   to all parties involved in that competition under the terms of this  
#   license.
#
#   In addition, if the authors of a derived work publish any conference  
#   proceedings, journal articles or other academic papers describing that  
#   derived work, then appropriate academic citations to the original work  
#   must be included in that publication.
#
#   This rUNSWift source is distributed in the hope that it will be useful,  
#   but WITHOUT ANY WARRANTY; without even the implied warranty of  
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  
#   General Public License for more details.
#
#   You should have received a copy of the GNU General Public License along  
#   with this source code; if not, write to the Free Software Foundation,  
#   Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


# 
#  Last modification background information
#  $Id: StateBasedAction.py 4620 2005-01-06 23:58:24Z alexn $
# 
#  Copyright (c) 2004 UNSW
#  All Rights Reserved.
# 




"""
StateBasedAction: base class in state-based behaviours framework.
TODO: 
FIX: state's DecideNextAction is called more than once
    -> print warning msg?
"""
#Comments:
#    - There might be a delay of 1 vision frame to jump to the right state to actually does something
#
#

#Bug history: 
# Global.frame != self.lastFrameID + 1
#
import Debug
import Global

def className(someClass):
    return str(someClass).strip("<>").split()[0]
    
STACK_LIMIT = 20


def instrument(stateName,debugMsg):
    if stateName == Debug.DEBUGING_STATE:
        print debugMsg," [%d]"%Global.frame
        
#Action is an un-intermitted sequence of decision (every frame)
class StateBasedAction:
    def __init__(self,name = None): 
        if name is None:
            name = className(self)
        self.name = name
        self.counter = 0
        self.lastFrameID = -2
        self.previousState = None
        self.outState = None
        self.subState = None
        self.isDoing = False
        self.exitGate = None
        
    #is set by DecideNextAction, private function
    def _setPreviousState(self,previousState):
        self.previousState = previousState
        
    #allow one output state. 
    #Probably only useful in pipeline architecture
    #On one hand, it supports loose-coupling, on the other hand, 
    #it might be violated in the code( for ex, use self.outState other than
    # "return self.outState"
    # exit to outState by calling "return self.outState"
    # otherwise "return None"
    def setOutState(self,outState):
        self.outState = outState
    def getOutState(self):
        return self.outState
    
    #called whenever the action is entered (an action can be entered many times)
    def _beginAction(self):
        if Debug.bDebugInOutState : print "Begin ", self.name
        #clean up last execution (zombie), sothat stateStack is poped properly
        if self.isDoing: 
            instrument(self.name,"F, isDoing")
            self._finishAction() 
        self.beginAction()
        self.counter = 0
        self.isDoing = True
        if len(Debug.stateStack) > STACK_LIMIT:
            if Debug.stateBasedDebugAll:   
                print "State stack overflow, must be broken states"
        else:
            Debug.stateStack.append(self.name)
        
    def _finishAction(self):
        if Debug.bDebugInOutState : print "Finish ", self.name, " gate:",self.exitGate
        self.finishAction()
        self.counter = 0
        self.lastFrameID = -2
        self.isDoing = False 
        if self.subState is not None: #when parent is finished, child is finished too
            instrument(self.subState.name,"F, parent quit")
            self.subState._finishAction()
        try:
            Debug.stateStack.remove(self.name)
        except ValueError:
            if Debug.stateBasedDebugAll:
                print "Weird, State not in the stack ",self.name
        
    def beginAction(self):
        pass
    #called whenever the action is finished (for e.g. : clean up for the next action)
    def finishAction(self): 
        pass
        
    #public function, for the end-user to call
    #can be overided when wanting to break the preset structure
    def DecideNextAction(self):
        if Global.frame > self.lastFrameID + 1:
            instrument(self.name,"B, DNA %d"%self.lastFrameID )
            self._beginAction()
        self.lastFrameID = Global.frame
        nextState = self.DoAction()
        self.counter += 1
        if nextState != self:#done this state, call finishAction()
            instrument(self.name,"F, DNA")
            self._finishAction()
            if nextState is not None:
                nextState._setPreviousState(self)

        return nextState
        
    #Write code here.
    def DoAction(self):
        pass
       
    #Pulling states
    def shouldBeActivated(self):
        return False
    
    def doSubState(self,state = None,immediate = True):
##~         print "do sub",state,self.subState
        if state is not None and state != self.subState:  
            if self.subState is not None:
                instrument(self.subState.name,"F, parent force exit")
                self.subState._finishAction()
            self.subState = state
        if self.subState is not None and immediate:
            self.subState = self.subState.DecideNextAction()
                
    def shouldContinue(self):
        return self.isDoing
    
    #for debugging
    def setExitGate(self,gate):
        self.exitGate = gate
    
    def getName(self):
        return self.name
