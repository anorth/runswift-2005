#!/usr/bin/python

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



import sys
from qt import *
from qtnetwork import *

class ClientSocket(QSocket):
    def __init__( self,sock, parent=None, name=None ) :
        QSocket.__init__(self, parent, name )
        line = 0;
        QObject.connect( self, SIGNAL("readyRead()"),
                self.readClient );
##~         QObject.connect( self, SIGNAL("connectionClosed()"),
##~                 self.deleteLater );
        self.setSocket( sock )
    
    def readClient(self):
        print "Got something to read"
        #ts = QTextStream (self);
##~         print self.bytesAvailable()
        while self.bytesAvailable() > 0 :
            print "Got ", self.bytesAvailable(), " to read "
            #str = self.readBlock(self.bytesAvailable());
            #str = self.readLine(self.bytesAvailable());
            #str = ts.read();
            str = self.readLine();
            print "Read successfully -> ",str, len(str)
            if  len(str) == 0: break
            self.emit( PYSIGNAL("logText"),(str,) )
            print "Emit successfully"
    
    def sendClient(self, text):
        ds = QDataStream(self)
        ds.writeRawBytes(text)

class SimpleServer (QServerSocket):
    def __init__( self,port=0, backlog = 1, parent=None ) :
        QServerSocket.__init__(self, port, backlog, parent )
        if not self.ok() :            
            raise ("Failed to bind to port :"+str(port) )
        else:
            print "Listening on port " , port


    def newConnection( self,socket ):
        print "Got new connection " 
        s = ClientSocket( socket);
        self.emit(PYSIGNAL("newConnect()"),(s,) )  
