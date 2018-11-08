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


#ifndef OFFLINE
#include <ERA201D1.h>
#endif //OFFLINE

using namespace std;

unsigned long macAddress = 0;
bool isNewDog2005 = false;

static unsigned long isNew2005[] = {
  0xc6740fac,
  0xc66b055c,
  0xc66b0561,
  0xc66aff42,
  0xc66aff3a
};


void initMACAddress() {
  macAddress = 0;
#ifndef OFFLINE
  EtherDriverGetMACAddressMsg myMsg;
  EtherStatus result;
  if ((result = ERA201D1_GetMACAddress(&myMsg)) != ETHER_OK) {
   cout << "Bad ERA201D1_GetMACAddress return val: "
	 << etherStatusStr(result) << endl;
    macAddress = 0;
  } else {
    macAddress = ((myMsg.address.octet[2] << 24)
		  | (myMsg.address.octet[3] << 16)
		  | (myMsg.address.octet[4] << 8)
		  | (myMsg.address.octet[5]));
    cout << "Got MAC address for dog: " << myMsg.address
	 << " with low 4 bytes: " << macAddress << endl;
  }
#endif //OFFLINE
  
  
  isNewDog2005 = false;
  for (int i=0;i<5;i++)
    if (macAddress == isNew2005[i]) {
      isNewDog2005 = true;
      break;
    }
}
