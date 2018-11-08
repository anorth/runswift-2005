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


/* reconfigure linear shift table in LinearShift.h
 * to have a different base dog
 *
 * though I have not calculated the error this may introduce
 * it was found experimentally to be extremely small
 * i.e. a new table was created for a new base dog, and using
 * that table a new table was calculated for the original base dog
 * and compared to the original table, the difference was negligible
 */

#include <stdlib.h>
#include "../../../robot/vision/LinearShifts.h"

using namespace std;

void usage(void);

int main(int argc, char* argv[]) {

    bool oldDogKnown = true;

    // check command line arguments for dog ips
    if (argc < 2) {
        usage();
        exit(1);
    }
    
    if (argc < 3) {
        oldDogKnown = false;
    }
    else if (argv[1] == argv[2])
    {
        fprintf(stderr, "ERROR: old base dog cannot be the same as new base dog\n");
        exit(1);
    }

    int i =0;
    int numFields = sizeof(DogMacLookup)/sizeof(DogMacLookupType);

    int newBaseDog = 0;
    bool oldBaseDogFound = false;
    bool newBaseDogFound = false;
        
    for (i=0; i  <numFields; i++) {
    
        if(DogMacLookup[i].mac == strtoul(argv[1], NULL, 16)) {
            newBaseDog = i;
            newBaseDogFound = true;
            continue;
        }
        
        if ((oldDogKnown) && (DogMacLookup[i].mac == strtoul(argv[2], NULL, 16))) {
            oldBaseDogFound = true;
        }
    }
    
    if (not(newBaseDogFound)) {
        fprintf(stderr, "ERROR: new base dog not found in DogMacLookup table\n");
        exit(1);
    }
    
    if (oldBaseDogFound) {
        fprintf(stderr, "ERROR: old base dog is a dog in the lookup table! (it shouldn't be)\n");
        exit(1);
    }

    DogColourDistortion newBaseDogShift = DogMacLookup[newBaseDog].colMunge;
    
    //calculate new dog shifts
    for (i=1; i < numFields; i++) {
        
        if (i == newBaseDog) continue;
        
        DogMacLookup[i].colMunge.My = DogMacLookup[i].colMunge.My / newBaseDogShift.My;
        DogMacLookup[i].colMunge.Cy = (DogMacLookup[i].colMunge.Cy - newBaseDogShift.Cy) / newBaseDogShift.My;
                                        
        DogMacLookup[i].colMunge.Mu = DogMacLookup[i].colMunge.Mu / newBaseDogShift.Mu;  
        DogMacLookup[i].colMunge.Cu = (DogMacLookup[i].colMunge.Cu - newBaseDogShift.Cu) / newBaseDogShift.Mu;
                                        
        DogMacLookup[i].colMunge.Mv = DogMacLookup[i].colMunge.Mv / newBaseDogShift.Mv;  
        DogMacLookup[i].colMunge.Cv = (DogMacLookup[i].colMunge.Cv - newBaseDogShift.Cv) / newBaseDogShift.Mv;
                                    
    }
    
    //calculate linear shift for old base dog
    if (oldDogKnown) {
        
        DogMacLookup[0].colMunge.Cy = -DogMacLookup[newBaseDog].colMunge.Cy      
                                                / DogMacLookup[newBaseDog].colMunge.My;
        DogMacLookup[0].colMunge.My = 1 / DogMacLookup[newBaseDog].colMunge.My;
        
        DogMacLookup[0].colMunge.Cu = -DogMacLookup[newBaseDog].colMunge.Cu      
                                                / DogMacLookup[newBaseDog].colMunge.Mu;
        DogMacLookup[0].colMunge.Mu = 1 / DogMacLookup[newBaseDog].colMunge.Mu;
        
        DogMacLookup[0].colMunge.Cv = -DogMacLookup[newBaseDog].colMunge.Cv      
                                                / DogMacLookup[newBaseDog].colMunge.Mv;
        DogMacLookup[0].colMunge.Mv = 1 / DogMacLookup[newBaseDog].colMunge.Mv;
        
        DogMacLookup[0].mac = strtoul(argv[2], NULL, 16);    
    }                
    
    //assign static entry for new base dog
    DogMacLookup[newBaseDog].colMunge.My = 1.0;
    DogMacLookup[newBaseDog].colMunge.Cy = 0.0;
    DogMacLookup[newBaseDog].colMunge.Mu = 1.0;
    DogMacLookup[newBaseDog].colMunge.Cu = 0.0;
    DogMacLookup[newBaseDog].colMunge.Mv = 1.0;
    DogMacLookup[newBaseDog].colMunge.Cv = 0.0;     
    DogMacLookup[newBaseDog].mac = 0;

    //swap new base dog with first entry
    DogMacLookupType tmp;
    tmp = DogMacLookup[0];
    DogMacLookup[0] = DogMacLookup[newBaseDog];
    DogMacLookup[newBaseDog] = tmp; 
  
    //print out the new lookup table
    bool newDogIsLast = false;
    if (newBaseDog == (numFields - 1)) {
        newDogIsLast = true;
    }

    cout << "DogMacLookupType DogMacLookup[] = {" << endl;
    cout << "    // Default Entry (Base Dog = " << argv[1] << ")" << endl;
    cout << "    {0, {1.0, 0.0, 1.0, 0.0, 1.0, 0.0}} ," << endl;
    for (i=1; i < numFields; i++) {
        if (not(oldDogKnown) && i == newBaseDog) {
            continue;
        }
    
        cout << "    {"; 
        
        printf("0x%x",(unsigned int) DogMacLookup[i].mac );
        cout << ", {" << DogMacLookup[i].colMunge.My << ", "
        << DogMacLookup[i].colMunge.Cy << ", "
        << DogMacLookup[i].colMunge.Mu << ", "
        << DogMacLookup[i].colMunge.Cu << ", "
        << DogMacLookup[i].colMunge.Mv << ", "
        << DogMacLookup[i].colMunge.Cv << "}}";
        
        if ((i != numFields - 1) 
            && not((newDogIsLast) && (not(oldDogKnown)) && (i == (numFields-2)))) {
            cout << ",";
        }
        
        cout << endl;          
    }
    
    cout << "};" << endl;
            
    return 0;
}

void usage(void) {
    char *msg =
        "Usage: reconfigureShifts newBaseDog [oldBaseDog]\n"
        "where;\n"
        "newBaseDog is the mac address of the dog in the table that you want to make the base dog\n"
        "oldBaseDog is the mac address of the base dog currently used in the table\n"  
        "the current base dog mac address can be found as a comment at the top\n"
        "of the DogMacLookup table, if the base dog mac address is specified an entry will be\n"
        "created in the table for the shift of that dog, else if it is not known it can\n"
        "be left out and that dog will just use the default linear shift (i.e. none)\n"      
        "\n"
        "IMPORTANT: the linear shift table is read in at COMPILE time, so it is vital that you\n"
        "remake this program every time you use it."
        "\n"        
        "NOTE: this program does not actually replace the table in LinearShifts.h\n"
        "it prints the new table to stdout so you will need to copy and paste it\n"
        "before it can take effect\n"
        "\n"
        "examples:\n"
        "   reconfigureShifts 0xC6245478 0xc6245481\n"
        "   reconfigureShifts 0xc66b055c 0xc624544b\n";
        
    fprintf(stderr, msg);
}
