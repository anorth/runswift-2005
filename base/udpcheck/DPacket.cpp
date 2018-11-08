#include "DPacket.h"

UDPPacket::UDPPacket(long size) : BinStream(size)
{
   network_order = true;
}

UDPPacket::UDPPacket(const byte *source, long size) : BinStream(source, size)
{
   network_order = true;
}


UDPPacket::~UDPPacket()
{
}

in_addr_t UDPPacket::get_addr()
{
   return addr;
}

std::string UDPPacket::get_addr_str()
{
   struct in_addr from;
   from.s_addr = addr;
   return inet_ntoa(from);
}

unsigned short int UDPPacket::get_port()
{
   return port;
}

void UDPPacket::set_addr(in_addr_t new_addr)
{
   addr = new_addr;
}

void UDPPacket::set_port(unsigned short int new_port)
{
   port = new_port;
}

void UDPPacket::set_size(long new_size)
{
   size = new_size;
}

byte *UDPPacket::get_writable_buffer()
{
   return buffer;
}
