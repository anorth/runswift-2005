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


import hTrack
import VisionLink
import Action
import Global
NEXTANGLE = 50
PAUSE_FRAME = 5
counter = 0

def DecideNextAction():
    global counter
    if counter == 0:
        print "counter:",counter
        hTrack.efficientScan(False, 120,int(NEXTANGLE*80/90),80, -80, 999)
        counter += 1
        return
        #Action.setHeadParams(20,0,0,Action.HTAbs_h)
    elif counter == 30:
        VisionLink.sendYUVPlane()
    #Total time = 180/JUMPING_ANGLE+PAUSE_FRAME
    elif counter > PAUSE_FRAME:        
        counter = -1
    counter +=1
    Action.setHeadParams(Global.desiredPan, Global.desiredTilt,Global.desiredCrane,Action.HTAbs_h) 