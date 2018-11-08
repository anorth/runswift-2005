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


static inline int32_t swapl(const int32_t val) {
	return ((((val) & 0xff000000) >> 24) | (((val) & 0x00ff0000) >> 8) |
			(((val) & 0x0000ff00) << 8) | (((val) & 0x000000ff) << 24));
}

// convert 4 consecutive data elements in a given byte array to an integer
static inline int bytesToInt(const unsigned char *data, int offset) {
	int32_t oldInt = ((int)(data[offset] & 0xff) + 
            (int)((data[offset+1] & 0xff) << 8) + 
            (int)((data[offset+2] & 0xff) << 16) + 
            (int)((data[offset+3] & 0xff) << 24));
	return oldInt;
}

#ifndef LITTLE_ENDIAN
// note: this is the opposite of network byte order

static inline void hostToDog(WDataInfo &wdata) {
	wdata.robot = swapl(wdata.robot);
	wdata.type = swapl(wdata.type);
	wdata.size = swapl(wdata.size);
}

static inline void dogToHost(WDataInfo &wdata) {
	wdata.robot = swapl(wdata.robot);
	wdata.type = swapl(wdata.type);
	wdata.size = swapl(wdata.size);
}

static inline void hostToDog(int32_t &data) {
	data = swapl(data);
}

static inline void dogToHost(int32_t &data) {
	data = swapl(data);
}

static inline void hostToDog(float &data) {
	int *idata = (int *)&data;
	*idata = swapl(*idata);
}

static inline void dogToHost(float &data) {
	int *idata = (int *)&data;
	*idata = swapl(*idata);
}

#else //is LITTLE_ENDIAN

static inline void hostToDog(WDataInfo &) {
}

static inline void dogToHost(WDataInfo &) {
}

static inline void hostToDog(int32_t &) {
}

static inline void dogToHost(int32_t &) {
}

static inline void hostToDog(float &) {
}

static inline void dogToHost(float &) {
}

#endif //LITTLE_ENDIAN

