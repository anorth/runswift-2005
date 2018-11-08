#ifndef DSOCKET_H
#define DSOCKET_H

#include <string>
#include <vector>
#include <map>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "DPacket.h"

#define DEFAULT_UDP_PACKET_SIZE 1400

typedef uint8_t byte;



// - UDPSocket designed for polling (using select())
// - Doesn't use async sockets
// - Not thread safe! Use in threads and DIE
class UDPSocket
{
   public:
      enum 
      {
         UDPSOCKET_ERROR = -1,
         FAIL = 0,
         OK = 1,
         EINVALID_ADDR = 200,
         ENULL_BUFFER,
         ENOT_BOUND,
      };

      // Main functions
      UDPSocket();
      ~UDPSocket();
      int bind(in_addr_t address, int port);
      int bind(std::string address, int port);
      int receive(UDPPacket *packet);
      int send(UDPPacket *packet, in_addr_t dest, unsigned short int dest_port);
      int send(UDPPacket *packet, std::string dest_str, unsigned short int dest_port);
      int close();
      int select(long milliseconds = 0);

      int set_broadcast(int broadcast);
      int get_error();
      const char *get_error_str(int err = -1);

      // Utility functions
      std::string get_addr();
      unsigned short int get_port();
      
   private:
      int port;
      in_addr_t address;
      struct sockaddr_in addr_struct; // The struct required for connect() and sendto()
      int sockfd;                     // Socket file descriptor
      int error;                      // Last error...
      int has_bound;                  // Have we bounded socket?

   private:
      static std::map<int, bool> socket_list;
      static char *einvalid_address;
      static char *enull_buffer;
      static char *enot_bound;
};

#endif
