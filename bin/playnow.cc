/* Sends one wireless packet over UDP mimicing the GameController.
 * The packet sets the state to playing.
 */

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <iostream>

typedef unsigned int uint32;

#include "../robot/share/RoboCupGameControlData.h"
#include "../robot/GameController/GameControllerConfig.h"

using namespace std;

int main(int argc, char* argv[]) {
    struct sockaddr_in bcast_addr, my_addr;
    struct hostent *he;
    int sockfd;
    int teamnum = 1;

    RoboCupGameControlData data;

    // check command line arguments for a team number

    // set up the struct info
    memcpy(data.header, STRUCT_HEADER, sizeof(data.header));
    data.version = STRUCT_VERSION;
    data.state = STATE_PLAYING;
    data.firstHalf = 1;
    data.kickOffTeam = TEAM_RED;
    data.secsRemaining = 600;
    data.dropInTeam = TEAM_RED;
    data.dropInTime = 0;
    // team info
    data.teams[0].teamNumber = teamnum;
    data.teams[0].teamColour = TEAM_RED;
    data.teams[0].score = 0;
    data.teams[0].players[0].penalty = PENALTY_NONE;
    data.teams[0].players[0].secsTillUnpenalised = 0;
    data.teams[0].players[1].penalty = PENALTY_NONE;
    data.teams[0].players[1].secsTillUnpenalised = 0;
    data.teams[0].players[2].penalty = PENALTY_NONE;
    data.teams[0].players[2].secsTillUnpenalised = 0;
    data.teams[0].players[3].penalty = PENALTY_NONE;
    data.teams[0].players[3].secsTillUnpenalised = 0;
    data.teams[1].teamNumber = teamnum;
    data.teams[1].teamColour = TEAM_RED;
    data.teams[1].score = 0;
    data.teams[1].players[0].penalty = PENALTY_NONE;
    data.teams[1].players[0].secsTillUnpenalised = 0;
    data.teams[1].players[1].penalty = PENALTY_NONE;
    data.teams[1].players[1].secsTillUnpenalised = 0;
    data.teams[1].players[2].penalty = PENALTY_NONE;
    data.teams[1].players[2].secsTillUnpenalised = 0;
    data.teams[1].players[3].penalty = PENALTY_NONE;
    data.teams[1].players[3].secsTillUnpenalised = 0;

    // get socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("Error creating socket");
        exit(1);
    }

    /* bind any port */
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    my_addr.sin_port = htons(0);

    if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(my_addr))) {
        perror("Error binding");
        exit(1);
    }
    cout << "Bound to port " << my_addr.sin_port << endl;


    // look up host name
    if ((he=gethostbyname("255.255.255.255")) == NULL) {
        perror("gethostbyname");
        exit(1);
    }

    memset(&(bcast_addr.sin_zero), '\0', 8); 
    bcast_addr.sin_addr = *((struct in_addr *)he->h_addr);
    bcast_addr.sin_family = AF_INET; 
    bcast_addr.sin_port = htons(GAMECONTROLLER_PORT); 

    /* send buffer contents */
    if (sendto(sockfd, &data, sizeof(data), 0, (struct sockaddr *)&bcast_addr, 
          sizeof(bcast_addr)) != sizeof(data)) {
        perror("Error sending data");
        return 1;
    }

    return 0;
}
