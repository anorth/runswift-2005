#include <stdio.h>
#include <vector>
#include <string>
#include "BinStream.h"
#include "DSocket.h"
#include <ncurses.h>
#include <signal.h>
#include <time.h>

class Robot;

class RobotList
{
   public:

      RobotList();
      void process_packet(UDPPacket *packet);
      std::vector<Robot *> list;
};

class Robot
{
   public:
      Robot(std::string ip, int playernum, int team);
      void setpos(float posx, float posy, float posh, float posvar);
      void setball(double ballx, double bally, double ballvar);

      std::string ip;
      int playernum, team;
      float posx, posy, posh, posvar;
      double ballx, bally, ballvar;
      bool just_updated;
      time_t updated;
};
