#include "BinStream.h"
//#include <stdio.h>

// WARNING: This file is messy, and probably really bad code

/**
 * @brief Initialises the buffer to the specified size.
 *
 * @param size  
 *
 * @return 
 */
BinStream::BinStream(long size)
{
   this->size = 0;
   buffer_size = 0;
   position = 0;
   buffer = NULL;
   network_order = false;
   expand_buffer(size);
}

BinStream::BinStream(const byte *source, long size)
{
   this->size = size;
   buffer_size = 0;
   position = 0;
   buffer = NULL;
   network_order = false;
   if (expand_buffer(size))
      memcpy(buffer, source, size);
}

BinStream::~BinStream()
{
   if (buffer != NULL)
      free(buffer);
}

template<typename T> int BinStream::read(T *r, int count)
{
   size_t var_size = sizeof(T);
   return read(r, var_size, count);
}
   
template<> int BinStream::read<int16_t>(int16_t *r, int count)
{
   return read(r, sizeof(int16_t), count, network_order ? BYTEORDER_SHORT : BYTEORDER_NONE);
}

template<> int BinStream::read<int32_t>(int32_t *r, int count)
{
   return read(r, sizeof(int32_t), count, network_order ? BYTEORDER_LONG : BYTEORDER_NONE);
}

template<> int BinStream::read<uint16_t>(uint16_t *r, int count)
{
   return read(r, sizeof(int16_t), count, network_order ? BYTEORDER_SHORT : BYTEORDER_NONE);
}

template<> int BinStream::read<uint32_t>(uint32_t *r, int count)
{
   return read(r, sizeof(int32_t), count, network_order ? BYTEORDER_LONG : BYTEORDER_NONE);
}

template<> int BinStream::read<long>(long *r, int count)
{
   return read(r, sizeof(long), count, network_order ? BYTEORDER_LONG : BYTEORDER_NONE);
}

template<> int BinStream::read<float>(float *r, int count)
{
   return read(r, sizeof(float), count, network_order ? BYTEORDER_SHORT : BYTEORDER_NONE);
}

template<> int BinStream::read<double>(double *r, int count)
{
   return read(r, sizeof(double), count, network_order ? BYTEORDER_LONG : BYTEORDER_NONE);
}

template<typename T> int BinStream::read(T *r, size_t var_size, int count, int byteorder)
{
   if (var_size * count + position > size)
   {
      // Calculate how many we can fit!
      count = (size - position) / var_size;
   }
   
   int i;

   // A little hacky, but efficient...
   if (byteorder == BYTEORDER_NONE)
   {
      for (i = 0; i < count; i++)
      {
         *r = *(T *)(buffer + position);
         position += var_size;
         r += 1;
      }
   }
   else if (byteorder == BYTEORDER_SHORT)
   {
      for (i = 0; i < count; i++)
      {
         *r = (T)ntohs( *(int16_t *)(buffer + position) );
         position += var_size;
         r += 1;
      }
   }
   else if (byteorder == BYTEORDER_LONG)
   {
      for (i = 0; i < count; i++)
      {
         *r = (T)ntohs( *(int32_t *)(buffer + position) );
         position += var_size;
         r += 1;
      }
   }
   return 1;
}

template<typename T> int BinStream::write(T r)
{
   return write(&r, 1);
}

template<typename T> int BinStream::write(T *r, int count)
{
   size_t var_size = sizeof(T);
   return write(r, var_size, count);
}

template<> int BinStream::write<int16_t>(int16_t *r, int count)
{
   return write(r, sizeof(int16_t), count, network_order ? BYTEORDER_SHORT : BYTEORDER_NONE);
}

template<> int BinStream::write<int32_t>(int32_t *r, int count)
{
   return write(r, sizeof(int32_t), count, network_order ? BYTEORDER_LONG : BYTEORDER_NONE);
}

template<> int BinStream::write<uint16_t>(uint16_t *r, int count)
{
   return write(r, sizeof(int16_t), count, network_order ? BYTEORDER_LONG : BYTEORDER_NONE);
}

template<> int BinStream::write<uint32_t>(uint32_t *r, int count)
{
   return write(r, sizeof(uint32_t), count, network_order ? BYTEORDER_LONG : BYTEORDER_NONE);
}

template<> int BinStream::write<long>(long *r, int count)
{
   return write(r, sizeof(long), count, network_order ? BYTEORDER_LONG : BYTEORDER_NONE);
}

template<> int BinStream::write<float>(float *r, int count)
{
   return write(r, sizeof(float), count, network_order ? BYTEORDER_SHORT : BYTEORDER_NONE);
}

template<> int BinStream::write<double>(double *r, int count)
{
   return write(r, sizeof(double), count, network_order ? BYTEORDER_LONG : BYTEORDER_NONE);
}

