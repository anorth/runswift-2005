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
#  $Id: sActiveLocalise.py 4572 2004-12-15 01:28:59Z alexn $
# 
#  Copyright (c) 2004 UNSW
#  All Rights Reserved.
# 




#===============================================================================
#   Python Behaviours : 2004 (c) 
#
# sActiveLocalise.py
#
# Implements smart active localising based on self covariance
#
#===============================================================================

import math
import Global
import Constant

import hMath
import Action
#import Indicator
import VisionLink

#==============================================================================
# Global variables of this module.
#==============================================================================

localiseBeacon  = None  # Which beacon active localise should look at.
                        # Either set this yourself or use SmartSetBeacon.
lastBeacon      = None  # The one we used last time
beaconAlternate = 0     # The beacon (except the beaconBestReduceVar)
                        # observable in least pan, as an alternate choice to
                        # beaconBestReduceVar.
isFocusing = False

myaGoingTo = Constant.dCLOCKWISE

# Closer than this distance we look at the coloured bit of the beacon rather
# than the base. (We look at the base otherwise to keep more of the camera
# frame on the field for lines, ball etc)
BEACON_LOOK_UP_DIST = 120

# We prefer beacons outside this distance since closer than this we don't
# get reliable information (beacon is too big)
MIN_BEACON_DIST = 70

#===============================================================================
# Functions that belong to this module
#===============================================================================



#--------------------------------------
# Makes the dog look at the beacon designated by localiseBeacon
def DecideNextAction(bShowLED = True):
    return perform(bShowLED)


# This function returns Constant.STATE_FAILED, if we can't active localise.
def perform(bShowLED=True): 
    global localiseBeacon, isFocusing

    # print "Active-localising!!"

    # Set debugging information.
    if bShowLED:
        pass
        #Indicator.showRedEye(True)

    if localiseBeacon == None:
        print "sActiveLocalise.perform: localiseBeacon None"
        Action.setHeadToLastPoint()
        return Constant.STATE_FAILED
    
    #print "sActiveLocalise.DecideNextAction() : Actively Localising at beacon: ", localiseBeacon

    relPos      = map(lambda b,s:b-s, localiseBeacon, Global.selfLoc.getPos())
    relPos      = hMath.rotate(relPos, 90 - Global.selfLoc.getHeading())
    targetX     = -relPos[0]
    targetY     = relPos[1]

    # We don't need to put the centre of the camera on the beacon since that
    # wastes at least half the image. Instead we look at the base of the beacon
    # if it is far enough away for the whole thing still to be in frame when we
    # do this. (The distance was calibrated from measurement)
    if hMath.getDistSquaredBetween(0, 0, targetX, targetY) <= hMath.SQUARE(BEACON_LOOK_UP_DIST):
        Action.setHeadParams(targetX, Constant.BEACON_HEIGHT, targetY,\
                                Action.HTAbs_xyz)
    else:
        Action.setHeadParams(targetX, 0, targetY, Action.HTAbs_xyz)
    
    return Constant.STATE_EXECUTING


def getBeaconCoords():
    coords = VisionLink.getGPSCoordArray()
    return [coords[2], coords[3], coords[4], coords[5]]
            #far left, far right, close left, close right


#--------------------------------------
# Sets which beacon the dog will look at based on self covariance. panLimit
# and distLimit specify which beacons should be considered. If you want to
# consider all beacons the dog could see from its current field position, use
# the defaults. Note that this may set localiseBeacon to None
# if there are no beacons that can be seen.
#
# This function returns True, if we can see beacons from our current heading.
# Otherwise, returns False,
def SmartSetBeacon(panLimit = 90, distLimit = 550):
    global localiseBeacon, lastBeacon

    # Get shared variables from elsewhere.
    selfLoc = Global.selfLoc
    cov     = VisionLink.getGPSSelfCovariance()
    
    # Calculate the direction of the larger axis of the self covariance
    b = -(cov[0][0] + cov[1][1])
    c = cov[0][0] * cov[1][1] - cov[1][0] * cov[1][0]
    det = b * b - 4 * c
    
    if (det < 0.00001):
        sol = -b / 2
    else:
        sol = (-b + math.sqrt(det)) / 2
    
