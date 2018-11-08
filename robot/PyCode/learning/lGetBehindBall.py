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
#  $Id: lGetBehindBall.py 4620 2005-01-06 23:58:24Z alexn $
# 
#  Copyright (c) 2004 UNSW
#  All Rights Reserved.
# 




#===============================================================================
#   Python Behaviours : 2004 (c) 
#
# lGetBehindBall.py
#
#
#===============================================================================
from SysPath import *

import Constant
import Debug
import Global
import HeadAction
import HelpGlobal
import HelpMath
import HelpTrack
import IndicatorAction
import lQLearning
import ReadyPlayer
import VisionLink
import WalkAction 

MAX_Q_VALUE = 100

EXPLORATION_PROBABILITY = 80
LEARNING_BALL_DISTANCE = 50

S_DISTANCE_TO_BALL    = 0
S_HEADING_TO_BALL     = 1
S_DIRECTION_TO_TARGET = 2


ACTION    = 0
A_FRAME   = 0
A_FORWARD = 0
A_SIDE    = 1
A_TURN    = 2
A_PG      = 3
A_HDF     = 4
A_HDB     = 5

g_CurrentS = None
g_CurrentA = None
g_NextS    = None


###
# [[s1,s2],[a1,a2,a3]]

STATE_INITIALIZE = 0
STATE_READY      = 1   
STATE_EXECUTE    = 2 
STATE_UPDATE     = 3
STATE_FIX        = 4
STATE_DONE       = 5
g_CurrentState = STATE_INITIALIZE
g_CurrentGenerator = None

g_Learning = None


def GetNextAction(): 
    forward = VisionLink.random()%20 - 7
    left    = VisionLink.random()%20 - 10
    turn    = VisionLink.random()%80 - 40
    pg      = 80 - VisionLink.random()%60
    hdf     = 40 - VisionLink.random()%40
    hdb     = 40 - VisionLink.random()%40
    
    return [forward,left,turn,pg,hdf,hdb]
    
    
def GetReward(current_state,current_action,next_state):
    print "Getting Reward" + str(current_state) + " " + str(current_action) + " " + str(next_state)
    reward_dist = current_state[0] - next_state[0] 
    reward_angle = current_state[1] - next_state[1]
    reward_action = current_action[0] + current_action[1] + current_action[2]/10     
    return reward_dist + reward_angle + reward_action



def DecideNextAction(): 
    global g_CurrentGenerator
    global g_CurrentState
    Global.frameReset()   
    
    # Tracking Visual Ball!!!                         
    HelpTrack.trackVisualBall()
    
    if g_CurrentGenerator == None: 
        if g_CurrentState == STATE_INITIALIZE: 
            print "Initializing"
            g_CurrentGenerator = StateInitialize()
        if g_CurrentState == STATE_READY: 
            print "Ready"
            g_CurrentGenerator = StateReady()
        elif g_CurrentState == STATE_EXECUTE: 
            print "Executing"
            g_CurrentGenerator = StateExecute()
        elif g_CurrentState == STATE_UPDATE: 
            print "Update"
            g_CurrentGenerator = StateUpdate()
        elif g_CurrentState == STATE_FIX: 
            print "Fixing" 
            g_CurrentGenerator = StateFix() 
        elif g_CurrentState == STATE_DONE: 
            print "Done"
            g_CurrentGenerator = StateDone()
    
    next_state = g_CurrentGenerator.next()
    if next_state != g_CurrentState: 
        g_CurrentState = next_state
        g_CurrentGenerator = None


        
def ChangeState(state): 
    global g_CurrentState, g_CurrentGenerator
    g_CurrentState = state
    g_CurrentGenerator = None


def StateInitialize(): 
    global g_Learning, g_CurrentS, g_NextS 
    g_CurrentS = [0,0,0]
    g_NextS = [0,0,0]
    g_Learning = lQLearning.QLearning(MAX_Q_VALUE) 
    yield STATE_READY
   
 
def StateReady(): 
    global g_CurrentA
    g_CurrentA = GetNextAction()
    yield STATE_EXECUTE 
   
   
