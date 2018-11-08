/* Sends a teamtalk packet to the specified ip (which determines team, dog)
 * toggling the argument-specified debug streams
 */

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <iostream>

//typedef unsigned int uint32;

#include "../robot/share/robolink.h"

using namespace std;

enum { MSG_LEN = 19, IP_LEN = 16 };

void usage(void);

char *default_ip = "10.1.2.";
unsigned short port_base = 10200; // port = port_base + 10 * teamnum

int main(int argc, char* argv[]) {
    struct sockaddr_in bcast_addr, my_addr;
    struct hostent *he;
    //char ip[IP_LEN];
    int sockfd;
    unsigned short port;
    char team = '0', player = '0';

    // check command line arguments for a player/team number
    if (argc < 3) {
        usage();
        exit(1);
    }
    
    if (strlen(argv[1]) < 2 || strlen(argv[1]) > 3) {
        usage();
        exit(1);
    }

    team = argv[1][strlen(argv[1]) - 2];
    player = argv[1][strlen(argv[1]) - 1];
    port = port_base + (10 * (team - '0'));

#if 0
    if (strlen(argv[1]) <= 3) { // specified IP suffix only
        strncpy(ip, default_ip, IP_LEN);
        strncpy(ip + strlen(default_ip), argv[1], 3);
    } else {
        strncpy(ip, argv[1], IP_LEN);
    }
#endif

    //cerr << "Sending to team " << team << " player " << player << endl;

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
    
    // set broadcast flag
    int one = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST,
                (char *) &one, sizeof(one));

    // look up host name
    if ((he=gethostbyname("255.255.255.255")) == NULL) {
        perror("gethostbyname");
        exit(1);
    }

    memset(&(bcast_addr.sin_zero), '\0', 8); 
    bcast_addr.sin_addr = *((struct in_addr *)he->h_addr);
    bcast_addr.sin_family = AF_INET; 
    bcast_addr.sin_port = htons(port); 

    /* send packets */
    
    for (int i = 2; i < argc; ++i) {
        // set up the packet
        char msg[MSG_LEN]; // see share/CommandData.h for info
        strncpy(msg, "NStt", 4);
        *((int *)(msg + 4)) = MSG_LEN - 8;
        strncpy(msg + 8, "tt__debug_", MSG_LEN - 8);
        msg[8 + 2] = team;
        msg[8 + 3] = player;
        msg[8 + 9] = (char)atoi(argv[i]);
        msg[8 + 10] = '\0';

        if (sendto(sockfd, &msg, MSG_LEN, 0, (struct sockaddr *)&bcast_addr, 
              sizeof(bcast_addr)) != MSG_LEN) {
            perror("Error sending data");
            exit(1);
        }
	for (unsigned delay = 0; delay < 10000000; ++delay) {}
    }

    return 0;
}

void usage(void) {
    char *msg =
        "Usage: sendData IPSuffix dataType [dataType...]\n"
        "IPSuffix must be two or three digits. Second last digit is team num.\n"
        "Last digit is player num.\n"
        "dataTypes:\n"
        "    1 = RLNK_DEBUG\n"
        "    2 = RLNK_CPLANE\n"
        "    3 = RLNK_YUVPLANE\n"
        "    4 = RLNK_CAM_ORIENT\n"
        "    5 = RLNK_SENSOR\n"
        "    6 = RLNK_SUBVISION\n"
        "    7 = RLNK_SUBOBJECT\n"
        "    8 = RLNK_INSANITY\n"
        "    9 = RLNK_PWALK\n"
        "    10 = RLNK_ATOMIC_ACTION\n"
        "    11 = RLNK_SUBCPLANE\n"
        "    12 = RLNK_DOGID\n"
        "    13 = RLNK_GPSINFO\n"
        "    14 = RLNK_BESTGAP\n"
        "    15 = RLNK_PINKOBJECT\n"
        "    16 = RLNK_GAUSSIANS\n"
        "    17 = RLNK_DROPFRAME (telnet)\n"    
        "    18 = RLNK_FRAME (telnet)\n";    
        
    fprintf(stderr, msg);
}
