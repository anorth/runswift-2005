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


#include "robots.h"


Robot::Robot(std::string ip, int playernum, int team, int port)
{
   this->ip = ip;
   this->playernum = playernum;
   this->team = team;
   this->port = port;
   current_time = -1;
   prev_time = -1;
   timer.start();
}

void Robot::setpos(float posx, float posy, float posh, float posvar)
{
   this->posx = posx;
   this->posy = posy;
   this->posh = posh;
   this->posvar = posvar;
}

void Robot::setball(double ballx, double bally, double ballvar)
{
   this->ballx = ballx;
   this->bally = bally;
   this->ballvar = ballvar;
}

void Robot::setbehaviour(uint8_t behaviourVals)
{
   hasGrabbedBall = (behaviourVals & 1) > 0;
   hasSeenBall = (behaviourVals & 2) > 0;
   hasLostBall = (behaviourVals & 4) > 0;

   role = behaviourVals >> 3;
}


void Robot::setballtime(short ballTime)
{
   this->ballTime = ballTime;
}

void Robot::setrolecounter(uint8_t roleCounter)
{
   this->roleCounter = roleCounter;
}

void Robot::update_timer()
{
   timer.start();
}

long Robot::get_elapsed_time()
{
   return timer.elapsed();
}

// *******************************************************************************


RobotList::RobotList()
{
   list.clear();
   removed.clear();
   added.clear();
   sockets.clear();
   idcount = 1;
}

void RobotList::process_packet(UDPPacket *packet, int port)
{
    char header[5] = {'\0'};
    int playerNum;
    int team;
    float posx, posy;
    float posh;
    float posVar, hVar;
    short ballTime;
    uint8_t behaviourVals;
    uint8_t roleCounter;

    double ballx;
    double bally;
    double ballvar;
    uint32_t obstacleVals[NUM_OBSTACLE_SHARE];

    packet->set_stream_endian(BS_LITTLE_ENDIAN); // AIBO's don't use network order to transmit,
    packet->rewind();
    packet->read(header, 4);

    if (strcmp(header, "NSwm") != 0)
      return;

    packet->read(&playerNum);
    packet->read(&team);

    Robot *rp = NULL;

    std::string ip = packet->get_addr_str();
    std::map<int, Robot *>::iterator i;

    // Search for robot
    for (i = list.begin(); i != list.end(); i++)
    {
      Robot *r = i->second;
      if ((r->ip == ip) && (r->playernum == playerNum) && (r->team == team))
      {
         rp = r;
         break;
      }
    }

    // Didn't find robot?
    if (rp == NULL)
    {
      rp = new Robot(ip, playerNum, team, port);
      int new_id = idcount;

      list[new_id] = rp;
      added.insert(new_id);

      idcount += 1;
    }

    //printf("header %s, playernum %d, team %d\n", header, playerNum, team);


    packet->seek(4, SEEK_CUR);
    packet->read(&posx);
    packet->read(&posy);
    packet->read(&posh);
    packet->read(&posVar);
    packet->read(&hVar);
    //packet->read(&behavioursVal);
    packet->read(&ballTime);
    packet->read(&behaviourVals);
    packet->read(&roleCounter);
    packet->read(&ballx);
    packet->read(&bally);
    packet->read(&ballvar);

    for (int i = 0; i < NUM_OBSTACLE_SHARE; i++) { 
        uint32_t val; 
        packet->read(&val);
        rp->obstacleVals[i] = val; 
    }



    rp->setpos(posx, posy, posh, posVar);
    rp->setball(ballx, bally, ballvar);
    rp->setballtime(ballTime);
    rp->setrolecounter(roleCounter);
    rp->setbehaviour(behaviourVals);
    rp->update_timer();

    //printf("posx %f, posy %f, posh %f, posvar %f, hvar %f\n", posx, posy, posh, posVar, hVar);
    //printf("behavioursVal %u\n", behavioursVal);
    //printf("ballx %f, bally %f, ballvar %f\n", ballx, bally, ballvar);
}

void RobotList::update()
{
   std::vector<UDPSocket *>::iterator it;
   removed.clear();
   added.clear();

   UDPSocket *s;

#ifdef ROBOT_DEBUG
   //printf("Updating...checking %d sockets\n", sockets.size());
#endif
   
   for (it = sockets.begin(); it != sockets.end(); it++)
   {
      s = *it;
      while (1)
      {
         int result = s->select(0);
         if (result == UDPSocket::UDPSOCKET_ERROR)
         {
            printf("We get error! %d: %s\n", s->get_error(), s->get_error_str());

            exit(1);
            break;
         }
         else if (result == 0)
            break;
         
#ifdef ROBOT_DEBUG
         printf("Incoming packet on %d!\n", s->get_port());
#endif
         
         UDPPacket packet(1200);

         result = s->receive(&packet);

         if (result == UDPSocket::UDPSOCKET_ERROR)
         {
            printf("We get error! %d: %s\n", s->get_error(), s->get_error_str());
         }

#ifdef ROBOT_DEBUG
         printf("Got packet\n");
#endif

         process_packet(&packet, s->get_port());
      }
   }

   std::map<int, Robot *>::iterator rit;
   for (rit = list.begin(); rit != list.end(); rit++)
   {
      Robot *r = rit->second;
      long elapsed = r->get_elapsed_time();

      r->prev_time = r->current_time;
      r->current_time = elapsed;
      
      // Haven't touched for 60 seconds?
      if (elapsed > 30000)
      {
         removed.insert(rit->first);
         list.erase(rit);
         delete(r);
      }
   }
}

int RobotList::add_socket(int port)
{
   std::vector<UDPSocket *>::iterator it;

   for (it = sockets.begin(); it != sockets.end(); it++)
   {
      UDPSocket *sock = *it;

      if (sock->get_port() == port)
         return 0;
   }
   
   // If port is already bound, it'll fail
   UDPSocket *s = new UDPSocket;
   if (s->bind(INADDR_ANY, port) != UDPSocket::UDPSOCKET_ERROR)
   {
      sockets.push_back(s);
#ifdef ROBOT_DEBUG
      printf("Added socket on port %d\n", port);
#endif
      return 1;
   }
   else
   {
      printf("Couldn't add port %d: Error %d: %s\n", port, s->get_error(), s->get_error_str(s->get_error()));
      return 0;
   }
}

int RobotList::remove_socket(int port)
{
   std::vector<UDPSocket *>::iterator it;

   for (it = sockets.begin(); it != sockets.end();)
   {
      UDPSocket *sock = *it;

      if (sock->get_port() == port)
      {
         delete sock;
         it = sockets.erase(it);
#ifdef ROBOT_DEBUG
         printf("Removed socket on port %d\n", port);
#endif
      }
      else
      {
         it++;
         //printf("Iterating...\n");
      }
   }

   return 1;
}

std::set<int> RobotList::get_ports()
{
   std::vector<UDPSocket *>::iterator it;
   std::set<int> port_list;

   for (it = sockets.begin(); it != sockets.end(); it++)
   {
      UDPSocket *sock = *it;

      port_list.insert(sock->get_port());
   }

   return port_list;
}

std::map<int, Robot *> RobotList::get_robot_list()
{
   return list;
}


std::set<int> RobotList::get_removed()
{
   return removed;
}

std::set<int> RobotList::get_added()
{
   return added;
}
