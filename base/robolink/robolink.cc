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


/* Robolink.cc
 * Base <-> Robot communications
 */

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

#include "robolink.h"
#include "../../robot/share/PackageDef.h"
#include "../../robot/share/hostToDog.h"


using namespace std;

int connectToDog(const char* host, int port);
int listenToPort(int port);
int sendAndReceive(int roboport, int clientsock, ofstream& logfile);
int readFromRobot(int robosock, ofstream& logfile,
                    fd_set& client_fds, int fd_max);
int readFromClient(int client, int robosock, fd_set& client_fds);

void usage(const char*);

static char recvbuf[ROBOLINK_BUF_LEN];
const char* IP_PREFIX = "10.1.2.";
static const int COMMAND_SEND_BUF_SIZE = sizeof(PackageDef) + sizeof(WDataInfo);

int main(int argc, char* argv[]) {
    const char* host;
    char *logfilename = "log.rlog";
    
    /* Parse command line args */
    if (argc < 2)
        usage(argv[0]);
    host = argv[1];

    if (strlen(host) == 3) { // prepend network to abbveviated ip
        ostringstream str;
        str << IP_PREFIX << host;
        host = str.str().c_str();
    }
    
    /* Open log file */
    ofstream logfile(logfilename, ofstream::binary);
    if (! logfile.is_open()) {
        perror("Error opening log file");
        exit(1);
    }
    
    /* Open data port for clients */
    int clientsock = listenToPort(ROBOLINK_DOG_PORT);
    // FIXME: open type-specific ports as well
    
    /* Loop over the connection and transmission so that this
     * reconnects to the dog when the connection closes.
     */
    while (true) {
        int ret;
        
        /* Connect to dog */
        int robosock = connectToDog(host, ROBOLINK_DOG_PORT);
        if (robosock < 0) {
            sleep(3);
            continue; // retry
        }

        /* Receive for ever */
        ret = sendAndReceive(robosock, clientsock, logfile);

        /* A non-zero return means end the program, otherwise close down the
         * robo port and connect again.
         */
        close(robosock);
        if (ret != 0) {
            break;
        }
    }

    logfile.close();

    // close data ports
    return 0;
}

/* Opens a socket to the specified host/ip string. Returns the socket fd */
int connectToDog(const char* host, int port) {
    struct sockaddr_in host_addr;
    struct hostent *he;
    int sockfd;

    // get socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Error creating socket");
        exit(1);
    }

    // look up host name
    if ((he=gethostbyname(host)) == NULL) {
        perror("gethostbyname");
        exit(1);
    }

    memset(&(host_addr.sin_zero), '\0', 8);  // zero the rest of the struct
    host_addr.sin_addr = *((struct in_addr *)he->h_addr);

    host_addr.sin_family = AF_INET;    // host byte order
    host_addr.sin_port = htons(port);  // short, network byte order

    // connect
    if (connect(sockfd, (struct sockaddr *)&host_addr,
                sizeof(struct sockaddr)) == -1) {
        cout << "Error connecting to host " << host << " on port "
            << port << endl;
        perror("Error in connect");
        return -1;
    }
    cout << "Connected to " << host << ":" << port << endl;
    return sockfd;
}

/* Listens on the specified TCP port. Returns the port fd. This does not
 * actually accept() any connections (and does not block). Do that elsewhere.
 */
int listenToPort(int port) {
    int sockfd;                     // listening on sock_fd
    struct sockaddr_in my_addr;     // my address information
    int yes = 1;

    // create socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Error creating socket");
        exit(1);
    }

    // allow the port to be reused quickly
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        perror("setsockopt");
        exit(1);
    }
    
    my_addr.sin_family = AF_INET;           // host byte order
    my_addr.sin_port = htons(port);         // short, network byte order
    my_addr.sin_addr.s_addr = INADDR_ANY;   // automatically fill with my IP
    memset(&(my_addr.sin_zero), '\0', 8);   // zero the rest of the struct

    // bind port
    if (bind(sockfd, (struct sockaddr *)&my_addr,
                sizeof(struct sockaddr)) == -1) {
        perror("Error in bind");
        exit(1);
    }

    // Listen. Allow a small backlog of connections to build while we listen 
    if (listen(sockfd, 5) == -1) {
        perror("Error in listen");
        exit(1);
    }
    return sockfd;
}

/* The main program loop that receives data, logs it, and sends relevant
 * chunks to the client programs. Robosock must be an already connected
 * fd to the robot. Clientsock must be a listening socket
 * for client programs to connect to. Logfile must be an already opened log
 * file. sendAndRecieve keeps track of connected clients so their connection
 * will not drop even if the dog connection resets. This should loop forever
 * and returns only on error: zero for an error talking to robosock and one
 * for any other error.
 */
