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


#ifndef DPACKET_H
#define DPACKET_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include "BinStream.h" // Look here for the real data jiggling stuff

class UDPPacket : public BinStream
{
   public:
      UDPPacket(long size);
      UDPPacket(const byte *source, long size);
      ~UDPPacket();

      // Getters
      in_addr_t get_addr();
      std::string get_addr_str();
      unsigned short int get_port();

      byte *get_writable_buffer();
      
      // Used by UDPSocket class, mainly...don't use these yourself!
      void set_size(long new_size);
      void set_addr(in_addr_t new_addr);
      void set_port(unsigned short int new_port);

   private:
      in_addr_t addr;
      unsigned short int port;
};
#endif