#    chooseCov = False   # Are we choosing a beacon to reduce covariance?
    if abs(cov[0][0] - sol) < 0.01 and cov[0][1] < 0.01:
        # If self position variance is circular, all beacons are equally good
        # to localise off, and so choose the one closest to current heading.
        # dHead is global heading we desire to stay close to
        if abs(cov[1][1] - sol) < 0.01:
            dHead = selfLoc.getHeading()
        else:
            dHead = hMath.RAD2DEG(math.atan2(-cov[1][0], cov[1][1] - sol))
            
    else:
        #chooseCov = True
        dHead = hMath.RAD2DEG(math.atan2(sol - cov[0][0], cov[0][1]))
            
    # Find the beacon that is closest to the variance direction.
    beaconPos = getBeaconCoords()
    bestHead = nextBestHead = 360
    localiseBeacon = None   # a tuple (x, y)
    nextBestBeacon = None

    #print beaconPos
    
    for beacon in beaconPos:
        relPos = map(lambda b,s:b-s, beacon, selfLoc.getPos())
        beaconHead = hMath.RAD2DEG(math.atan2(relPos[1], relPos[0]))
        beaconDist = hMath.getLength(relPos)
        localHead  = hMath.normalizeAngle_180(beaconHead - selfLoc.getHeading())

        if abs(localHead) < panLimit and beaconDist < distLimit:
            #print beacon, localHead, beaconHead, dHead
            compHead = abs(hMath.normalizeAngle_180(dHead - beaconHead))
            if compHead > 90:
                compHead = 180 - compHead
            
            if compHead < bestHead:
                nextBestBeacon = localiseBeacon
                nextBestHead = bestHead
                localiseBeacon = beacon
                bestHead  = compHead
            elif compHead < nextBestHead:
                nextBestBeacon = beacon
                nextBestHead = compHead
                

    #print "sActiveLocalise best =", localiseBeacon, "next =", nextBestBeacon

    # Don't choose the same beacon twice to reduce covariance. If we chose it
    # last time based on covariance and the covariance is still large that way
    # we likely didn't see it, so choose the next best
    if localiseBeacon == lastBeacon and nextBestBeacon != None:
        # print "Choosing a different beacon to last time"
        localiseBeacon = nextBestBeacon
        nextBestBeacon = None

    # Prefer not to look at a really close beacon - we don't get good
    # information
    if nextBestBeacon != None and \
        hMath.getDistanceBetween(selfLoc.getX(), selfLoc.getY(),\
                localiseBeacon[0], localiseBeacon[1]) < MIN_BEACON_DIST:
        # print "Choosing not to use close beacon"
        localiseBeacon = nextBestBeacon

    
    #print "Choosing beacon at", localiseBeacon, "frame", Global.frame
    lastBeacon = localiseBeacon

    if localiseBeacon == None: 
        return False 
    else:
        return True    


#--------------------------------------
# Sets the beacon the dog should look at to the one with smallest pan from
# current heading
def AlternateSetBeacon(panLimit = 90, distLimit = 550):
    global localiseBeacon, lastBeacon

    selfLoc = Global.selfLoc
    beaconPos = getBeaconCoords()
    curCompHead = panLimit
    localiseBeacon = None    
    
    for beacon in beaconPos:
        relPos = map(lambda b,s:b-s, beacon, selfLoc.getPos())
        beaconHead = hMath.RAD2DEG(math.atan2(relPos[1], relPos[0]))
        beaconDist = hMath.getLength(relPos)
        localHead  = hMath.normalizeAngle_180(beaconHead - selfLoc.getHeading())
    
        if abs(localHead) < curCompHead and beaconDist < distLimit:
            curCompHead  = localHead
            localiseBeacon = beacon

    lastBeacon = localiseBeacon
