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




# Q(s,a) + ALPHA * (reward + GAMMA * max Q(s',a') - Q(s,a))

GAMMA       = 0.8
ALPHA       = 0.9 
MAX_Q_VALUE = 1000

class QLearning:     

    def __init__(self,max_q_value=100,alpha=0.9,gamma=0.8): 
        global MAX_Q_VALUE, ALPHA, GAMMA
        MAX_Q_VALUE  = max_q_value
        ALPHA        = alpha
        GAMMA        = gamma
        self.m_Table = {} 
    
    def UpdateValues(self,current_state,current_action,reward,next_state): 
        new_q_value = (1 - ALPHA) * self.GetQValue(current_state,current_action) \
                      + ALPHA * (reward + GAMMA * self.GetMax(next_state))
    
        self.SetQValue(current_state,current_action,new_q_value)
    
    def SetQValue(self,state,action,q_value):
        state_obj = StateObj(state) 
        action_obj = ActionObj(action)  
        
        if not self.m_Table.has_key(state_obj): 
            self.m_Table[state_obj] = {}
              
        self.m_Table[state_obj][action_obj] = q_value
        
    def GetQValue(self,state,action):
        state_obj = StateObj(state) 
        action_obj = ActionObj(action)
        
        if not self.m_Table.has_key(state_obj): 
            return MAX_Q_VALUE
        
        state_table = self.m_Table[state_obj]
        
        if not state_table.has_key(action_obj):
            return MAX_Q_VALUE
        
        return state_table[action_obj]
 
        
    def GetMax(self,state): 
        state_obj = StateObj(state)
        
        if not self.m_Table.has_key(state_obj): 
            return MAX_Q_VALUE 
        
        state_table = self.m_Table[state_obj] 
        
        # should use ... function approx?
        return max(state_table.values())
        
    def GetArgMax(self,state): 
        state_obj = StateObj(state) 
        
        if not self.m_Table.has_key(state_obj): 
            return None 
        
        state_table = self.m_Table[state_obj] 
        
        max_q_value = 0
        arg_max = None
        for action_obj in state_table.keys(): 
            action = action_obj.value()
            q_value = state_table[action_obj]
            if max_q_value < q_value: 
                max_q_value = q_value
                arg_max = action 
        return arg_max
        
            
          
class StateObj:
    def __init__(self,state): 
        self.m_State = state
        
    def __hash__(self): 
        return str(self.m_State).__hash__()
        
    def value(self): 
        return self.m_State
    
class ActionObj:
    def __init__(self,action): 
        self.m_Action = action
        
    def __hash__(self): 
        return str(self.m_Action).__hash__()
        
    def value(self):
        return self.m_Action
