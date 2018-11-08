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



"""
import Constant
import Global
import hMath
import hWhere
import sGrabTurnKick
import sWTrack
import Indicator
import Action

EDGE_OFFSET = 40

ballAtEdgeCount = 0
lockMode = False
"""

"""
DEBUG_ALONE = False
def DecideNextAction():
    if not DEBUG_ALONE:
        edgerAction()
    else: # if DEBUG_ALONE == True
        if shouldIBecomesEdger():
            edgerAction()
        else:
            sWTrack.DecideNextAction()
            Indicator.showHeadColor((False, False, False))


def edgerAction():
    global lockMode
    # if in target half field, but not at the top edge, be more aggressive
    # (should be Constant.FIELD_WIDTH*0.5, but the localisation is stuffed. #%$&%@ -- weiming)
    
    if sGrabTurnKick.isGrabbed():
        Global.lostBall = hMath.DECREMENT(Global.lostBall)
        sGrabTurnKick.DecideNextAction()
    
    else:
        selfX, selfY = Global.selfLoc.getX(), Global.selfLoc.getY()

        if selfY > Constant.FIELD_LENGTH * 0.66\
            and not hWhere.ballOnTEdge(EDGE_OFFSET):

            sGrabTurnKick.perform()

        elif selfY > Constant.FIELD_LENGTH * 0.3\
            and not hWhere.ballOnTEdge(EDGE_OFFSET): 

            if selfX > Constant.FIELD_WIDTH / 2:
                sGrabTurnKick.perform(False,\
                                               Action.ChestPushWT,\
                                               2 * Constant.FIELD_WIDTH / 3,\
                                               Constant.FIELD_LENGTH - Constant.GOALBOX_DEPTH - 10)
            else:
                sGrabTurnKick.perform(False,\
                                               Action.ChestPushWT,\
                                               Constant.FIELD_WIDTH / 3,\
                                               Constant.FIELD_LENGTH - Constant.GOALBOX_DEPTH - 10)   
        else:

            if selfX > Constant.FIELD_WIDTH / 2:
                sGrabTurnKick.perform(False,\
                                               Action.DiveKickWT,\
                                               Constant.FIELD_WIDTH - 50,\
                                               Constant.FIELD_LENGTH - Constant.GOALBOX_DEPTH - 10)
            else: 
                sGrabTurnKick.perform(False,\
                                               Action.DiveKickWT,\
                                               50,\
                                               Constant.FIELD_LENGTH - Constant.GOALBOX_DEPTH - 10)

    lockMode = sGrabTurnKick.isGrabbed()




def shouldIBecomesEdger():
    global ballAtEdgeCount, lockMode

    if lockMode:
        return True

    # if our teammate has the ball, return false
    for i in range(Constant.NUM_TEAM_MEMBER):
        if (Global.teamPlayers[i].getCounter() > 0 
            and i != Global.myPlayerNum - 1
            and Global.teamPlayers[i].hasBall()):
            print "sEdger: our teammate has the ball!!"
            reset()
            return False

    ballAtEdge = not hWhere.ballNotAlongEdge(EDGE_OFFSET)
    #print Global.ballX, "  ", Global.ballY, "  ball at edge?", ballAtEdge

    # if ball NOT at the edges
    # or in the goal box
    # or we have lost the ball for 2 seconds
    if not ballAtEdge \
           or hWhere.inTargetGoalBox(Global.ballX, Global.ballY) \
           or Global.lostBall > 60 \
           or (Global.vTGoal.getConfidence() > 0 and Global.vTGoal.getDistance() < 100):
        reset()
        return False # do not become edger
    
    if ballAtEdge and sWTrack.canSeeBall():
        ballAtEdgeCount += 1

    return ballAtEdgeCount >= 5


def reset():
    global ballAtEdgeCount, lockMode
    ballAtEdgeCount = 0
    lockMode = False
"""
