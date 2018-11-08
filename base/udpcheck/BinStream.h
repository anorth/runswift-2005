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


// Only for reading at the moment!
class BinStream
{
   public:
      enum 
      {
         BINSTREAM_ERROR = -1,
         OK = 1,
         BYTEORDER_NONE = 40,
         BYTEORDER_LONG,
         BYTEORDER_SHORT,
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

      void set_network_order(bool net_order);
      int set_buffer(const byte *source, long size);

   protected:
      template<typename T> int read(T *r, size_t var_size, int count, int byteorder = BYTEORDER_NONE);
      template<typename T> int write(T *r, size_t var_size, int count, int byteorder = BYTEORDER_NONE);
      int resize_buffer(long new_buffer_size);
      int expand_buffer(long target_size);
      int contract_buffer(long min_size = -1);
      
   protected:
      byte *buffer;
      long size;
      long buffer_size;
      long position;
      bool network_order;
};

#endif
