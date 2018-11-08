#
#   Copyright 2005 The University of New South Wales (UNSW) and National  
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
#  $Id: Forward.py 4572 2004-12-15 01:28:59Z alexn $
# 
#  Copyright (c) 2005 UNSW
#  All Rights Reserved.
# 

#===============================================================================
#   Python Behaviours : 2005 (c) 
#
# Forward.py
#
# Our rUNSWift 2005 Forward Player!!
#
# edited 6/1/2005 to work with new behaviours structure
#
#===============================================================================

import Action
import Constant
import Debug
import Global
import hFWHead
import hTeam
import hTrack
import Indicator
#import sBirdOfPrey
import sFindBall
import sGrab
import sGreenBackOff
import VisionLink

# If True, an absence of wireless information will trigger
# a switch to the fWirelessDown formation, which fixes roles.
DETECT_WIRELESS_DOWN = True

# Frames to tolerate two dogs in one role        
SYMMETRY_BREAK_COUNT = 60

# Max ball distance at which to become attacker at the expense of
# an attacker who has lost the ball
VBALL_ATTACK_DIST = 100

#############################################
# imported formations must have the following
# function:
#   (new_role, params) = selectRole()
# which returns the new role to execute and
# a dictionary of parameters
# to pass to the role (or None for defaults).
# selectRole must set
# Global.myRole to an appropriate value.
#############################################
import fAttSupDef     #Attacker/Supporter/Defender
import fAttStrDef     #Attacker/Striker/Defender
import fAttSupWing    #Attacker/Supporter/Winger
import fWirelessDown  #Fixed Attacker/Attacker/Defender

#############################################
# these imported role modules should
# have the following interface functions:
#     perform(params = None)
##############################################
import rDistFindBall
import rPenalised
import rSetPlay
#import rAttacker
#import rDefender
#import rSupporter
#import rWinger
#import rStriker

# Role parameters specified by a formation
params = None

# STARTUP FORMATION
# -----------------
Global.formation = fAttSupDef      #Attacker/Supporter/Defender
#Global.formation = fAttStrDef      #Attacker/Striker/Defender
#Global.formation = fAttSupWing     #Attacker/Supporter/Winger

def DecideNextAction():
    VisionLink.startProfile("pForward.py")
    if Global.debugBehaviour_line:
        print "..... new frame: ", Global.frame
    
    #if Global.lostBall == 1 and Global.ballD < 60:# or Global.haveBall == 1:
    #if Global.haveBall == 1:
    #    VisionLink.sendYUVPlane()
    #if Global.haveGoal == 1: # or Global.lostGoal == 1:
    #if Global.lostGoal == 1:
    #    VisionLink.sendYUVPlane()

    #if Global.vBluePink.getConfidence() > 0\
    #    or Global.vPinkBlue.getConfidence() > 0\
    #    or Global.vYellowPink.getConfidence() > 0\
    #    or Global.vPinkYellow.getConfidence() > 0\
    #    or Global.vTGoal.getConfidence() > 0\
    #    or Global.vOGoal.getConfidence() > 0:
    #    VisionLink.sendYUVPlane()

    playGameMode()
    
    Indicator.superDebug()
       
    # Send wireless info to the teammates.
    hTeam.sendWirelessInfo()
    VisionLink.stopProfile("pForward.py")

def changeFormation(f):
    if f == "fAttSupDef":
        print "changeFormation: fAttSupDef"
        Global.formation = fAttSupDef
    elif f == "fAttStrDef":
        print "changeFormation: fAttStrDef"
        Global.formation = fAttStrDef
    elif f == "fAttSupWing":
        print "changeFormation: fAttSupWing"
        Global.formation = fAttSupWing
    else:
        print "changeFormation: UNKNOWN FORMATION"
    
#--------------------------------------
# General Initialisation
def resetEachFrame():

    # update my previous assigned role. 
    Global.myLastRole = Global.myRole
    
    #if Global.state != Constant.FINISHEDSTATE:
    #    Global.myRole  = Constant.NOROLE
         
    Action.standStill()
    Indicator.setDefault()
    hFWHead.resetEachFrame()

######################################################################
######################################################################

