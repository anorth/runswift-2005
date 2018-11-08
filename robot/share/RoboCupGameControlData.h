/*
   Copyright 2005 The University of New South Wales (UNSW) and National  
   ICT Australia (NICTA).

   This file is part of the 2005 team rUNSWift RoboCup entry.  You may  
   redistribute it and/or modify it under the terms of the GNU General  
   Public License as published by the Free Software Foundation; either  
   version 2 of the License, or (at your option) any later version as  
   modified below.  As the original licensors, we add the following  
   conditions to that license:

   In paragraph 2.b), the phrase "distribute or publish" should be  
   interpreted to include entry into a competition, and hence the source  
   of any derived work entered into a competition must be made available  
   to all parties involved in that competition under the terms of this  
   license.

   In addition, if the authors of a derived work publish any conference  
   proceedings, journal articles or other academic papers describing that  
   derived work, then appropriate academic citations to the original work  
   should be included in that publication.

   This rUNSWift source is distributed in the hope that it will be useful,  
   but WITHOUT ANY WARRANTY; without even the implied warranty of  
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  
   General Public License for more details.

   You should have received a copy of the GNU General Public License along  
   with this source code; if not, write to the Free Software Foundation,  
   Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 */


/*
    Copyright (C) 2005  University Of New South Wales

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software source code file, to deal in the file without restriction, 
    including without limitation the rights to use, copy, modify, merge, 
    publish, distribute, sublicense, and/or sell copies of the file, and to 
    permit persons to whom the file is furnished to do so, subject to the 
    following conditions:

    The above copyright notice and this permission notice shall be included in 
    all copies or substantial portions of the software source file.

    THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
    IN THE SOFTWARE.
*/

/*******************************************************************************
*
* RoboCupGameControlData.h
*
* Broadcasted data structure and constants
*
* willu@cse.unsw.edu.au
* shnl327@cse.unsw.edu.au
*
*******************************************************************************/

#ifndef _RoboCupGameControlData_h_DEFINED
#define _RoboCupGameControlData_h_DEFINED

// data structure version number
#define STRUCT_VERSION              4

// data structure header
#define STRUCT_HEADER               "RGme"                  

// team numbers
#define TEAM_BLUE                   0
#define TEAM_RED                    1
    
// game states
#define STATE_INITIAL               0
#define STATE_READY                 1
#define STATE_SET                   2
#define STATE_PLAYING               3
#define STATE_FINISHED              4
    
// penalties
#define PENALTY_NONE                0
#define PENALTY_BALL_HOLDING        1
#define PENALTY_GOALIE_PUSHING      2
#define PENALTY_PLAYER_PUSHING      3
#define PENALTY_ILLEGAL_DEFENDER    4
#define PENALTY_ILLEGAL_DEFENSE     5
#define PENALTY_OBSTRUCTION         6
#define PENALTY_REQ_FOR_PICKUP      7
#define PENALTY_LEAVING             8    
#define PENALTY_DAMAGE              9    



// information that describes a player
struct RobotInfo {
    uint32 penalty;             // penalty state of the player
    uint32 secsTillUnpenalised; // estimate of time till unpenalised
};


// information that describes a team
struct TeamInfo {
    uint32 teamNumber;          // unique team number
    uint32 teamColour;          // colour of the team
    uint32 score;               // team's score
    RobotInfo players[4];       // the team's players
};


struct RoboCupGameControlData {
    char   header[4];           // header to identify the structure
    uint32 version;             // version of the data structure
    uint32 state;               // state of the game
    uint32 firstHalf;           // 1 = game in first half, 0 otherwise
    uint32 kickOffTeam;         // the next team to kick off (TEAM_RED/BLUE)
    uint32 secsRemaining;       // estimate of seconds remaining in the half
    uint32 dropInTeam;          // team that caused last drop-in (TEAM_RED/BLUE)
    uint32 dropInTime;          // seconds passed since the last drop-in
    TeamInfo teams[2];
};

#endif

