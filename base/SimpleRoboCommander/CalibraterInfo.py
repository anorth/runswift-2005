##  
##     Copyright 2004 The University of New South Wales (UNSW) and National  
##     ICT Australia (NICTA).
##  
##     This file is part of the 2004 team rUNSWift RoboCup entry.  You may  
##     redistribute it and/or modify it under the terms of the GNU General  
##     Public License as published by the Free Software Foundation; either  
##     version 2 of the License, or (at your option) any later version as  
##     modified below.  As the original licensors, we add the following  
##     conditions to that license:
##  
##     In paragraph 2.b), the phrase "distribute or publish" should be  
##     interpreted to include entry into a competition, and hence the source  
##     of any derived work entered into a competition must be made available  
##     to all parties involved in that competition under the terms of this  
##     license.
##  
##     In addition, if the authors of a derived work publish any conference  
##     proceedings, journal articles or other academic papers describing that  
##     derived work, then appropriate academic citations to the original work  
##     must be included in that publication.
##  
##     This rUNSWift source is distributed in the hope that it will be useful,  
##     but WITHOUT ANY WARRANTY; without even the implied warranty of  
##     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  
##     General Public License for more details.
##  
##     You should have received a copy of the GNU General Public License along  
##     with this source code; if not, write to the Free Software Foundation,  
##     Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
##  
##  
##  
##   Last modification background information
##   $Id$
##  
##   Copyright (c) 2004 UNSW
##   All Rights Reserved.
##  
##  



from qt import *
from FormCalibraterInfo import FormCalibraterInfo

class CalibraterInfo(FormCalibraterInfo):
    def __init__(self,parent = None,name = None,fl = 0):
        FormCalibraterInfo.__init__(self,parent,name,fl)
        self.parent = parent
        self.initMemberVariables()
        self.initLogFile()
        
    def initMemberVariables(self):
        self.bStarted = False
        
    def initLogFile(self):
        f = open(str(self.ledFileName.text()),"w")
        f.close()
        
    def slotGotNewMessage(self,msg):
        f = open(str(self.ledFileName.text()),"a")
        f.write("%s %s\n" % (str(self.ledDistance.text()),msg))
        f.close()
        self.ledBlobArea.setText(msg)
        
    def slotStartStopSending(self):
        print "CalibraterInfo.slotStartStopSending(): "
        msg = "__03pyc__091 area %d" % self.bStarted
        self.parent.slotServerSend(msg)
        
        if not self.bStarted:
            self.btStartStop.setText("Stop")
        else:
            self.btStartStop.setText("Start")
        self.bStarted = not self.bStarted
        
    def slotReset(self):
        self.bStarted = True
        self.slotStartStopSending()
        self.initLogFile()
        
def run(argv):
    a = QApplication(argv)
    QObject.connect(a,SIGNAL("lastWindowClosed()"),a,SLOT("quit()"))
   
    w = CalibraterInfo()
    a.setMainWidget(w)
    w.show()
    a.exec_loop()

if __name__ == "__main__":
    run(sys.argv)
