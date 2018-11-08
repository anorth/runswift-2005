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



import os

#you know what.
def convertTo8dot3Name(fileName):
    if fileName.count(".") > 1:
        raise "%s contains too many 'dot'"
    
    part1 = fileName[:fileName.find('.')]
    part2 = fileName[fileName.find('.') + 1:]
    return part1[:8] + '.' + part2[:3]
    
def isNotGoodFtpRespons(resp):
    return int(resp) < 200 and int(resp) > 299

def getSizeAndTimeStamp(fileName):
        st = os.stat(fileName)
        return st[6:7] + st[8:9]
        
def getOnlyFileName(fileNameAndType):
    if '.' in fileNameAndType:
        return fileNameAndType[:fileNameAndType.find('.')]
    else:
        return fileNameAndType

def replace(st, u,v, rep):
    return st[:u] + rep + st[v:]
    
# Alexn: I think the message format is two underscores, followed by two ascii
# chars length, followed by length chars of name, two more underscores
# two chars length2, then length2 chars of value.
# When encoded the two underscores and two char length are replaced
# by a single byte binary length.
def decodeMsg(msg):
    t = -1
    code = -1
    while (msg.find("__") != -1):
        lastT = t
        t = msg.find("__")
        try:
            code = int(msg[t+2:t+4])  
            #HACK to enable msg length > 99
            if code == 99: code = 150 
            msg = replace(msg,t,t+4, "%c"%code)
        except ValueError:
            t = lastT
            break
            

    if (t > -1):
        if (len(msg[t+1:]) != code):
            if (len(msg[t+1:]) > code):
                msg = msg[0:t+1+code]
            else:
                msg += ' ' * (code - len(msg[t+1:]))
    return msg

def twoDigits(s):
    s = '0'*2 + s
    return s[-2:]

def runShellCommand(commandStr):
    put,get = os.popen4(commandStr)

def firstNLine(nline, msg):
    lines = msg.split("\n")[:nline]
    return "\n".join(lines)
