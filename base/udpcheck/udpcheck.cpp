#include "udpcheck.h"

// WARNING: THIS FILE IS A MESS, YOU HAVE BEEN WARNED (thrown up in 1.5 days)

bool robotLessThan(Robot *a, Robot *b)
{
   if (a->team < b->team)
      return true;
   else if (a->team == b->team)
      return a->playernum < b->playernum;
   else
      return false;
}

Robot::Robot(std::string ip, int playernum, int team)
{
   this->ip = ip;
   this->playernum = playernum;
   this->team = team;
}

void Robot::setpos(float posx, float posy, float posh, float posvar)
{
   this->posx = posx;
   this->posy = posy;
   this->posh = posh;
   this->posvar = posvar;
}

void Robot::setball(double ballx, double bally, double ballvar)
{
   this->ballx = ballx;
   this->bally = bally;
   this->ballvar = ballvar;
}

RobotList::RobotList()
{
   list.clear();
}

void RobotList::process_packet(UDPPacket *packet)
{
   char header[5] = {'\0'};
   int playerNum;
   int team;
	float posx, posy;
	float posh;
	float posVar, hVar;
   uint32_t behavioursVal;

	double ballx;
	double bally;
	double ballvar;

   
   packet->set_network_order(false); // AIBO's don't use network order to transmit,
                                     // and they're little endian like us intels

   packet->rewind();
   packet->read(header, 4);

   if (strcmp(header, "NSwm") != 0)
      return;

   packet->read(&playerNum);
   packet->read(&team);

   Robot *rp = NULL;
   
   std::string ip = packet->get_addr_str();
   std::vector<Robot *>::iterator i;

   // Search for robot
   for (i = list.begin(); i != list.end(); i++)
   {
      Robot *r = *i;
      if ((r->ip == ip) && (r->playernum == playerNum) && (r->team == team))
      {
         rp = (*i);
         break;
      }
   }

   // Didn't find robot?
   if (rp == NULL)
   {
      rp = new Robot(ip, playerNum, team);

      list.push_back(rp);

      sort(list.begin(), list.end(), robotLessThan);
   }

   //printf("header %s, playernum %d, team %d\n", header, playerNum, team);

   packet->seek(4, SEEK_CUR);
   packet->read(&posx);
   packet->read(&posy);
   packet->read(&posh);
   packet->read(&posVar);
   packet->read(&hVar);
   packet->read(&behavioursVal);
   packet->read(&ballx);
   packet->read(&bally);
   packet->read(&ballvar);

   rp->setpos(posx, posy, posh, posVar);
   rp->setball(ballx, bally, ballvar);
   
   time(&(rp->updated));
   rp->just_updated = true;
   
   //printf("posx %f, posy %f, posh %f, posvar %f, hvar %f\n", posx, posy, posh, posVar, hVar);
   //printf("behavioursVal %u\n", behavioursVal);
   //printf("ballx %f, bally %f, ballvar %f\n", ballx, bally, ballvar);

}

static void finish(int sig);

void ncurses_init()
{
   signal(SIGINT, finish);
   initscr();
   cbreak();
   noecho();
   nodelay(stdscr, TRUE);
   use_default_colors();
   start_color();
   curs_set(0);

   init_pair(1, COLOR_RED, -1);
   init_pair(2, COLOR_BLUE, -1);
}

static void finish(int sig)
{
   endwin();

   printf("dying...gasp...\n");
   exit(0);
}