template<typename T> int BinStream::write(T *r, size_t var_size, int count, int byteorder)
{
   long new_position = var_size * count + position;
   int result;

   // Fix later, but must fix! Otherwise we're resizing the buffer
   // multiple times unnecessarily
   if (new_position > buffer_size)
      result = expand_buffer(new_position);

   if (result == BINSTREAM_ERROR)
      return BINSTREAM_ERROR;
   
   int i;

   if (byteorder == BYTEORDER_NONE)
   {
      T *tcast;
      for (i = 0; i < count; i++)
      {
         // HACKY HACKY HACKY :D
         // Only way I could typecast without warnings and errors, though
         tcast = (T *)(void *)(buffer + position);
         *tcast = r[i];
         position += var_size;
      }
   }
   else if (byteorder == BYTEORDER_SHORT)
   {
      int16_t *tcast;
      for (i = 0; i < count; i++)
      {
         tcast = (int16_t *)(void *)(buffer + position);
         *tcast = htons(*((int16_t *)(r + i)));
         //*tcast = htons(*((int16_t *)((void *)&r[i])));  // 10 points if you figure this out
         position += var_size;
      }
   }
   else if (byteorder == BYTEORDER_LONG)
   {
      int32_t *tcast;
      for (i = 0; i < count; i++)
      {
         tcast = (int32_t *) ((void *)&(buffer[position]));
         *tcast = htons(*((int32_t *)(r + i)));
         //*tcast = htonl(*((int32_t *)((void *)&r[i])));  // 10 points if you figure this out
         position += var_size;
      }
   }

   if (new_position > size)
      size = new_position;
   
   return count;
}


/**
 * @brief 
 *
 * @param offset  
 * @param origin  
 *
 * @return 
 */
void BinStream::seek(long offset, long origin)
{
   switch (origin)
   {
      case SEEK_SET:
         if (offset > size)
            position = size;
         else if (offset < 0)
            position = 0;
         else
            position = offset;
         break;
      case SEEK_CUR:
         if (offset + position > size)
            position = size;
         else
            position += offset;
         break;
      case SEEK_END:
         if (offset > size)
            position = 0;
         else if (offset < 0)
            position = size;
         else
            position = size - offset;
         break;
      default:
         break;
   }
}


/**
 * @brief 
 *
 * @return 
 */
void BinStream::rewind()
{
   position = 0;
}


/**
 * @brief 
 *
 * @return 
 */
long BinStream::get_size()
{
   return size;
}

/**
 * @brief 
 *
 * @return 
 */
const byte *BinStream::get_buffer()
{
   return buffer;
}

/**
 * @brief 
 *
 * @return 
 */
long BinStream::get_buffer_size()
{
   return buffer_size;
}

/**
 * @brief 
 *
 * @param new_buffer_size  
 *
 * @return 
 */
int BinStream::resize_buffer(long new_buffer_size)
{
   // Size in bytes, so don't need sizeof(byte)

   if (buffer == NULL)
      buffer = (byte *)malloc(new_buffer_size);
   else
      buffer = (byte *)realloc(buffer,new_buffer_size);

   if (buffer == NULL)
   {
      buffer_size = -1;  
      return BINSTREAM_ERROR;
   }
   else
   {
      buffer_size = new_buffer_size;
      return OK;
   }
}

/**
 * @brief 
 *
 * @return 
 */
int BinStream::expand_buffer(long target_size)
{
   if (buffer_size > target_size)
      return OK;
   
   // Get next power of two
   long value = 1;
   while (value < target_size)
      value <<= 1;
   
   // Simple strategy for now...
   return resize_buffer(value);
}

int BinStream::contract_buffer(long min_size)
{
   if (min_size < size)
      min_size = size;
   
   // Get next power of two
   long value = 1;
   while (value < min_size)
      value <<= 1;
   
   // Simple strategy for now...
   return resize_buffer(value);
}

/**
 * @brief 
 *
 * @param net_order  
 *
 * @return 
 */
void BinStream::set_network_order(bool net_order)
{
   network_order = net_order;
}

int BinStream::set_buffer(const byte *source, long size)
{
   int result;
   
   if (size > buffer_size)
      result = expand_buffer(size);

   if (result == BINSTREAM_ERROR)
      return BINSTREAM_ERROR;

   memcpy(buffer, source, size);
   this->size = size;

   return OK;
}

template int BinStream::write<char>(char r);
template int BinStream::write<float>(float r);
template int BinStream::write<int16_t>(int16_t r);
template int BinStream::write<int32_t>(int32_t r);
template int BinStream::write<long>(long r);
template int BinStream::write<double>(double r);

template int BinStream::write<char>(char *r, int count);
template int BinStream::write<float>(float *r, int count);
template int BinStream::write<int16_t>(int16_t *r, int count);
template int BinStream::write<int32_t>(int32_t *r, int count);
template int BinStream::write<uint16_t>(uint16_t *r, int count);
template int BinStream::write<uint32_t>(uint32_t *r, int count);
template int BinStream::write<long>(long *r, int count);
template int BinStream::write<double>(double *r, int count);

template int BinStream::read<char>(char *r, int count);
template int BinStream::read<float>(float *r, int count);
template int BinStream::read<int16_t>(int16_t *r, int count);
template int BinStream::read<int32_t>(int32_t *r, int count);
template int BinStream::read<uint16_t>(uint16_t *r, int count);
template int BinStream::read<uint32_t>(uint32_t *r, int count);
template int BinStream::read<long>(long *r, int count);
template int BinStream::read<double>(double *r, int count);

