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


#ifndef BINSTREAM_H
#define BINSTREAM_H

#include <netinet/in.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>

#ifndef SEEK_SET
#define SEEK_SET 0
#endif
#ifndef SEEK_CUR
#define SEEK_CUR 1
#endif
#ifndef SEEK_END
#define SEEK_END 2
#endif

typedef uint8_t byte;

// Endian order enums
enum
{
   BS_HOST_ENDIAN = 0,
   BS_BIG_ENDIAN = 1,
   BS_LITTLE_ENDIAN = 2,
};

class BinStream
{
   public:
      enum 
      {
         BINSTREAM_ERROR = -1,
         OK = 1,
      };
      
      BinStream(long size);
      BinStream(const byte *source, long size);
      virtual ~BinStream();

      template<typename T> int read(T *r, int count=1);
      template<typename T> int write(T *r, int count=1);
      template<typename T> int write(T r);

      long tell();
      void seek(long offset, long origin);
      void rewind();

      long get_size();
      long get_buffer_size();
      const byte *get_buffer();

      // Do we perform any byter order convertsion or not?
      void set_convert_byte_order(bool do_convert_byte_order);
      
      // Normally, our stream is network byte order (big endian)
      void set_stream_endian(int endian = BIG_ENDIAN); 
      void set_host_endian(int endian = BS_HOST_ENDIAN); // Can't think of any time when you have to change this
      
      int set_buffer(const byte *source, long size);
      
      static int get_default_host_endian();

   protected:
      void initialise();
      
      template<typename T> int read(T *r, size_t var_size, int count, int byteorder);
      template<typename T> int write(T *r, size_t var_size, int count, int byteorder);
      int resize_buffer(long new_buffer_size);
      int expand_buffer(long target_size);
      int contract_buffer(long min_size = -1);

   protected:
      byte *buffer;
      long size;
      long buffer_size;
      long position;
      bool do_convert_byte_order;

      int stream_endian;
      int host_endian;
};

// Swap macros (out of place swapping)
// Yes, I think they're the htons and htonl macros...

#define SHORT_SWAP(A) ((((uint16_t)(A) & 0xff00) >> 8) | \
                       (((uint16_t)(A) & 0x00ff) << 8))

#define LONG_SWAP(A)  ((((uint32_t)(A) & 0xff000000) >> 24) | \
                       (((uint32_t)(A) & 0x00ff0000) >> 8)  | \
                       (((uint32_t)(A) & 0x0000ff00) << 8)  | \
                       (((uint32_t)(A) & 0x000000ff) << 24))

#endif
