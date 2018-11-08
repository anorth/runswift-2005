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



#!/usr/bin/python

import os,sys,math


def jointValueFormat(val):
    if val == "2000000000": return "nan"
    else:
        t = float(val)
        d = ( (t/1000000)/math.pi ) * 180.0
        return str(d)

class JointLog:
    def __init__(self, jointLogFile = None, destDir = None):
        
        self.destDir = destDir
        self.jointLogFile = jointLogFile
        self.log = []
        
        if jointLogFile is not None:
            self.loadJointLog()
            
        
    
    def loadJointLog(self):
        f = open(self.jointLogFile)        
        str = f.readline()
        while str != "":
            self.log.append( str.split() )
            str = f.readline()
        f.close()
        
    def writeToDirectory(self):
        if self.destDir is None : 
            self.destDir = "."
        else:
            #do all sort of "brute" delete and re-create the directory
            try:
                os.remove(self.destDir + "/*")
            except:
                pass
            try:
                os.remove(self.destDir)
            except:
                pass
            try:
                os.mkdir(self.destDir)
            except:
                pass
            #now , hopefully we have a clean directory called self.destDir
            
        
        
        fileNameList = ["HeadTilt.log",
                "HeadPan.log",
                "HeadCrane.log",
        
                "FrontLeftJoint.log",
                "FrontLeftShoulder.log",
                "FrontLeftKnee.log",
        
                "FrontRightJoint.log",
                "FrontRightShoulder.log",
                "FrontRightKnee.log",
        
                "RearLeftJoint.log",
                "RearLeftShoulder.log",
                "RearLeftKnee.log",
        
                "RearRightJoint.log",
                "RearRightShoulder.log",
                "RearRightKnee.log",
            ]

        fileList = []
        for j in range(15):
            f = open(self.destDir + "/" + fileNameList[j],"w")
            fileList.append(f)
        
        for i in range(len(self.log)):
            oneLog = self.log[i]
            for j in range(15):
                try:
                    fileList[j].write("%3d %10s %10s\n" % (i,jointValueFormat(oneLog[j]),jointValueFormat(oneLog[j+15]) ) )
                except:
                    fileList[j].write("%3d %10s %10s\n" % (i,"nan" , "nan"))
                    
        for j in range(15):
            fileList[j].close()
                        
if __name__ == "__main__":
    if len(sys.argv) != 3:
        print "Usage: ./JointLog [joint log file name] [output directory]"
    else:
        w = JointLog(sys.argv[1],sys.argv[2])
        w.writeToDirectory()
    


