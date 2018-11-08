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


# Attacker-supporter-defender formation
# Attacker/supporter are as usual. The defender stays in the defending half
# of the field. This uses the default role selection from pForward.

import Constant
import Global
import hTeam
import hTrack

import pForward
import rAttacker
import rSupporter
import rDefender

# Returns the role to execute
def selectRole():
    if pForward.shouldIBeAttacker():   
        #if Global.myRole != Constant.ATTACKER:
        #    newAttackerBonus = NEW_ATTACKER_BONUS
        new_role = rAttacker 
        Global.myRole = Constant.ATTACKER             
    
    elif shouldIBeSupporter():     # custom shouldIBeSupporter
        new_role = rSupporter   
        Global.myRole = Constant.SUPPORTER     
    
    elif pForward.shouldIBeDefender(): 
        new_role = rDefender
        Global.myRole = Constant.DEFENDER
          
    else: 
        print "Warning: formation failed to select a role, using rAttacker"
        new_role = rAttacker
        Global.myRole = Constant.ATTACKER

    return (new_role, None)

# Determines who should be supporter based on which non-attacking robot is 
# farthest from defender position.
def shouldIBeSupporter():
    # If there are 2 or less forwards alive, it is better that 
    # non-attacker to be a defender.
    if hTeam.forwardAlive() <= 2: 
        return False
        
    # If there is an attacker and no supporter, then be a supporter.
    numAttacker = hTeam.countAttacker()
    numSupporter = hTeam.countSupporter()
    if numAttacker == 1 and numSupporter == 0: 
        return True
    # If there is no attacker and a supporter, then be a supporter.
    if numAttacker == 0 and numSupporter == 1:
        return True
    
    
    # If you are the furthest back in the field, then be a defender instead.
    if hTeam.amIFurthestBack(30):
        return False    

    # If my estimate time to reach defender pos is the largest among 
    # all the forwards except an attacker.
    selfLoc = Global.selfLoc
    myEstimateTime = hTrack.timeToReachDefenderPos(selfLoc.getX(),\
                                         selfLoc.getY(),\
                                         selfLoc.getHeading(),\
                                         Global.myLastRole == Constant.DEFENDER)
    for i in Global.otherValidForwards: 
        mate = Global.teamPlayers[i]
        if not mate.isAttacker(): 

            mateLoc = Global.teammatesLoc[i]
            mateEstimateTime = hTrack.timeToReachDefenderPos(mateLoc.getX(),\
                                                       mateLoc.getY(),\
                                                       mateLoc.getHeading(),\
                                                       mate.isDefender())
            
            # hysterisis introduced for furthest back.                                           
            if myEstimateTime < mateEstimateTime and hTeam.amIFurthestBack(0):
                break # It returns False    
    else:
        # When this else branch is reached, it means I can be a 
        # supporter. But before that, I should check if I should 
        # break symmetry when there are more than 1 supporter on the 
        # field.
        if Global.myLastRole == Constant.SUPPORTER\
            and Global.myRoleCounter > pForward.SYMMETRY_BREAK_COUNT:

            for i in Global.otherValidForwards: 
                mate = Global.teamPlayers[i]
                if mate.isSupporter()\
                    and mate.getRoleCounter() > pForward.SYMMETRY_BREAK_COUNT:
                    
                    if pForward.shouldIBreakSupporterSymmetry(i):
                        return False 
            else:
                return True
                                
        else:            
            return True        
    
    return False

