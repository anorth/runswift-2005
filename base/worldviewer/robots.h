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


#include <stdio.h>
#include <map>
#include <string>
#include <set>
#include "BinStream.h"
#include "DSocket.h"
#include "timer.h"

//#define ROBOT_DEBUG 1

static const int NUM_OBSTACLE_SHARE = 24;

class Robot
{
   public:
      Robot(std::string ip, int playernum, int team, int port);
      void setpos(float posx, float posy, float posh, float posvar);
      void setball(double ballx, double bally, double ballvar);
      void setballtime(short ballTime);
      void setrolecounter(uint8_t roleCounter);
      void setbehaviour(uint8_t behavioursVal);
      void update_timer();
      long get_elapsed_time();

      std::string ip;
      
      // world model obj
      int playernum, team, port;
      float posx, posy, posh, posvar;
      double ballx, bally, ballvar;
      
      //uint32_t behavioursval;
      short ballTime;
      bool hasGrabbedBall; // set if you grabbed a ball
      bool hasSeenBall;    // set if you see a ball 
      bool hasLostBall;    // set if you lose a ball?
      uint8_t role;
      uint8_t roleCounter;
      
      // obstacle obj
      uint32_t obstacleVals[NUM_OBSTACLE_SHARE];
      
      time_t updated;
      long current_time;
      long prev_time;
      Timer timer;
};

class RobotList
{
   public:
      RobotList();
      void update();
      int add_socket(int port);
      int remove_socket(int port);
      std::set<int> get_ports();
      
      std::map<int, Robot *> get_robot_list();
      std::set<int> get_removed();
      std::set<int> get_added();
      
   protected:
      void process_packet(UDPPacket *packet, int port);
      std::map<int, Robot *> list;
      std::set<int> removed;
      std::set<int> added;
      std::vector<UDPSocket *> sockets;
      int idcount;
};