int sendAndReceive(int robosock, int clientsock, ofstream& logfile) {
    int ret;
    static fd_set client_fds;   // client fds, static to allow reconnects to dog
    fd_set read_fds;            // for select()
    static int fd_max = 0;
 
    // First time through we initialise fd_max
    if (fd_max == 0) {
        fd_max = (robosock > clientsock) ? robosock : clientsock;
        FD_ZERO(&client_fds);
    }

    cerr << "sendAndRecieve(): robosock = " << robosock << " clientsock = "
       << clientsock << endl;

    while (true) {

        read_fds = client_fds; // copy client fds
        FD_SET(robosock, &read_fds);    // add robot socket
        FD_SET(clientsock, &read_fds);  // add client socket

        if ((ret = select(fd_max+1, &read_fds, NULL, NULL, NULL)) == -1) {
            perror("select");
            exit(1);
        }
        //cerr << "select has activity on " << ret << " fds" << endl;

        /* Something has happened. Run through the connections to see what */
        for (int i = 0; i <= fd_max; ++i) {
            if (FD_ISSET(i, &read_fds)) {
                if (i == clientsock) { // a new connection
                    struct sockaddr_in remoteaddr; // new client address
                    int addrlen = sizeof(remoteaddr);
                    int newfd = accept(clientsock,
                                        (struct sockaddr *)&remoteaddr,
                                        (socklen_t*)&addrlen);
                    if (newfd == -1) {
                        perror("accept");
                        continue;
                    }
                    FD_SET(newfd, &client_fds);
                    if (newfd > fd_max)
                        fd_max = newfd;
                    cout << "New client connected from "
                        << inet_ntoa(remoteaddr.sin_addr) << " on socket "
                        << newfd << endl;
                } else if (i == robosock) { // data from the robot
                    ret = readFromRobot(robosock, logfile, client_fds, fd_max);
                    if (ret != 0)
                        return ret - 1;
                } else { // data from a client
                    ret = readFromClient(i, robosock, client_fds);
                    if (ret != 0)
                        return 1;
                }

            }
        }


    }    // end of main loop
}

/* Reads a chunk from robosock and writes it to the logfile and to
 * all file descriptors in client_fds, the maximum of which is fd_max.
 * This will block on input if it is not yet available from robosock. Returns
 * zero on success, one if an error occured reading from robosock, or two if an
 * error occured elsewhere. If an error occurs writing to a particular client
 * then that client will be disconnected without an error being returned.
 */
int readFromRobot(int robosock, ofstream& logfile,
                    fd_set& client_fds, int fd_max) {
    int ret;
    unsigned count;
    robolink_header* header = reinterpret_cast<robolink_header*>(recvbuf);

    /* First read a header into buf*/
    count = 0;
    while (count < sizeof(*header)) {
        ret = recv(robosock, recvbuf + count, sizeof(*header) - count, 0);
        if (ret == 0) {
            cerr << "Remote robot closed connection on header recv" << endl;
            return 1;
        } else if (ret < 0) {
            perror("recv");
            return 1;
        }
        count += ret;
    }

    cout << header->magic[0] << header->magic[1]
        << header->magic[2] << header->magic[3]
        << " Chunk: team " << header->team_num
        << ", player " << header->player_num
        << ", frame " << header->frame_num
        << ", type " << header->data_type
        << ", size " << header->data_len << "...";

    /* Check integrity and determine data type, size */
    if (memcmp(header->magic, ROBOLINK_MAGIC, 4) != 0) {
        cout << "Robolink header magic didn't match. Disconnecting."
            << endl;
        return 1;
    }

    /* Read the rest of the data chunk */
    unsigned len = header->data_len;
    if (len > ROBOLINK_BUF_LEN) {
        cout << "WARNING: data length " << len << " greater than "
            << "ROBOLINK_BUF_SIZE = " << ROBOLINK_BUF_LEN << endl;
    }
    char* buf = recvbuf + sizeof(*header);
    count = 0;
    while (count < len) {
        ret = recv(robosock, buf + count, len - count, 0); // MSG_WAITALL?
        if (ret == 0) {
            cerr << "Remote robot closed conneciton on data recv" << endl;
            return 1;
        } else if (ret < 0) {
            perror("recv");
            return 1;
        }
        count += ret;
    }

    cout << "received" << endl;
    
    /* Write header and chunk to log */
    logfile.write(recvbuf, sizeof(*header) + len);
    if (! logfile.good())
        return 2;

    /* Copy chunk to subscribed receivers */
    for (int i = 0; i <= fd_max; ++i) {
        len = header->data_len + sizeof(*header);
        count = 0;
        if (FD_ISSET(i, &client_fds)) {
            //cerr << "Streaming chunk to " << i << endl;
            while (count < len) {
                ret = send(i, recvbuf + count, len - count, 0);
                if (ret < 0) {
                    perror("send()ing to client");
                    cerr << "disconnecting client " << i << endl;
                    // rather than cark it totally, we disconnect this client
                    close(i);
                    FD_CLR(i, &client_fds);
                    break;
                }
                count += ret;
            }
        }
    }
    
    logfile.flush();

    return 0; // success
}

/* Reads a chunk of data from client and sends it to robosock.  Returns zero on
 * success or one if there is an error communicating with the robot. If there
 * is an error communicating with the client (including the client closing the
 * connection) the client socket will be closed and removed from client_fds.
 */
