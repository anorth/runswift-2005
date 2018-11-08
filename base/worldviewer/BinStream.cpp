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


#include "BinStream.h"
//#include <stdio.h>

/**
 * @brief Initialises the buffer to the specified size.
 *
 * @param size  
 *
 * @return 
 */
BinStream::BinStream(long size)
{
   initialise();
   expand_buffer(size);
}

BinStream::BinStream(const byte *source, long size)
{
   initialise();
   if (expand_buffer(size))
      memcpy(buffer, source, size);
}

BinStream::~BinStream()
{
   if (buffer != NULL)
      free(buffer);
}

void BinStream::initialise()
{
   this->size = 0;
   buffer_size = 0;
   position = 0;
   buffer = NULL;
   do_convert_byte_order = false;           // No byte swapping by default

   stream_endian = BS_BIG_ENDIAN;           // Default to network order
   host_endian = get_default_host_endian(); // Use default host endian value...
}

template<typename T> int BinStream::read(T *r, int count)
{
   size_t var_size = sizeof(T);

   // Check if we need to flip bytes
   if (do_convert_byte_order && (host_endian != stream_endian))
      return read(r, var_size, count, 1);
   else
      return read(r, var_size, count, 0);
}
   
template<typename T> int BinStream::read(T *r, size_t var_size, int count, int swap_byteorder)
{
   if (var_size * count + position > size)
   {
      // Calculate how many remaining elements we can read from the stream,
      // and cap it if the user has overstated the number of elements to read
      count = (size - position) / var_size;
   }
   
   int i;

   if (swap_byteorder && var_size == 2)
   {
      for (i = 0; i < count; i++)
      {
         *r = (T)SHORT_SWAP( *(uint16_t *)(buffer + position) );
         position += var_size;
         r += 1;
      }
   }
   else if (swap_byteorder && var_size == 4)
   {
      for (i = 0; i < count; i++)
      {
         *r = (T)LONG_SWAP( *(uint32_t *)(buffer + position) );
         position += var_size;
         r += 1;
      }
   }
   else
   {
      for (i = 0; i < count; i++)
      {
         *r = *(T *)(buffer + position);
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

  // Check if we need to flip bytes
   if (do_convert_byte_order && (host_endian != stream_endian))
      return write(r, var_size, count, 1);
   else
      return write(r, var_size, count, 0);
}

template<typename T> int BinStream::write(T *r, size_t var_size, int count, int swap_byteorder)
{
   // Calculate end position
   long new_position = var_size * count + position;
   
   int result;
   if (new_position > buffer_size)
      result = expand_buffer(new_position);

   if (result == BINSTREAM_ERROR)
      return BINSTREAM_ERROR;
   
   int i;

   if (swap_byteorder && var_size == 2)
   {
      uint16_t *tcast;
      for (i = 0; i < count; i++)
      {
         tcast = (uint16_t *)(void *)(buffer + position);
         *tcast = SHORT_SWAP(*((uint16_t *)(r + i)));
         position += var_size;
      }
   }
   else if (swap_byteorder && var_size == 4)
   {
      uint32_t *tcast;
      for (i = 0; i < count; i++)
      {
         tcast = (uint32_t *) ((void *)&(buffer[position]));
         *tcast = LONG_SWAP(*((uint32_t *)(r + i)));
         position += var_size;
      }
   }
   else
   {
      T *tcast;
      for (i = 0; i < count; i++)
      {
         tcast = (T *)(void *)(buffer + position);
         *tcast = r[i];
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
void BinStream::set_convert_byte_order(bool do_convert_byte_order)
{
   this->do_convert_byte_order = do_convert_byte_order;
}

void BinStream::set_stream_endian(int endian)
{
   if (endian == BS_HOST_ENDIAN)
      stream_endian = get_default_host_endian();
   else
      stream_endian = endian;
}

void BinStream::set_host_endian(int endian)
{
   if (endian == BS_HOST_ENDIAN)
      host_endian = get_default_host_endian();
   else
      host_endian = endian;
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

int BinStream::get_default_host_endian()
{
   // Run-time test for byte endian checking
   union { long l; char c[sizeof (long)]; } u;
   u.l = 1;
   
   if (u.c[sizeof (long) - 1] == 1)
      return BS_BIG_ENDIAN;
   else
      return BS_LITTLE_ENDIAN;
}

template int BinStream::write<char>(char r);
template int BinStream::write<float>(float r);
template int BinStream::write<int16_t>(int16_t r);
template int BinStream::write<int32_t>(int32_t r);
template int BinStream::write<uint8_t>(uint8_t r);
template int BinStream::write<uint16_t>(uint16_t r);
template int BinStream::write<uint32_t>(uint32_t r);
template int BinStream::write<long>(long r);
template int BinStream::write<double>(double r);

template int BinStream::write<char>(char *r, int count);
template int BinStream::write<float>(float *r, int count);
template int BinStream::write<int16_t>(int16_t *r, int count);
template int BinStream::write<int32_t>(int32_t *r, int count);
template int BinStream::write<uint8_t>(uint8_t *r, int count);
template int BinStream::write<uint16_t>(uint16_t *r, int count);
template int BinStream::write<uint32_t>(uint32_t *r, int count);
template int BinStream::write<long>(long *r, int count);
template int BinStream::write<double>(double *r, int count);

template int BinStream::read<char>(char *r, int count);
template int BinStream::read<float>(float *r, int count);
template int BinStream::read<int16_t>(int16_t *r, int count);
template int BinStream::read<int32_t>(int32_t *r, int count);
template int BinStream::read<uint8_t>(uint8_t *r, int count);
template int BinStream::read<uint16_t>(uint16_t *r, int count);
template int BinStream::read<uint32_t>(uint32_t *r, int count);
template int BinStream::read<long>(long *r, int count);
template int BinStream::read<double>(double *r, int count);