FRAME_INTERVAL = 30  
def StateExecute(): 
    global g_CurrentState
    global g_CurrentS, g_NextS
    
    for i in range(FRAME_INTERVAL):
        if Global.vBall.getConfidence() > 0 or Global.lostBall < 5:
            
            if Global.ballD > LEARNING_BALL_DISTANCE: 
                yield STATE_FIX
                
            if Global.vBall.getConfidence() > 0: 
                # Keep updating the next state  
                g_NextS[S_DISTANCE_TO_BALL]    = Global.ballD
                g_NextS[S_HEADING_TO_BALL]     = Global.ballH
                g_NextS[S_DIRECTION_TO_TARGET] = 0
                
            Global.finalAction[Constant.AAWalktype] = Constant.NormalWalkWT
            Global.finalAction[Constant.AAForward]  = g_CurrentA[A_FORWARD]
            Global.finalAction[Constant.AALeft]     = g_CurrentA[A_SIDE] 
            Global.finalAction[Constant.AATurnCCW]  = g_CurrentA[A_TURN]
            Global.finalAction[Constant.AAPg]       = g_CurrentA[A_PG]
            Global.finalAction[Constant.AAHdf]      = g_CurrentA[A_HDF]
            Global.finalAction[Constant.AAHdb]      = g_CurrentA[A_HDB]

            yield STATE_EXECUTE
        else:  
            print "You have lost the ball, no good, reward must be low"
            WalkAction.setNormalWalk(0,0,0)
            yield STATE_FIX     
    yield STATE_UPDATE
    
    
def StateUpdate(): 
    global g_CurrentS, g_CurrentA
    
    reward = GetReward(g_CurrentS,g_CurrentA,g_NextS)
    print "reward is " + str(reward)
    g_Learning.UpdateValues(g_CurrentS,g_CurrentA,reward,g_NextS)
    
    # s <- s'
    g_CurrentS = g_NextS[:] 
     
    prob = VisionLink.random() % 100 
    if prob < EXPLORATION_PROBABILITY: 
        g_CurrentA = GetNextAction() 
    else: 
        g_CurrentA = g_Learning.GetArgMax(g_CurrentS)
        print "Using QTable to get good action!!!"
        if g_CurrentA == None: 
            g_CurrentA = GetNextAction()
        
    yield STATE_EXECUTE


def StateFix(): 
    global g_CurrentS, g_CurrentA, g_Learning
    print "reward is -1000"
    g_Learning.UpdateValues(g_CurrentS,g_CurrentA,-1000,g_NextS)
    
    while 1: 
        if Global.vBall.getConfidence() > 0: 
            
            g_NextS[S_DISTANCE_TO_BALL]    = Global.ballD
            g_NextS[S_HEADING_TO_BALL]     = Global.ballH
            g_NextS[S_DIRECTION_TO_TARGET] = 0
            g_CurrentA = GetNextAction() 
            
            turn = HelpMath.CLIP(Global.ballH,20)
            
            WalkAction.setNormalWalk(10,0,turn) 
            
            if Global.ballD > LEARNING_BALL_DISTANCE: 
                yield STATE_FIX
            else:    
                WalkAction.setForceStepComplete()
                yield STATE_EXECUTE
        
        else:    
                        
            HeadAction.setHeadParams(0,0,0)    
            WalkAction.setNormalWalk(0,0,10) 
            yield STATE_FIX
  

def StateDone(): 
    while 1:
        yield STATE_DONE

   
def processCommand(cmdStr):
    cmdStr = cmdStr.strip()
    command = cmdStr.split()[0]
    parameters = cmdStr[len(command):]
    print "WalkLearner got [%s]%s." % (command,parameters)
    
    if command == "start":
        ChangeState(STATE_READY)        
    elif command == "query":
        pass
    elif command == "stop":
        ChangeState(STATE_DONE)        
    elif command == "set":
        pass    
    elif command == "slp":
        pass
    elif command == "ready":
        pass