def playGameMode():
#    global newAttackerBonus
    global params

    resetEachFrame() 

    ##---------------------------------------------------------
    ## Decision making part...    
    specialPlayer = Global.penaltyShot or Global.lightingChallenge

    ### First select any role that is necessary regardless of formation
    if Global.penalised and not Global.lightingChallenge:
        #print "I'm penalised"
        new_role = rPenalised 
        Global.myRole = Constant.PENALISED          

    # Keep executing any set play that is running
    elif (not specialPlayer) and rSetPlay.perform():
        #print "performing a set play"
        return

    # Distributed findball if no-one knows where it is
    elif shouldWeDoDistributedFindBall(): 
        #print "doing distfindball"
        new_role = rDistFindBall
        #Global.myRole = Constant.NOROLE

    # If there is no wireless information use the wireless-down formation
    elif DETECT_WIRELESS_DOWN and hTeam.wirelessIsDown() and not specialPlayer:
        #print "Wireless is down"
        (new_role, params) = fWirelessDown.selectRole()
            
    ### Else the formation selects the role
    else:    
        (new_role, params) = Global.formation.selectRole()

    # If I was assigned to the same role, then increment the role counter
    if Global.myLastRole == Global.myRole:
        if Global.myRoleCounter < 255: 
            Global.myRoleCounter += 1
    else: 
        Global.myRoleCounter = 0        

    #Action.standStill()
    new_role.perform(params)            
    
    # Activate green back off, if we need to. 
    if Global.myRole != Constant.PENALISED\
        and Global.myRole != Constant.ATTACKER\
        and not Global.lightingChallenge:
                
        if sGreenBackOff.perform() == Constant.STATE_EXECUTING:
            hFWHead.compulsoryAction = hFWHead.doNothing
            Action.setHeadParams(0,0,0,Action.HTAbs_h)
            Indicator.showFacePattern([1,1,3,1,1]) 
    
    # If the ball is in our own goalbox and I am also in the goalbox, 
    # then don't move forward or left. 
    # This is to avoid illegal defender.
    # Alex: this is crap
    #if Global.myRole == Constant.ATTACKER: 
    #    if Global.vBall.isVisible()\
    #        and hWhere.ballInOwnGoalBox()\
    #        and hWhere.isInOwnGoalBox(30)\
    #        and Global.selfLoc.getPosVar() < hMath.get95var(30):
    #        
    #        Action.finalValues[Action.Forward] = 0
    #        Action.finalValues[Action.Left] = 0            
              

            
    # ---------------------------------------------------------
    # Debugging part...
    if Debug.roleChange:
        if Global.myRole == Constant.DEFENDER: 
            print "my role is defender @ " + str(Global.frame)
        elif Global.myRole == Constant.WINGER: 
            print "my role is winger @ " + str(Global.frame)
        elif Global.myRole == Constant.SUPPORTER: 
            print "my role is supporter @ " + str(Global.frame)
        elif Global.myRole == Constant.ATTACKER: 
            print "my role is attacker @ " + str(Global.frame)
        elif Global.myRole == Constant.STRIKER: 
            print "my role is striker @ " + str(Global.frame)            
        else:
            print "my role is UNKNOWN @ " + str(Global.frame)


def shouldWeDoDistributedFindBall(): 
    # Special case for wireless down strategy
    if hTeam.wirelessIsDown() and (Global.lostBall < Constant.LOST_BALL_SPIN \
                                or Global.myLastRole != Constant.ATTACKER):
        return False

    # If no one sees a ball for a long time, then do distributed ball 
    # search.
    # One robot searches in upper field.
    #  attackers (finding a ball) and one defender (back up).
    if hTeam.haveAllTeammatesLostBall(): 
        if Global.myLastRole == Constant.ATTACKER\
            and Global.lostBall > Constant.LOST_BALL_SPIN:
            return True       
        elif Global.myLastRole != Constant.ATTACKER\
            and Global.lostBall > Constant.LOST_BALL_SPIN - 100:
            return True 
        
    return False              


####################################################################
# Standard version of shouldIBe<Role> are here to promote code reuse,
# but formations are free to define and use their own if these are
# not suitable. These versions assume a priority of Attacker, Supporter,
# Winger/Defender

