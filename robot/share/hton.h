/*

   Copyright 2004 The University of New South Wales (UNSW) and National  
   ICT Australia (NICTA).

   This file is part of the 2004 team rUNSWift RoboCup entry.  You may  
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
   must be included in that publication.

   This rUNSWift source is distributed in the hope that it will be useful,  
   but WITHOUT ANY WARRANTY; without even the implied warranty of  
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  
   General Public License for more details.

   You should have received a copy of the GNU General Public License along  
   with this source code; if not, write to the Free Software Foundation,  
   Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 */

/*
 * Last modification background information
 * $Id: hton.h 4658 2005-01-18 21:57:45Z alexn $
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 *
**/



#ifndef _hton_h
#define _hton_h

// Note: Dog byte order is the opposite of network byte order

#ifdef OFFLINE

#ifndef __USE_BSD
#define __USE_BSD
#endif  //__USE_BSD

#include <arpa/inet.h>

#if BYTE_ORDER == BIG_ENDIAN

#define      htodl(x)        ((((x) & 0xFF000000) >> 24) | (((x) & 0x00FF0000) >> 8) | \
								(((x) & 0x0000FF00) << 8) | (((x) & 0x000000FF) << 24))
#define      dtohl(x)        ((((x) & 0xFF000000) >> 24) | (((x) & 0x00FF0000) >> 8) | \
								(((x) & 0x0000FF00) << 8) | (((x) & 0x000000FF) << 24))
#define      htods(x)        ((((x) & 0xFF00) >> 8) | (((x) & 0x00FF) << 8))
#define      dtohs(x)        ((((x) & 0xFF00) >> 8) | (((x) & 0x00FF) << 8))

#else /* BYTE_ORDER == BIG_ENDIAN */

#define      htodl(x)        (x)
#define      dtohl(x)        (x)
#define      htods(x)        (x)
#define      dtohs(x)        (x)

#endif /* BYTE_ORDER == BIG_ENDIAN */

#else	/* offline */

#define      htonl(x)        ((((x) & 0xFF000000) >> 24) | (((x) & 0x00FF0000) >> 8) | \
								(((x) & 0x0000FF00) << 8) | (((x) & 0x000000FF) << 24))
#define      ntohl(x)        ((((x) & 0xFF000000) >> 24) | (((x) & 0x00FF0000) >> 8) | \
								(((x) & 0x0000FF00) << 8) | (((x) & 0x000000FF) << 24))
#define      htons(x)        ((((x) & 0xFF00) >> 8) | (((x) & 0x00FF) << 8))
#define      ntohs(x)        ((((x) & 0xFF00) >> 8) | (((x) & 0x00FF) << 8))
#define      htodl(x)        (x)
#define      dtohl(x)        (x)
#define      htods(x)        (x)
#define      dtohs(x)        (x)

#endif	/* offline */

#endif /* _hton_h */
