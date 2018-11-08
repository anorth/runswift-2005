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
#  $Id: Behaviou.py 4572 2004-12-15 01:28:59Z alexn $
# 
#  Copyright (c) 2005 UNSW
#  All Rights Reserved.
# 




#===============================================================================
#   Python Behaviours : 2005 (c) 
#
# Behaviou.py
#
# This is the top level module in our python behaviour program.
# processFrame() got called first and it will call the "DecideNextAction()" of
# relevant player.
#
#==============================================================================

# In python, each code file is called a module. Python programs work by having 
# one and only one top-level module, which executes the "main function" and 
# import other modules to carry out more complex tasks.

import Action
import Constant
import coverage
import Debug # everything you need for debugging
import Global
import hTeam
import hFrameReset
import Player
import pReady
import profile
import pSet
import pstats
import Reload
import sGrab
import VisionLink


evalResult = "None has been evaluated"

# These are 4 levels of pausing.
# level 1: pause the leg only. --> The dog stands still and thinks.
# level 2: pause both leg and head --> The dog is thinking from a particular
# cplane.
# level 3: stop DecideNextAction()  --> The dog is really pausing,
# everything in its mind in the previous state is remained level 4: (mode 0)
# Stop everything, cannot communicate at all, except for sending cplane and
# world model

# the commands for them are (pairs of pausing/resuming)
# level 1 : (pyc) n pleg            /   (pyc) n rleg
# level 2 : (pyc) n paus[e]       /   (pyc) n resu[me]
# level 3 : (pyc) n nodna         /   (pyc) n nodna
# level 4 : (mode) 0                  /   (mode) 1
# n is the dog's number.

bStopMotion = False
bStopLegOnly = False
bStopDecideNextAction = False


Debug.bDebugOn = True # just to stop pychecker warning

profilerInstance = None
coverageInstance = None

#===============================================================================
# Functions belong to this module.
#===============================================================================

def dumpProfileStats():
    global profilerInstance
    
    if profilerInstance is not None:
    
        mystats = pstats.Stats(profilerInstance)
        profilerInstance = None
        mystats.dump_stats("/MS/profstat")
        print "profiler stats dumped"
    else:
        print "profiler was not switched on"

def startProfiler():
    global profilerInstance
    
    if coverageInstance == None:
        print "starting profiler"
        profilerInstance = profile.Profile()
    else:
        print "Cannot start profiler as coverage is running"

def stopCoverage():
    global coverageInstance
    if not (coverageInstance is None):
        coverageInstance.stop()
        coverageInstance.save()
        coverageInstance = None
        print "coverage stopped"
    else:
        print "coverage not switched on"

def startCoverage():
    global coverageInstance
    if profilerInstance is None:              
        print "starting coverage tool"
        coverageInstance = coverage.the_coverage
        coverageInstance.cache = "/MS/coverage"
        coverageInstance.erase()
        coverageInstance.start()
    else:
        print "Cannot start coverage as profiler is running"

def profileFunc(f):
    global profilerInstance
    
    if profilerInstance is None:              
        f()
    else:
        profilerInstance.runcall(f)

#--------------------------------------
# Called by processFrame() in robot/vision/pyEmbed.cc,
# is used for dynamic evaluation
def processFrame():
    VisionLink.startProfile("Behaviou.py")
    global profilerInstance,bStopMotion

    sec, usec = VisionLink.getCurrentTime()

    profileFunc(hFrameReset.framePreset)
    
    if Debug.frameDebug:
        print "RLNK{", VisionLink.getCurrentPreviousFrameID()[0], "}"
    
    if not bStopDecideNextAction:                           
        
        # If first call to ready state, reset the readyplayer's globals
        kickOffTeam = VisionLink.getKickOffTeam()
        if VisionLink.getTheCurrentMode() == Constant.READYSTATE\
                and (Global.state != Constant.READYSTATE\
                    or Global.kickOffTeam != kickOffTeam):
            Global.kickOffTeam = kickOffTeam
            pReady.initReadyMatchPlaying()
                
        profileFunc(hFrameReset.frameReset)
            
        # If it's not playing state, then we should reset grab.
        # Because we don't want to turn off gps vision update.
        if Global.state != Constant.PLAYINGSTATE:
            sGrab.resetPerform()                 
                   
            
        # Ready state - let me move to my correct position.
        if Global.state == Constant.READYSTATE or Debug.mustReady:            
            pReady.readyMatchPlaying()
            hTeam.sendWirelessInfo()
            
        # Set state - I can't move my legs, but I can move head to look for
        # ball.
        elif Global.state == Constant.SETSTATE:
            pSet.DecideNextAction()
            hTeam.sendWirelessInfo()
            
        elif Global.state == Constant.FINISHEDSTATE:
            hTeam.sendWirelessInfo()    
            
        else:
            profileFunc(Player.player.DecideNextAction)



    debugOnDemand()
    dynamic_gain()
    
    hFrameReset.framePostset()

    if bStopLegOnly:
        Action.stopLegs()

    if not bStopMotion:
        VisionLink.sendAtomicAction(*tuple(Action.finalValues) )
        
    VisionLink.stopProfile("Behaviou.py")    