def shouldIBeAttacker(): 
    
    # If there is no other attacker then be an attacker.
    numAttacker = hTeam.countAttacker()
    if numAttacker == 0:
        #print "Criteria A"
        #print "Global.otherValidTeammates : ", Global.otherValidTeammates
        #print "no other attackers"
        return True            

    # There is at least one other attacker, but if they can't see the ball
    # and I can and I'm close to the ball then be attacker
    numAttacker = hTeam.countAttacker(True)
    if numAttacker == 0 and Global.lostBall < Constant.LOST_BALL_LAST_VISUAL \
            and Global.ballD < VBALL_ATTACK_DIST:
        
        #print "Criteria B"
        #print "Global.lostBall < Constant.LOST_BALL_LAST_VISUAL ", (Global.lostBall < Constant.LOST_BALL_LAST_VISUAL)
        #print "numAttacker == 0 ", (numAttacker == 0)
        #print "Global.ballD < VBALL_ATTACK_DIST ", (Global.ballD < VBALL_ATTACK_DIST)        
        #print "Global.otherValidTeammates : ", Global.otherValidTeammates
        return True
        
        
    # If I am grabbing a ball, then continue as an attacker.    
    if sGrab.isGrabbed\
        or sGrab.grabbingCount > 0\
        or sFindBall.shouldIForce():
        
        #print "Criteria C"
        #print "sGrab.isGrabbed ", sGrab.isGrabbed
        #print "sGrab.grabbingCount ", sGrab.grabbingCount
        #print "sFindBall.shouldIForce() ", sFindBall.shouldIForce()
        #print "Global.otherValidTeammates : ", Global.otherValidTeammates
        return True                    
                         
    
    # If my estimate time to reach ball is the smallest among all the
    # forwards, then be an attacker.
    myEstimateTime = hTrack.timeToReachBall()
    for i in Global.otherValidForwards: 
        mate = Global.teamPlayers[i]
            
        mateEstimateTime = mate.getTimeToReachBall()            
        if myEstimateTime > mateEstimateTime\
            and myEstimateTime > 1500:
            break                
    else:
        # When this else branch is reached, it means I can be an 
        # attacker. But before that, I should check if I should 
        # break symmetry when there are more than 1 attacker on the 
        # field.
        if Global.myLastRole == Constant.ATTACKER\
            and Global.myRoleCounter > SYMMETRY_BREAK_COUNT:

            for i in Global.otherValidForwards: 
                mate = Global.teamPlayers[i]
                if mate.isAttacker()\
                    and mate.getRoleCounter() > SYMMETRY_BREAK_COUNT:
                    
                    if shouldIBreakAttackerSymmetry(i):
                        return False
            else:
                #print "Criteria D"
                #print "Global.otherValidTeammates : ", Global.otherValidTeammates
                return True
                                
        else:            
            #print "Criteria E"
            #print "Global.otherValidTeammates : ", Global.otherValidTeammates
            return True
         
    return False      
    

def shouldIBeSupporter():

    # If there is an attacker and no supporter, then be a supporter.
    numAttacker = hTeam.countAttacker()
    numSupporter = hTeam.countSupporter()
    if numAttacker == 1 and numSupporter == 0: 
        return True
    # If there is no attacker and a supporter, then be a supporter.
    if numAttacker == 0 and numSupporter == 1:
        return True
        

    # If my estimate time to reach supporter pos is the smallest among 
    # all the forwards except an attacker.
    selfLoc = Global.selfLoc
    myEstimateTime = hTrack.timeToReachSupporterPos(selfLoc.getX(),\
                                                 selfLoc.getY(),\
                                                 selfLoc.getHeading(),\
                                                 Global.myLastRole == Constant.SUPPORTER)
    for i in Global.otherValidForwards: 
        mate = Global.teamPlayers[i]
        if not mate.isAttacker(): 

            mateLoc = Global.teammatesLoc[i]
            mateEstimateTime = hTrack.timeToReachSupporterPos(mateLoc.getX(),\
                                                       mateLoc.getY(),\
                                                       mateLoc.getHeading(),\
                                                       mate.isSupporter())
            if myEstimateTime > mateEstimateTime:
                break      
    else:
        # When this else branch is reached, it means I can be a 
        # supporter. But before that, I should check if I should 
        # break symmetry when there are more than 1 supporter on the 
        # field.
        if Global.myLastRole == Constant.SUPPORTER\
            and Global.myRoleCounter > SYMMETRY_BREAK_COUNT:

            for i in Global.otherValidForwards: 
                mate = Global.teamPlayers[i]
                if mate.isSupporter()\
                    and mate.getRoleCounter() > SYMMETRY_BREAK_COUNT:
                    
                    if shouldIBreakSupporterSymmetry(i):
                        return False 
            else:
                return True
                                
        else:            
            return True        
    
    return False

