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


################################################################################
# Team/Game related communications / functions
################################################################################


import Constant
import Debug
import Global
#import hMath
import hTrack
#import sFindBall
import sGrab
import VisionLink

# Return true if I am the furthest back (except ignoreRoles) by at least dist
def amIFurthestBack(dist=30,ignoreRoles=[Constant.ATTACKER]): 
    myy = Global.selfLoc.getY()
    for i in Global.otherValidForwards:
        mate = Global.teamPlayers[i]
        if not mate.getRole() in ignoreRoles\
            and Global.teammatesLoc[i].getY() < myy + dist:            
            return False
    
    return True

    
    
#--------------------------------------
# Returns true iff 'hasGrabbed ball' signal received from another teammate.
def hasTeammateGrabbedBall():
    for i in Global.otherValidForwards:
        mate = Global.teamPlayers[i]
        if mate.hasGrabbedBall():            
            return True
            
    return False

def hasTeammateSeenBall(): 
    for i in Global.otherValidForwards: 
        mate = Global.teamPlayers[i] 
        if mate.hasSeenBall(): 
            return True
    return False

def haveAllTeammatesLostBall(): 
    for i in Global.otherValidForwards: 
        mate = Global.teamPlayers[i]
        if not mate.hasLostBall():
            return False
                
    return True               
        

##----------------------------------
def getIndexOfClosestTeammateToBall():
    mateI = None
    mateD = None     
    for i in Global.otherValidForwards:
        mate = Global.teamPlayers[i]
        
        if mate.hasSeenBall():
            if mateD == None or mate.getBallDist() < mateD:        
                mateI = i
                mateD = mate.getBallDist()  
    
    return mateI 
    

##----------------------------------------------------
## Returns true if your mate is close to the ball.
def IsTeammateCloseToBall(): 
    BALL_DISTANCE = 50
    for i in Global.otherValidForwards: 
        mate = Global.teamPlayers[i]

        if mate.hasGrabbedBall(): 
            return True
        
        if mate.hasSeenBall()\
            and mate.getDistance() < BALL_DISTANCE: 
            return True

    return False

#--------------------------------------
# Returns True if we have received no wireless info recently
def wirelessIsDown():   
#    if robotAlive() == 1 and not VisionLink.gameDataIsValid():
#        print "wireless down - robots =", robotAlive(),\
#            "gamedatavalid =", VisionLink.gameDataIsValid()
    return robotAlive() == 1 and (not VisionLink.gameDataIsValid())

#--------------------------------------
# Return number of robots alive including yourself.
def robotAlive():

    # me plus others
    return 1 + len(Global.otherValidTeammates)
            

#--------------------------------------
def forwardAlive():
    
    # me plus others - not true for goalie..
    return 1 + len(Global.otherValidForwards)

### Count players filling various roles. If you add a new role please
# add a new function

# If onlyWithBall then only attackers who have recently seen the ball count
def countAttacker(onlyWithBall = False):     
    if onlyWithBall:
        return Global.attackerWithBallCount
    else:
        return Global.attackerCount


def countDefender(): 
    num = 0
    for i in Global.otherValidForwards:
        mate = Global.teamPlayers[i]
        
        if mate.isDefender():
            num = num + 1
            
    return num   
    
def countWinger(): 
    num = 0
    for i in Global.otherValidForwards:
        mate = Global.teamPlayers[i]
    
        if mate.isWinger():
            
            num += 1
            
    return num 
    
def countSupporter(): 
    num = 0
    for i in Global.otherValidForwards:
        mate = Global.teamPlayers[i]
        
        if mate.isSupporter():
            
            num += 1      
            
    return num   
    
def countStriker(): 
    num = 0
    for i in Global.otherValidForwards:
        mate = Global.teamPlayers[i]
        
        if mate.isStriker():
            num += 1      
            
    return num       
    
def countBackOff(): 
    num = 0
    for i in Global.otherValidForwards: 
        mate = Global.teamPlayers[i]
        if mate.isBackOff(): 
            num += 1
    
    return num      
    
def sendMyBehaviourInfo(timeToReachBallOrKickOff=0,\
                        hasGrabbedBall=False,\
                        hasSeenBall=False,\
                        hasLostBall=False,\
                        myRole=Global.myRole,\
                        myRoleCounter=0):
    
    VisionLink.sendMyBehaviourInfo(timeToReachBallOrKickOff,\
                                   hasGrabbedBall,\
                                   hasSeenBall,\
                                   hasLostBall,\
                                   myRole,\
                                   myRoleCounter)
    

 
                                   
#--------------------------------------
# Send wireless info to teammates. The 2003 team didn't group the sending
# together, and the code became so messy.
#
# However by grouping them, the assigning may have some problems. But two or
# more could be set at the same time. Come back later.
lostBallForLong = False
def sendWirelessInfo():
    global lostBallForLong
    
    timeToReachBallOrKickOff = Constant.MAX_TIME_TO_REACH
    hasGrabbedBall,\
    hasSeenBall,\
    hasLostBall = [False,False,False]
    
     
    #if not (Global.ballSource == Constant.GPS_BALL and Global.lostBall < Constant.LOST_BALL_GPS):
    timeToReachBallOrKickOff = hTrack.timeToReachBall() 

        
    # If in ready state, then send the kickoff signal.
    # Kim and Daniel both agreed, so at the moment ballDist in the readyState is
    # really not a ball distance, it is the kickoff signal. Reason is the
    # kickOff signal in interpBehav is a boolean with two values, but kickoff
    # signal need 5 values. However any other state (eg: set), the ballDist is
    # really the ball distance. Notice that in the readyState robots don't need
    # to send ball distance to other robots as they are not even tracking the
    # ball.
    if Global.state == Constant.READYSTATE or Debug.mustReady:
        timeToReachBallOrKickOff = Global.kickOffState
        
            
    # Should I tell my teammates that I have the ball?
    hasGrabbedBall = sGrab.isGrabbed    
    
    if Global.vBall.getConfidence() > 0\
        or Global.lostBall < 5:
        hasSeenBall = True
    
    # When I lost the ball for too long, tell everyone that I lost the 
    # ball for a long time. If I see a ball, I will still 
    # tell everyone that I lost the ball, because I might be seeing a 
    # wrong ball.    
    if Global.lostBall >= Constant.LOST_BALL_SCAN:
        lostBallForLong = True
        hasLostBall = True
        
    elif lostBallForLong\
        and Global.haveBall <= 3:
        hasLostBall = True
    else:
        lostBallForLong = False   

    sendMyBehaviourInfo(timeToReachBallOrKickOff,\
                        hasGrabbedBall,\
                        hasSeenBall,\
                        hasLostBall,\
                        Global.myRole,\
                        Global.myRoleCounter)
                                   

# Broadcasts a TeamTalk message on wireless. Recipient should be a dog ID
# number (1-4), or zero for all teammates.
def sendTeamTalk(message, recipient):
    # The header info is stripped by Behaviou.py
    message = "tt" + str(Global.myPlayerNum) + str(recipient) + message
    VisionLink.sendTeamTalk(message)
