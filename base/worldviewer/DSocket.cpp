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


#include "DSocket.h"
#include <stdio.h>

std::map<int, bool> UDPSocket::socket_list;

UDPSocket::UDPSocket()
{
   has_bound = 0;
   
   sockfd = socket(AF_INET, SOCK_DGRAM, 0);

   if (sockfd == -1)
      error = errno;
   else
      error = 0;
}

UDPSocket::~UDPSocket()
{
   if (sockfd != -1)
   {
      close();
   }
}

// Use INADDR_ANY as the address parameter for listening for stuff (e.g. server, udp broadcasts)
int UDPSocket::bind(in_addr_t address, int port)
{
   if (sockfd == -1)
      return UDPSOCKET_ERROR;
   
   int result;
   
   this->port = port;
   this->address = address;

   addr_struct.sin_family = AF_INET;
   addr_struct.sin_port = htons(port);
   addr_struct.sin_addr.s_addr = address;
   memset(&(addr_struct.sin_zero), '\0', 8); // zero the rest of the struct

   result = ::bind(sockfd, (struct sockaddr *)&addr_struct,sizeof(struct sockaddr));
   
   if (result == -1)
   {
      error = errno;
      return UDPSOCKET_ERROR;
   }
   else
   {
      has_bound = 1;
      return OK;
   }
}

// Same as above function, except with string as the address
int UDPSocket::bind(std::string address_str, int port)
{
   struct in_addr a;
   in_addr_t new_address;
   
   int result = inet_aton( address_str.c_str(), &a);

   if (result == 0)
   {
      error = EINVALID_ADDR;
      return UDPSOCKET_ERROR;
   }
   else
      new_address = a.s_addr;

   result = bind(port, new_address);

   return result;
}

int UDPSocket::receive(UDPPacket *packet)
{
   struct sockaddr_in from;
   socklen_t from_len;
   
   int buffer_size = packet->get_buffer_size();
   byte *buffer = packet->get_writable_buffer();

   if (!has_bound)
   {
      error = ENOT_BOUND;
      return UDPSOCKET_ERROR;
   }
   
   if (buffer == NULL)
   {
      // Should we flush the network socket here?
      error = ENULL_BUFFER;;
      return UDPSOCKET_ERROR;
   }
   
   from_len = sizeof(struct sockaddr);
   int result = recvfrom(sockfd, buffer, buffer_size, 0, (struct sockaddr *)&from, &from_len);

   if (result == -1)
   {
      error = errno;
      return UDPSOCKET_ERROR;
   }

   //printf("Read %d bytes from UDP...\n", result);
   
   packet->set_size(result);
   packet->set_addr(from.sin_addr.s_addr );
   packet->set_port(from.sin_port);

   socket_list[sockfd] = 0;
   
   return OK;
}


int UDPSocket::send(UDPPacket *packet, in_addr_t dest_address, unsigned short int dest_port)
{
   const byte *buffer = packet->get_buffer();
   struct sockaddr_in dest_addr_struct;

   if (buffer == NULL)
   {
      error = ENULL_BUFFER;
      return UDPSOCKET_ERROR;
   }
   
   if (!has_bound)
   {
      error = ENOT_BOUND;
      return UDPSOCKET_ERROR;
   }
   
   dest_addr_struct.sin_family = AF_INET;
   dest_addr_struct.sin_port = htons(dest_port);
   dest_addr_struct.sin_addr.s_addr = dest_address;
   memset(&(addr_struct.sin_zero), '\0', 8); // zero the rest of the struct

   //printf("Sending to address: %s\n", inet_ntoa(dest_addr_struct.sin_addr));
   
   int result = sendto(sockfd, buffer, packet->get_size(),0,(struct sockaddr *)&dest_addr_struct,sizeof(struct sockaddr));

   if (result == -1)
   {
      error = errno;
      return UDPSOCKET_ERROR;
   }

   return OK;
}

int UDPSocket::send(UDPPacket *packet, std::string dest_str, unsigned short int dest_port)
{
   struct in_addr a;
   in_addr_t new_address;
   
   int result = inet_aton( dest_str.c_str(), &a);

   if (result == 0)
   {
      error = EINVALID_ADDR;
      return UDPSOCKET_ERROR;
   }
   else
      new_address = a.s_addr;

   result = send(packet, new_address, dest_port);

   return result;
}
      
int UDPSocket::close()
{
   if (sockfd != -1)
   {
      socket_list.erase(sockfd);
      ::close(sockfd);
   }

   return OK;
}

int UDPSocket::select(long milliseconds)
{
   if (!has_bound)
   {
      error = ENOT_BOUND;
      return UDPSOCKET_ERROR;
   }
   
   // Check if we already have data waiting on the socket
   // (perhaps discovered by another class)
   if (socket_list[sockfd] == true)
      return OK;

   // We don't, so let's call select()!
   
   // Set the time
   struct timeval tv;

   tv.tv_sec = milliseconds / 1000;
   tv.tv_usec = (milliseconds % 1000) * 1000;

   // printf("seconds: %d microseconds: %d\n", tv.tv_sec, tv.tv_usec);
   
   // Create the fd_set containing all sockets without known waiting data
   fd_set readfds;
   FD_ZERO(&readfds);

   int highest_socket = -1;
  
   // Loop through current open sockets, find out which ones need to be checked
   std::map<int, bool>::iterator i;
   for (i = socket_list.begin(); i != socket_list.end(); i++)
   {
      int has_data_waiting = (*i).second;
      int socket_fd        = (*i).first;
      
      // Socket needs to be checked?
      if (has_data_waiting == false)
      {
         // Need the highest socket number for use with select()
         if (socket_fd > highest_socket)
            highest_socket = socket_fd;
         
         // Add it to the fd_set
         FD_SET(socket_fd, &readfds);
      }
   }

   int result = ::select(highest_socket + 1, &readfds, NULL, NULL, &tv);

   if (result == -1)
   {
      error = errno;
      return UDPSOCKET_ERROR;
   }

   // Now loop through the FD_SET to see which ones have data waiting
   for (i = socket_list.begin(); i != socket_list.end(); i++)
   {
      int has_data_waiting = (*i).second;
      int socket_fd        = (*i).first;
      
      // Check if this socket has data waiting
      if (FD_ISSET(socket_fd, &readfds))
      {
         // It does, so set it to true
         socket_list[socket_fd] = true;
      }
   }

   // Now check if OUR socket has data waiting!
   if (socket_list[sockfd] == true)
      return OK;
   else
      return FAIL;
}


int UDPSocket::set_broadcast(int broadcast)
{
   int result = setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));

   if (result == -1)
   {
      error = errno;
   }
   else
      return OK;
}

int UDPSocket::get_error()
{
   return error;
}

// Wrapper around strerror
char *UDPSocket::einvalid_address = "Invalid address supplied.";
char *UDPSocket::enull_buffer = "Null buffer in packet - possible allocation failure.";
char *UDPSocket::enot_bound = "Socket has not been bound to an address.";
const char *UDPSocket::get_error_str(int err)
{
   if (err == -1)
   {
      err = error;
   }
   
   switch (err)
   {
      case EINVALID_ADDR:
         return einvalid_address;
      case ENULL_BUFFER:
         return enull_buffer;
      case ENOT_BOUND:
         return enot_bound;
      default:
         return strerror(error);
   }
}

std::string UDPSocket::get_addr()
{
   struct in_addr in;

   in.s_addr = address;

   return inet_ntoa(in);
}


unsigned short int UDPSocket::get_port()
{
   return port;
}


