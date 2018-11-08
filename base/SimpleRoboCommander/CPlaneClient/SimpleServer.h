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
 * $Id$
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 *
**/



/****************************************************************************
** $Id: qt/server.cpp   3.1.1   edited Nov 8 10:35 $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef SIMPLESERVER_H
#define SIMPLESERVER_H

#include <qsocket.h>
#include <qserversocket.h>
#include <qapplication.h>
#include <qvbox.h>
#include <qtextview.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qtextstream.h>

#include <stdlib.h>


/*
  The ClientSocket class provides a socket that is connected with a client.
  For every client that connects to the server, the server creates a new
  instance of this class.
*/
class ClientSocket : public QSocket
{
    Q_OBJECT
public:
    ClientSocket( int sock, QObject *parent=0, const char *name=0 ) :
        QSocket( parent, name )
    {
        line = 0;
        connect( this, SIGNAL(readyRead()),
                SLOT(readClient()) );
        //connect( this, SIGNAL(connectionClosed()),
        //        SLOT(deleteLater()) );
        setSocket( sock );
    }

    ~ClientSocket()
    {
    }

signals:
    void logText( const QString& );

private slots:
    void readClient()
    {
        /*
        QTextStream ts( this );
        while ( canReadLine() ) {
            QString str = ts.readLine();
            emit logText( tr("Read: '%1'\n").arg(str) );

            ts << line << ": " << str << endl;
            emit logText( tr("Wrote: '%1: %2'\n").arg(line).arg(str) );

            line++;
        }
        */
    }

private:
    int line;
};


/*
  The SimpleServer class handles new connections to the server. For every
  client that connects, it creates a new ClientSocket -- that instance is now
  responsible for the communication with that client.
*/
class SimpleServer : public QServerSocket
{
    Q_OBJECT
public:
    SimpleServer( int port = 0, int backLog = 0 , QObject* parent=0 ) :
        QServerSocket( port, backLog, parent )
    {
        if ( !ok() ) {
            qWarning("Failed to bind to port 4242");
            //exit(1);
        }
    }

    ~SimpleServer()
    {
    }

    void newConnection( int socket )
    {
        ClientSocket *s = new ClientSocket( socket, this );
        emit newConnect( s );
    }

signals:
    void newConnect( ClientSocket* );
};

#endif // SIMPLESERVER_H