bool teamColour;
void display(RobotList *rl)
{
   erase();

   move(0,0);
   if (rl->list.size() == 1)
      printw("Seen 1 robot...\n\n", rl->list.size());
   else
      printw("Seen %d robots...\n\n", rl->list.size());
   
   time_t now;
   time(&now);

   attron(A_BOLD);
   printw(" IP address    | P | T | Posx, Posy, Posh, Posvar | Ballx, bally, ballvar... | Time\n");
   printw("\n");
   attroff(A_BOLD);
   

   std::vector<Robot *>::iterator i;
   for (i = rl->list.begin(); i != rl->list.end(); i++)
   {
      Robot *r = *i;
      double diff = difftime(now, r->updated);

      // Decide which team is red and which is blue
      int colour_pair = teamColour ? 2 - (r->team % 2) : (r->team % 2) + 1;

      attron(COLOR_PAIR(colour_pair));

      printw(" %-15s %d   %d   %4.0f  %4.0f  %4.0f  %5.0f      %4.0f   %4.0f  %-10.0f     %2.0f\n", \
            r->ip.c_str(), r->playernum, r->team, r->posx, r->posy, r->posh, r->posvar, r->ballx, r->bally, r->ballvar, diff);

      if (r->just_updated)
      {
         r->just_updated = false;
      }

      attroff(COLOR_PAIR(colour_pair));
   }

   attron(A_BOLD);
   mvhline(1,0,ACS_HLINE,COLS);
   mvhline(3,0,ACS_HLINE,COLS);
   mvvline(2,15,ACS_VLINE,LINES);
   mvvline(2,19,ACS_VLINE,LINES);
   mvvline(2,23,ACS_VLINE,LINES);
   mvvline(2,50,ACS_VLINE,LINES);
   mvvline(2,77,ACS_VLINE,LINES);

   mvaddch(3,15,ACS_PLUS);
   mvaddch(3,19,ACS_PLUS);
   mvaddch(3,23,ACS_PLUS);
   mvaddch(3,50,ACS_PLUS);
   mvaddch(3,77,ACS_PLUS);
   
   mvaddch(1,15,ACS_TTEE);
   mvaddch(1,19,ACS_TTEE);
   mvaddch(1,23,ACS_TTEE);
   mvaddch(1,50,ACS_TTEE);
   mvaddch(1,77,ACS_TTEE);
   attroff(A_BOLD);


   for (i = rl->list.begin(); i != rl->list.end(); i++)
   {
      Robot *r = *i;
      double diff = difftime(now, r->updated);
      
      // Haven't touched for 60 seconds?
      if (diff > 60)
      {
         delete(r);
         rl->list.erase(i);

	 // Couldn't get erase to happen properly..so have do this in a dodgy way
         i = rl->list.begin();
         i--;
         continue;
      }
   }
}

int main(int argc, char *argv[])
{
   int result;
   std::vector<UDPSocket *> sockets;
  
   if (argc <= 1)
   {
      printf("Usage: udpcheck ip [ip...]\n");
      printf("   \"ip\" should be the last part of the ip, i.e. number used in spip\n");
      printf("   Only one ip from each team is needed\n");
      exit(0);
   }

   int i;
   for (i = 1; i < argc; i++)
   {
      int ip = strtol(argv[i], NULL, 10);
      int port = (ip / 10) * 3 + 11900;

      if (ip == 0)
      {
         printf("Couldn't read IP %s properly?\n", argv[i]);
         exit(0);
      }

      UDPSocket *s = new UDPSocket;
      if (s->bind(INADDR_ANY, port))
      {
         sockets.push_back(s);
         printf("Added socket on port %d\n", port);
      }
   }
   
   RobotList rl;
   
   ncurses_init();

   char key;
   
   //printf("Waiting for UDP packet");
   while (1)
   {
      std::vector<UDPSocket *>::iterator it;

      UDPSocket *s;
      
      for (it = sockets.begin(); it != sockets.end(); it++)
      {
         s = *it;
         while (s->select(0))
         {
            UDPPacket packet(1200);

            result = s->receive(&packet);

            if (result == UDPSocket::UDPSOCKET_ERROR)
            {
               printf("We get error! %d: %s\n", s->get_error(), s->get_error_str());
            }

            rl.process_packet(&packet);
         }
      }

      display(&rl);

      key = getch();
      if (key == 27)
         break;
      else if (key == 's')
         teamColour = !teamColour;
   }

   endwin();
   printf("Noo...don't leave me :(\n");
   return 0;
}