#    newSec, newUsec = VisionLink.getCurrentTime()
#    elapsed = newSec-sec + (newUsec-usec)/1000000.0
#    if elapsed > 1/30.0:
#        print "Python: Behaviou.py: DROPPING FRAMES!!!!!!!!!!!!!!!!!!!!!!!!"
    
            
#--------------------------------------
# Called when received a wireless command.
def processCommand(command):
    global evalResult, bDebugForwardLeftTurn
    global bStopMotion,bStopLegOnly,bStopDecideNextAction
    cmd = command.strip()
    print "Python:I just got this command: ",cmd    
    if cmd[:4] == "eval":
        evalResult = eval(cmd[4:])
        VisionLink.sendMessageToBase("****"+str(evalResult))
    elif cmd[:4] == "exec":
        print "Executing (%s)" % cmd[4:].strip()
        exec(cmd[4:].strip())
    elif cmd[:4] == "pleg": #put the dog stand still, head still moves
        bStopLegOnly = True
    elif cmd[:4] == "rleg": #resume legs
        bStopLegOnly = False    
    elif cmd[:4] == "paus": #allow to pause the dog(both head and legs)
        bStopMotion = True
    elif cmd[:4] == "resu": #allow to resume the dog
        bStopMotion = False
    elif cmd[:4] == "colo": #allow to set team color
        color = int(cmd.split()[-1])
        VisionLink.setGPSGoal(color)
    elif cmd[:4] == "fmat": #change formation
        fmation = cmd[4:]
        try:
            Player.player.changeFormation(fmation)
        except:
            pass # some robots are above changing formations
    elif cmd[:5] == "nodna":
        bStopDecideNextAction = not bStopDecideNextAction
    elif command[:2] == "tt": # non strip()ed version
        try:
            sender = int(command[2])
            recipient = int(command[3])
            if recipient == Global.myPlayerNum or \
                   (recipient == 0 and sender != Global.myPlayerNum):
                 # strip header and send to player
                Player.player.processTeamTalk(command[4:], sender)
        except:
            pass # some robots just aren't talkative
    else:
        try:
            Player.player.processCommand(cmd)
        except:
            pass #that's fine, some player doesn't handle message


#--------------------------------------
#Print debug information only when some flag is turned on ( preferable via Dynamic Execution)
def debugOnDemand():
    if Global.bDebugForwardLeftTurn:
        print "F = %.2f, L = %.2f, T = %.2F" % \
            (Action.finalValues[Action.Forward],Action.finalValues[Action.Left],Action.finalValues[Action.TurnCCW])

def setLowGain():
    #print "LOW GAIN" #, Action.finalValues
    if Global.isHighGain:
        Global.isHighGain = False
        Action.finalValues[Action.Command] = Action.cmdSetLowGain

def setHighGain():
    #print "HIGH GAIN" #, Action.finalValues
    if not Global.isHighGain:
        Global.isHighGain = True
        Action.finalValues[Action.Command] = Action.cmdSetHighGain
    
# -------------------------------
# set the motor join gain by battery current
def dynamic_gain ():
    current = VisionLink.getBatteryCurrent()
    
    wt = Action.finalValues[Action.WalkType]
    if wt in [Action.ChestPushWT,     
              Action.FwdFastWT,
              Action.FwdHardWT,       
              Action.UpennRightWT,
              Action.UpennLeftWT,     
              Action.DiveKickWT, 
              Action.HandKickRightWT, 
              Action.HandKickLeftWT,
              Action.BuBuFwdKickWT,
              Action.NUFwdKickWT,
              Action.TestKickWT]:
        Global.forceHighGain = True
        #print "Kicking, force high gain"
    
    if Global.forceHighGain is None: #do dynamic_gain
        if current < 2500 and not Global.isHighGain:
            setHighGain()
        elif current > 3500 and Global.isHighGain:
            setLowGain()
    else:
        if Global.forceHighGain:
            setHighGain()
        else:
            setLowGain()
            
#--------------------------------------
# Initialisations before any frame is processed.
def init():
    print "Starting initialization of Behaviou.py"
    VisionLink.setCallbacks(processFrame, processCommand)    
    print "Finish initialization of Behaviou.py"


#===============================================================================
# These following statments are sort of like main function in C.
#===============================================================================

# Only the top-level module (ie. this file) should have this (non-function-scope
# statements). If other modules have statements not in function scope, like this, 
# they will be executed whenever they are imported, which is not what we want.

# Reload every dependent modules as described in Reload.py. 
reload(Reload)

init()

# Profiling
# ---------
# turn on profiling by uncommenting startProfiler() at the bottom
# of this comment block, or calling startProfiler() as described for 
# dumpProfileStats() below.
# to get profiling data, run the dog around for a while,
# open a telnet to the dog, run a SimpleRoboCommander,
# connect the SimpleRoboCommander to the dog with client,
# in 'Debug' tab type dumpProfileStats() and press execute
# make sure in the telnet you get the output of the function which
# should be "profiler stats dumped"
# shutdown the dog, get the file 'profstat' from the memory stick
# open up a terminal, navigate to where you put the profstat file and 
# go into interactive python
# type;
# import pstats
# pstats.Stats('profstat').print_stats()
# you can also sort the output by typing
# pstats.Stats('profstat').sort_stats('cumulative').print_stats()
# NOTE: it is best to run 4 dogs at once, all using profiling,
# since a lot of functions change their behaviour/speed
# for different amounts of dogs, and running profiling on all dogs
# at once will ensure they all take the same hit to speed so
# that the profiling dog won't be given a disadvantage 
# in terms of role switching (though it is only necessary to examine 
# the output from one forward)

# IMPORTANT: DO NOT LEAVE THIS ON BY DEFAULT, 
# TAKES A LONG TIME AND CAUSES MUCH FRAME DROPPAGE. 


#startProfiler()
