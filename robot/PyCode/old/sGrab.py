##     Copyright 2005 The University of New South Wales (UNSW) and National  
##     ICT Australia (NICTA).
##  
##     This file is part of the 2005 team rUNSWift RoboCup entry.  You may  
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
##     should be included in that publication.
##  
##     This rUNSWift source is distributed in the hope that it will be useful,  
##     but WITHOUT ANY WARRANTY; without even the implied warranty of  
##     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  
##     General Public License for more details.
##  
##     You should have received a copy of the GNU General Public License along  
##     with this source code; if not, write to the Free Software Foundation,  
##     Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


import Constant
import Global
import Action

import hMath
import hTrack
import sFindBall
import sGrabWalk

DUMMY_PARAMETER = (Action.cmdParaWalk,Action.NormalWalkWT,10,0,0,20,70,100,10,10,40,10,-40,5,Action.HTAbs_h,0,0,0) 


########################################
# List of parameters which can be tuned
########################################

PARAM_NEAR_DISTANCE  = 38
PARAM_READY_DISTANCE = 14

PARAM_FAR_FORWARD    = 10   
PARAM_FAR_TURNCCW    = 40
PARAM_FAR_PG         = 40
PARAM_FAR_HDF        = 20
PARAM_FAR_HDB        = 20

PARAM_NEAR_FORWARD   = 6
PARAM_NEAR_TURNCCW   = 10
PARAM_NEAR_PG        = 28
PARAM_NEAR_HF        = 95
PARAM_NEAR_HB        = 115
PARAM_NEAR_HDF       = 13
PARAM_NEAR_HDB       = 10
PARAM_NEAR_FSO       = 15

PARAM_FIX_PG         = 20


PARAM_READY_FORWARD  = 6
PARAM_READY_PG       = 28
PARAM_READY_HF       = 95
PARAM_READY_HB       = 115
PARAM_READY_HDF      = 15
PARAM_READY_HDB      = 13
PARAM_READY_FFO      = 60
PARAM_READY_FSO      = 15
PARAM_READY_BFO      = -40
PARAM_READY_TILT     = -58 
PARAM_READY_CRANE    = 90
PARAM_READY_ANGLE    = 10


##########################################

g_PerformGenerator = None

def DecideNextAction():
    if g_PerformGenerator is None: 
        resetPerform()
    g_PerformGenerator.next()


def resetPerform():
    global g_PerformGenerator
    g_PerformGenerator = performGenerator()


def perform(): 
    if g_PerformGenerator is None: 
        resetPerform()
    return g_PerformGenerator.next()