def shouldIBeStriker():
    # If there are 2 or less forwards alive, it is better that 
    # non-attacker be a defender.
    if hTeam.forwardAlive() <= 2: 
        return False

    # If there is an attacker and no striker, then be a striker.
    numAttacker = hTeam.countAttacker()
    numStriker = hTeam.countStriker()
    if numAttacker == 1 and numStriker == 0: 
        return True
    # If there is no attacker and a striker, then be a striker.
    if numAttacker == 0 and numStriker == 1:
        return True
        

    # If my estimate time to reach striker pos is the smallest among 
    # all the forwards except an attacker.
    selfLoc = Global.selfLoc
    myEstimateTime = hTrack.timeToReachStrikerPos(selfLoc.getX(),\
                                                 selfLoc.getY(),\
                                                 selfLoc.getHeading(),\
                                                 Global.myLastRole == Constant.STRIKER)
    for i in Global.otherValidForwards: 
        mate = Global.teamPlayers[i]
        if not mate.isAttacker(): 

            mateLoc = Global.teammatesLoc[i]
            mateEstimateTime = hTrack.timeToReachStrikerPos(mateLoc.getX(),\
                                                       mateLoc.getY(),\
                                                       mateLoc.getHeading(),\
                                                       mate.isStriker())
            if myEstimateTime > mateEstimateTime:
                break      
    else:
        # When this else branch is reached, it means I can be a 
        # striker. But before that, I should check if I should 
        # break symmetry when there are more than 1 striker on the 
        # field.
        if Global.myLastRole == Constant.STRIKER\
            and Global.myRoleCounter > SYMMETRY_BREAK_COUNT:

            for i in Global.otherValidForwards: 
                mate = Global.teamPlayers[i]
                if mate.isStriker()\
                    and mate.getRoleCounter() > SYMMETRY_BREAK_COUNT:
                    
                    if shouldIBreakStrikerSymmetry(i):
                        return False 
            else:
                return True
                                
        else:            
            return True        
    
    return False
    
def shouldIBeWinger(): 
    return True    

def shouldIBeDefender(): 
    return True     

# This function returns True, if I should not be an attacker anymore.
def shouldIBreakAttackerSymmetry(mateI):                   
    return False
    #return mateI < Global.myPlayerNum - 1

# This function returns True, if I should not be a supporter anymore.
def shouldIBreakSupporterSymmetry(mateI):

    mate = Global.teamPlayers[mateI]
    
    # Force break, if no break has happened for a while    
    if Global.myRoleCounter > 2 * SYMMETRY_BREAK_COUNT\
        and mate.getRoleCounter() > 2 * SYMMETRY_BREAK_COUNT:
        return mateI < Global.myPlayerNum - 1
 
    selfLoc = Global.selfLoc
    mateLoc = Global.teammatesLoc[mateI]

    if selfLoc.getY() < mateLoc.getY():
        return True                             
    else:         
        return False 
    
# This function returns True, if I should not be a striker anymore.
def shouldIBreakStrikerSymmetry(mateI):

    mate = Global.teamPlayers[mateI]
    
    # Force break, if no break has happened for a while    
    if Global.myRoleCounter > 2 * SYMMETRY_BREAK_COUNT\
        and mate.getRoleCounter() > 2 * SYMMETRY_BREAK_COUNT:
        return mateI < Global.myPlayerNum - 1
 
    selfLoc = Global.selfLoc
    mateLoc = Global.teammatesLoc[mateI]

    if selfLoc.getY() < mateLoc.getY():
        return True                             
    else:         
        return False         
