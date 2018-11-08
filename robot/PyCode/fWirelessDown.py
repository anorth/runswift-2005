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


# Wireless down formation
# We have two attackers and one defender. Roles are determined statically
# by player numbers. It is assumed that communication is impossible.

import Constant
import Global
#import hTeam
#import hTrack

#import pForward
import rAttacker
#import rSupporter
import rDefender

# Returns the role to execute
def selectRole():
    if Global.myPlayerNum == 2 or Global.myPlayerNum == 3:   
        new_role = rAttacker 
        Global.myRole = Constant.ATTACKER             
    
    elif Global.myPlayerNum == 4: 
        new_role = rDefender
        Global.myRole = Constant.DEFENDER
          
    else: 
        print "Warning: formation failed to select a role, using rAttacker"
        new_role = rAttacker
        Global.myRole = Constant.ATTACKER

    return (new_role, None)
