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
 * $Id:$
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 *
**/



#include <stdio.h>
#include <iostream>
#include <stdlib.h>

#include <qapplication.h>
#include "CPlaneDisplay.h"

void usage(const char* progname);

static char *saveFileName = "CPLANE_LOG";

int main (int argc, char *argv []) {
    char *readFileName = 0;
    
    QApplication app (argc, argv);
    CPlaneDisplay *cplaneDisplay = NULL;
    
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-h") == 0) {
            usage(argv[0]);
        } else if (strcmp(argv[i], "-l") == 0) {
            if (argc > i+1) {
                saveFileName = argv[++i];
            } else {
                usage(argv[0]);
            }
        } else if (strcmp(argv[i], "-f") == 0) {
            if (argc > i+1) {
                readFileName = argv[++i];
            } else {
                usage(argv[0]);
            }
        } else {
            usage(argv[0]);
        }
    }
    cplaneDisplay = new CPlaneDisplay(saveFileName, readFileName);
     app.setMainWidget(cplaneDisplay);
    cplaneDisplay->show();
    app.exec();
    return 0;
}

void usage(const char* progname) {
    cout << "Usage: " << progname << " [-h] [-l savefile] [-f readfile]" << endl;
    cout << "Specifying savefile means savefile will be " << endl
            << "overwriten with the received stream (logging is" << endl
            << "initially off) otherwise a default file name CPLANE_LOG is set." << endl;
    cout << "Specifying readfile loads plane data from readfile instead" << endl
            << "of listening for a network connection." << endl;
    cout << "-h shows this message." << endl;
    exit(1);
}