int readFromClient(int client, int robosock, fd_set& client_fds) {
    unsigned char namelen, valuelen;
    char name[NAME_SIZE], value[VALUE_SIZE];
    
    int ret = 0, count = 0;
    int len = sizeof(namelen);
    // This function is black magic since it was simply copied from the old
    // client.cc rather than reimplementing the protocol to not suck.
    
    // Read length of name
    while (count < len) {
        ret = recv(client, &namelen + count, len - count, 0);
        if (ret == 0) {
            cerr << "client closed conneciton on data recv" << endl;
            close(client);
            FD_CLR(client, &client_fds);
            return 0;
        } else if (ret < 0) {
            perror("recv");
            close(client);
            FD_CLR(client, &client_fds);
            return 0;
        }
        count += ret;
    }
    // Read name
    len = namelen;
    count = 0;
    while (count < len) {
        ret = recv(client, name + count, len - count, 0);
        if (ret == 0) {
            cerr << "client closed conneciton on data recv" << endl;
            close(client);
            FD_CLR(client, &client_fds);
            return 0;
        } else if (ret < 0) {
            perror("recv");
            close(client);
            FD_CLR(client, &client_fds);
            return 0;
        }
        count += ret;
    }
    name[namelen] = '\0';
    // Read length of value
    len = sizeof(valuelen);
    count = 0;
    while (count < len) {
        ret = recv(client, &valuelen + count, len - count, 0);
        if (ret == 0) {
            cerr << "client closed conneciton on data recv" << endl;
            close(client);
            FD_CLR(client, &client_fds);
            return 0;
        } else if (ret < 0) {
            perror("recv");
            close(client);
            FD_CLR(client, &client_fds);
            return 0;
        }
        count += ret;
    }
    // Read value
    len = valuelen;
    count = 0;
    while (count < len) {
        ret = recv(client, value + count, len - count, 0);
        if (ret == 0) {
            cerr << "client closed conneciton on data recv" << endl;
            close(client);
            FD_CLR(client, &client_fds);
            return 0;
        } else if (ret < 0) {
            perror("recv");
            close(client);
            FD_CLR(client, &client_fds);
            return 0;
        }
        count += ret;
    }
    value[valuelen] = '\0';

    // Now send the command to all robots
    WDataInfo info;
    WDataInfo nameInfo, valueInfo;
    int totalSent;
    char send_buffer[COMMAND_SEND_BUF_SIZE];

    strcpy(nameInfo.header, WDataHeader);
    nameInfo.robot = 0;
    nameInfo.type = 200;
    nameInfo.size = strlen(name);
    hostToDog(nameInfo);

#if 0
    if (strcmp (name, "ip") == 0){
        for (int x=0; x<countPlayer; x++){
            sprintf(value, "%d", x+1);

            strcpy(valueInfo.header, WDataHeader);
            valueInfo.robot  = 0;
            valueInfo.type   = 201;
            valueInfo.size   = strlen(value);
            hostToDog(valueInfo);
            PackageDef pack  = PackageDef(name, nameInfo, value, valueInfo);
            strcpy(info.header, WDataHeader);
            info.robot = 0;
            info.type  = 200;
            info.size  = sizeof(pack);
            hostToDog(info);

            memcpy(send_buffer, &info, sizeof(WDataInfo));
            memcpy(send_buffer+sizeof(WDataInfo), &pack, sizeof(pack));

            totalSent = sizeof(pack) + sizeof(WDataInfo);

            cout << "CLIENT assigning " << robotIp[x] << " IP = " << (x+1) << endl;
            result = clientSend(crossid[x], send_buffer, totalSent);

            if (result != 0) {
                perror("write on cross_id : " + x);
                close(crossid[x]);
                exit(1);
            }
        }
    } else {
#endif
    strcpy(valueInfo.header, WDataHeader);
    valueInfo.robot = 0;
    valueInfo.type  = 201;
    valueInfo.size  = strlen(value);
    hostToDog(valueInfo);
    PackageDef pack = PackageDef(name, nameInfo, value, valueInfo);
    strcpy(info.header, WDataHeader);
    info.robot = 0;
    info.type  = 200;
    info.size  = sizeof(pack);
    hostToDog(info);

    memcpy(send_buffer, &info, sizeof(WDataInfo));
    memcpy(send_buffer+sizeof(WDataInfo), &pack, sizeof(pack));

    totalSent = sizeof(pack) + sizeof(WDataInfo);
    if (totalSent != COMMAND_SEND_BUF_SIZE) {
        cout << "CLIENT: Error totalSent!=COMMAND_SEND_BUFFER_MAX_SIZE, "
            << totalSent << " != " << COMMAND_SEND_BUF_SIZE
            << "." << endl;
    }

    count = 0;
    len = totalSent;
    while (count < len) {
        ret = send(robosock, send_buffer + count, len - count, 0);
        if (ret < 0) {
            perror("send()ing to robot");
            return 1;
        }
        count += ret;
    }
#if 0
    }
#endif

    return 0;
}
 
void usage(const char* progname) {
    cout << "Usage: " << progname << " host" << endl;
    exit(1);
}