def performGenerator():  
    
    current_branch = 0
    BRANCH_FAR = 1 
    BRANCH_NEAR = 2
    BRANCH_READY = 3
    
    while 1:
        
        Action.closeMouth()
                
        if Global.vBall.getConfidence() > 0:            

            ballD, ballH = Global.vBall.getDistance(), Global.vBall.getHeading()    
            ballH *= 0.8
            
            hTrack.trackVisualBall()
            
            # Adding hysterisis here..
            if ballD > PARAM_NEAR_DISTANCE\
                or (ballD > PARAM_NEAR_DISTANCE - 5 and current_branch == BRANCH_FAR):                

                current_branch = BRANCH_FAR
                    
                turnccw = hMath.CLIP(ballH,PARAM_FAR_TURNCCW)            
                Action.walk(PARAM_FAR_FORWARD,0,turnccw)
                Action.finalValues[Action.Pg]  = PARAM_FAR_PG
                Action.finalValues[Action.Hdf] = PARAM_FAR_HDF
                Action.finalValues[Action.Hdb] = PARAM_FAR_HDB 
                yield Constant.STATE_EXECUTING

            else:
                if ballD > PARAM_READY_DISTANCE: 
                    
                    current_branch = BRANCH_NEAR
                    
                    if abs(ballH) > PARAM_READY_ANGLE\
                        or abs(Global.desiredPan) > PARAM_READY_ANGLE: 

                        turnccw = hMath.CLIP(ballH,PARAM_NEAR_TURNCCW)
                        left = hMath.CLIP(-ballH,3)
                        Action.walk(1,left,turnccw) 
                        Action.finalValues[Action.Pg]  = PARAM_FIX_PG
                        Action.finalValues[Action.Hf]  = PARAM_NEAR_HF
                        Action.finalValues[Action.Hb]  = PARAM_NEAR_HB
                        Action.finalValues[Action.Hdf] = PARAM_NEAR_HDF
                        Action.finalValues[Action.Hdb] = PARAM_NEAR_HDB
                        yield Constant.STATE_EXECUTING         
                    
                    else: 
                        
                        turnccw = hMath.CLIP(ballH,PARAM_NEAR_TURNCCW)
                        Action.walk(PARAM_NEAR_FORWARD,0,turnccw) 
                        Action.finalValues[Action.Pg]  = PARAM_NEAR_PG
                        Action.finalValues[Action.Hf]  = PARAM_NEAR_HF
                        Action.finalValues[Action.Hb]  = PARAM_NEAR_HB
                        Action.finalValues[Action.Hdf] = PARAM_NEAR_HDF
                        Action.finalValues[Action.Hdb] = PARAM_NEAR_HDB
                        Action.finalValues[Action.Fso] = PARAM_NEAR_FSO
                        yield Constant.STATE_EXECUTING

                else:
                
                    current_branch = BRANCH_READY
                    
                    turnccw = hMath.CLIP(ballH,10)                               
                    for _ in range(15):
                        
                        Action.walk(PARAM_READY_FORWARD,0,turnccw) 
                        Action.finalValues[Action.Pg]       = PARAM_READY_PG
                        Action.finalValues[Action.Hf]       = PARAM_READY_HF
                        Action.finalValues[Action.Hb]       = PARAM_READY_HB
                        Action.finalValues[Action.Hdf]      = PARAM_READY_HDF
                        Action.finalValues[Action.Hdb]      = PARAM_READY_HDB
                        Action.finalValues[Action.Ffo]      = PARAM_READY_FFO
                        Action.finalValues[Action.Fso]      = PARAM_READY_FSO
                          
                        Action.finalValues[Action.HeadType] = Action.HTAbs_h
                        Action.finalValues[Action.Panx]     = 0
                        Action.finalValues[Action.Tilty]    = PARAM_READY_TILT
                        Action.finalValues[Action.Cranez]   = PARAM_READY_CRANE 
                        yield Constant.STATE_EXECUTING                        

                    yield Constant.STATE_SUCCESS
                        
        else:
            if Global.lostBall < 3: 
                hTrack.trackVisualBall()
            else:
                if Global.lostBall < 8: 
                    hTrack.trackGpsBall()
                else:
                    sFindBall.perform()
            
            yield Constant.STATE_EXECUTING



#############################################
# Variables and functions needed for learning
#############################################

g_ReadyGenerator = None

def ResetGetReady():
    global g_ReadyGenerator
    g_ReadyGenerator = None

def GetReady(): 
    global g_ReadyGenerator
    if g_ReadyGenerator is None: 
        g_ReadyGenerator = GetReadyGenerator()
    return g_ReadyGenerator.next()    
    
def GetReadyGenerator():

    selfx = Global.selfLoc.getX() 
    selfy = Global.selfLoc.getY()
    selfh = hMath.normalizeAngle_180(Global.selfLoc.getHeading()) 
    centerx = Constant.FIELD_WIDTH / 2
    centery = Constant.FIELD_LENGTH / 2
    heading = hMath.getHeadingBetween(selfx,selfy,centerx,centery)
    
    # 1st quad
    if selfx <= centerx and selfy <= centery: 
        turn = heading - selfh                    
    # 2nd quad
    elif selfx <= centerx and selfy >= centery:  
        turn = heading + selfh 
    # 3rd quad
    elif selfx >= centerx and selfy <= centery: 
        turn = - heading + selfh 
    # 4th quad
    else: 
        turn = - heading - selfh
    
    print "Turn Angle is " + str(turn) + "   Heading is " + str(heading)
                                       
    period = turn / 30 * 8                                   
                                        
    for _ in range(period):
        sGrabWalk.Perform(0,0,30) 
        yield Constant.STATE_EXECUTING
        
    for _ in range(15):
        Action.kick(Action.DiveKickWT)
        yield Constant.STATE_EXECUTING 
    
    Action.forceStepComplete()
    
    i = 0     
    while 1:
        i += 1
        if Global.vBall.getConfidence() > 0:
            # Wait at least one second before checking this.. 
            # So that the ball goes somewhere far
            if i > 75\
               and Global.vBall.getDistance() < 60\
               and Global.haveBall > 5: 
                break    
            turn = hMath.CLIP(Global.vBall.getHeading(),20) 
            Action.walk(3,0,turn)
            hTrack.trackVisualBall()
        else: 
            Action.walk(3,0,5)
            hTrack.spinningLocalise()             
        yield Constant.STATE_EXECUTING
    
    while 1:
        yield Constant.STATE_SUCCESS    

