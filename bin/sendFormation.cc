/* Sends a teamtalk packet to the specified team
 * setting their playing formation
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

void usage(void);

char *default_ip = "192.168.0.";
unsigned short port_base = 10000; // port = port_base + 10 * teamnum

int main(int argc, char* argv[]) {
    struct sockaddr_in bcast_addr, my_addr;
    struct hostent *he;
    //char ip[IP_LEN];
    int sockfd;
    unsigned short port;
    char team = '0'; 
    char* formation;

    // check command line arguments for a team number
    if (argc < 3) {
        usage();
        exit(1);
    }

    if (strlen(argv[1]) > 1) {
        usage();
        exit(1);
    }    
        
    // get data from command line args
    team = argv[1][0];
    formation = argv[2];
    port = port_base + (10 * (team - '0'));

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

    /* send packet */
    
    // set up the packet
    
    int sizelen = 4;
    int nowAt = 0;
    
    char* head1 = "NStt";
    char* head2 = "tt__forma";
    
    int msg_len = strlen(head1) + sizelen + strlen(head2) + strlen(formation) + 1; 
    
    char msg[msg_len]; // see share/CommandData.h for info
    
    strncpy(msg + nowAt, head1, strlen(head1));
    nowAt = nowAt + strlen(head1);
    
    *((int *)(msg + nowAt)) = msg_len - strlen(head1) - sizelen;
    nowAt = nowAt + sizelen;
    
    strncpy(msg + nowAt, head2, strlen(head2));
    msg[nowAt + 2] = team;
    msg[nowAt + 3] = '0'; //player number... broadcast all
    nowAt = nowAt + strlen(head2);
    
    strncpy(msg + nowAt, formation, strlen(formation));
    nowAt = nowAt + strlen(formation);
    
    msg[nowAt] = '\0';

    //fprintf(stderr, msg);
    //fprintf(stderr, "\n");
    
    if (sendto(sockfd, &msg, msg_len, 0, (struct sockaddr *)&bcast_addr, 
            sizeof(bcast_addr)) != msg_len) {
        perror("Error sending data");
        exit(1);
    }
    
    
    return 0;
}

void usage(void) {
    char *msg =
        "Usage: sendFormation teamNum formationName\n"
        "\n"
        "examples:\n"
        " sendFormation 5 fAttSupDef\n"
        " sendFormation 3 fAttStrDef\n";
        
    fprintf(stderr, msg);
}
