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
