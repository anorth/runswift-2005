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


import SysPath

import Global
import HelpMath
import IndicatorAction
import sConstant
import sGrab as LearningBehaviour
import VisionLink

g_ParameterIncrement = None
g_PerformGenerator = None

def DecideNextAction(): 
    Global.frameReset()
    if g_PerformGenerator is None: 
        ResetPerform()
    g_PerformGenerator.next()

def ResetPerform():
    global g_PerformGenerator
    g_PerformGenerator = PerformGenerator()

def Perform(): 
    global g_PerformGenerator 
    if g_PerformGenerator is None: 
        g_PerformGenerator = PerformGenerator() 
    g_PerformGenerator.next() 


def PerformGenerator():
    t = 10
    num_trials_per_policy = 10 
    
    base_policy = LearningBehaviour.GetInitialPolicy() 
    last_policy = None
    
    while base_policy != last_policy:
        policies = [base_policy]*t     
        pscores = [0]*t
        
        for i in range(1,t):
            ##############################    
            # Generate new random policies
            ##############################
            policies[i] = GeneratePolicy(base_policy)
        
        for i in range(0,t): 
            total_score = 0 
            
            #########################################
            # Set i-th policy to the actual behaviour
            ######################################### 
            print str(policies[i])
            LearningBehaviour.SetPolicy(policies[i])
            
            # Give enough CPU break??
            yield sConstant.STATE_EXECUTING
            
            ###########################
            # Call the actual behaviour
            ###########################
            for _ in range(num_trials_per_policy): 
            
                ##############
                # Doing action
                ##############
                LearningBehaviour.ResetPerform()
                frame_count = 1
                while 1:
                    IndicatorAction.showHeadColor((1,0,0))
                    r = LearningBehaviour.Perform()
                    if r == sConstant.STATE_EXECUTING:
                        frame_count += 1 
                        yield sConstant.STATE_EXECUTING 
                    elif r == sConstant.STATE_SUCCESS: 
                        total_score += 1
                        lap_time = frame_count / 15
                        if lap_time > 20: 
                            lap_time = 20
                        total_score += 20 - lap_time 
                        break
                    elif r == sConstant.STATE_FAILED:
                        break
                  
                ################################
                # Getting ready for the next run
                ################################
                LearningBehaviour.ResetGetReady()        
                swap_frame = 0
                ind = (0,1,0)
                while 1:   
                       
                    if swap_frame > 30: 
                        swap_frame = 0 
                        if ind == (0,1,0): 
                            ind = (0,0,1)
                        else: 
                            ind = (0,1,0)
                    else: 
                        swap_frame += 1 
                    IndicatorAction.showHeadColor(ind)
                        
                    r = LearningBehaviour.GetReady()
                    if r == sConstant.STATE_EXECUTING: 
                        yield sConstant.STATE_EXECUTING
                    elif r == sConstant.STATE_SUCCESS:
                        break
            
            print "Total score for this trial: " + str(total_score)
             
            pscores[i] = (total_score + 0.0) / num_trials_per_policy    
        
        #####################################################
        # Yield here, so that the robot will not drop frames?
        #####################################################
        yield sConstant.STATE_EXECUTING
        
        last_policy = base_policy
        base_policy = Update(policies,pscores)
       
        #####################################################
        # Yield here, so that the robot will not drop frames?
        #####################################################
        yield sConstant.STATE_EXECUTING
        
        
        print "#####Best Policy So Far : " + str(base_policy)
    
    while 1: 
        yield sConstant.STATE_SUCCESS
    

def GeneratePolicy(base_policy): 
    new_policy = base_policy[:]
    incs = LearningBehaviour.GetParameterIncrements()
    
    for i in range(len(base_policy)):
        prob = VisionLink.random() % 3
        if prob == 0: 
            new_policy[i] -= incs[i]
        elif prob == 1: 
            new_policy[i] += incs[i] 
            
    return new_policy
    

def Update(policies,pscores): 
    
    base_policy = policies[0]
    n = len(base_policy)     
    
    ##############################
    # Setting up adjustment vector
    ##############################
    adjust_vec = []
    for parameter_index in range(n): 
    
        sum_inc  = [] 
        sum_zero = []
        sum_dec  = []
        
        for policy_index in range(len(policies)):
        
            if base_policy[parameter_index] < policies[policy_index][parameter_index]: 
                sum_inc.append(pscores[policy_index])
            elif base_policy[parameter_index] == policies[policy_index][parameter_index]: 
                sum_zero.append(pscores[policy_index])
            else: 
                sum_dec.append(pscores[policy_index])
        
        avg_inc  = HelpMath.mean(sum_inc)
        avg_zero = HelpMath.mean(sum_zero)
        avg_dec  = HelpMath.mean(sum_dec)
        
        if avg_zero > avg_inc and avg_zero > avg_dec: 
            adjust_vec.append(0) 
        else: 
            adjust_vec.append(avg_inc - avg_dec) 
    
    ########################################  
    # adjust <- adjust / |adjust| * constant
    ########################################
    constant = 2     
    adjust_vec = map(lambda x : x * constant, HelpMath.normalise(adjust_vec))
    
    ###########################
    # policy <- policy + adjust
    ########################### 
    best_policy = map(lambda x, y : x + y, base_policy, adjust_vec)
             
    return best_policy
