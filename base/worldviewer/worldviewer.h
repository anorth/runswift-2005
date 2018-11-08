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


#include <qapplication.h>
#include <qpushbutton.h>
#include <qcanvas.h>
#include <qdockwindow.h>
#include <qtable.h>
#include <qmainwindow.h>
#include <qlayout.h>
#include <qgl.h>
#include <qlabel.h>
#include <qdialog.h>
#include <qlistbox.h>
#include <qmenubar.h>
#include <qlineedit.h>
#include <qtextedit.h>
#include <qvalidator.h>
#include <qsettings.h>
#include <qpixmap.h>
#include <qcombobox.h>
#include <qsignalmapper.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>

#include <iostream> 
#include <vector>
#include "robots.h"
#include "drawfield.h"
#include "gltext.h"

#define WM_DEBUG

//max teams to show wireless ball for;
//i.e. for 10 teams, wireless ball will be
//highlighted for teams 0 to 9 (if they exist)
//very much a hack, but also very much makes showing wireless easier
//make this zero to turn of wireless ball highlighting
#define MAX_SHARE_BALL_TEAMS 10

typedef struct _RobotOption
{
   bool show_ball;
   bool show_ball_var;
   bool show;
   bool show_var;
   bool show_obstacle;
} RobotOption;

enum fliptype {UP, RIGHT, DOWN, LEFT};

// Role constants..
enum {
   NOROLE           = 0,
   FACEBACKOFF      = 1,
   ATTACKER         = 2,
   WINGER           = 3,
   ATTACKBACKOFF    = 4,
   GETBEHINDBACKOFF = 5,
   SUPPORTER        = 6,
   SIDEBACKOFF      = 7,
   SUPPORTBACKOFF   = 8,
   DEFENDER         = 9,
   DEFENDERGOALBOX  = 10,
   STEALTHDOG       = 11,
   STUCKER          = 12,
   GOALIE           = 13,
   GOALIEATTACK     = 14,
   GOALIEDEFENDSEEB = 15,
   GOALIEDEFENDNOTS = 16,
   BACKOFF          = 17
};

typedef struct _DisplayOption
{
   bool oddIsRed;
   bool show_text;
   int orientation;
} DisplayOption;

class wvInfoDockWindow : public QDockWindow
{
   Q_OBJECT
   public:
      wvInfoDockWindow(QWidget *parent, QString name="Robot Info", WFlags f = 0);
      int get_robot();
      void select_robot(int);

      void add_robot(int);
      void remove_robot(int);
      
   private:
      QComboBox *combobox;
      QTextEdit *textedit;
      std::map<int, QString> id_map;
      std::map<QString, int> str_map;
      void timerEvent( QTimerEvent * );
      
   public slots:
      void robot_added(int);
      void robot_removed(int);

   protected:
      void hideEvent(QHideEvent *);
      void closeEvent(QCloseEvent *);
};

class wvAddPortWindow : public QDialog
{
   Q_OBJECT
   public:
      wvAddPortWindow(QWidget *parent, QString name="Add port");
      QLineEdit *port_edit;
   private slots:
      void edit_ip(const QString &);
      void edit_port(const QString &);
   private:
      QLineEdit *ip_edit;
      QPushButton *ok;
      QPushButton *cancel;
};

class wvPortWindow : public QDialog
{
   Q_OBJECT
   public:
      wvPortWindow(QWidget *parent, QString name="Edit ports");
   private slots:
      void add_port();
      void remove_port();
      void highlight(int);
      
   private:
      QListBox *listbox;
      QPushButton *add;
      QPushButton *remove;
      QPushButton *ok;
      QPushButton *cancel;
};

class wvTable : public QTable
{
   public:
      wvTable(int rows, int cols, QWidget *parent);
      void adjustColumns();
   protected:
      void resizeEvent(QResizeEvent *e);
};

class wvTableDockWindow : public QDockWindow
{
   Q_OBJECT
   public:
      wvTableDockWindow( QWidget * parent, const char * name = 0, WFlags f = 0 );
      ~wvTableDockWindow();
      QTable *get_table();

   public slots:
      void value_changed(int, int);
      void robot_added(int);
      void robot_removed(int);
      //void do_update();

   signals:
      void show_ball(int, bool);
      void show_robot(int, bool);
      void show_robot_var(int, bool);
      void show_ball_var(int, bool);
      void show_obstacle(int, bool);
      
   private:
      QTable *table;
      QSignalMapper *info_sigmap;
};

class wvMainWindow : public QMainWindow
{
   Q_OBJECT
   public:
      wvMainWindow(QWidget *parent=0, const char *name=0, WFlags f=0);
      ~wvMainWindow();

      std::map<int, RobotOption> get_robot_options();
      DisplayOption get_display_options();

   public:
      QPixmap now_pix;
      QPixmap old_pix;
      QPixmap recent_pix;
      RobotList *robotlist;
      
      int shareBallSourceBot[MAX_SHARE_BALL_TEAMS];
      double shareBallVar[MAX_SHARE_BALL_TEAMS];
      
      bool allow_update;
      static wvMainWindow *instance;
      std::vector<wvInfoDockWindow *> info_list;

   signals:
      void add_robot(int id);
      void remove_robot(int id);
      void change_odd_red(bool oddIsRed);
      void change_show_text(bool show_text);
      
   public slots:
      void do_config_ports();
      void do_odd_red();
      void do_show_text();
      void before_quit();
      void show_info(int id);

      void show_robot(int, bool);
      void show_ball(int, bool);
      void show_robot_var(int, bool);
      void show_ball_var(int, bool);
      void show_obstacle(int, bool);
      
      void rotate_left();
      void rotate_right();

      void robot_added(int);
      void robot_removed(int);

   private:
      void timerEvent( QTimerEvent * );
      void update_table();
      void load_settings();
      void save_settings();
      void set_odd_red(bool odd_red);
      void set_show_text(bool show_text);

      QGLWidget *glview;
      QMenuBar *menu;
      wvTableDockWindow *tabledock;
      QPopupMenu *settings_menu;

      std::map<int, RobotOption> robot_options;
      DisplayOption display_options;

      int oddIsRed_id;
      int showText_id;
};


class wvGLWidget : public QGLWidget
{
   Q_OBJECT
   public:
      wvGLWidget( QWidget *parent=0, const char *name=0 );
   public slots:
      void add_robot(int id);
      void remove_robot(int id);
   protected:
      void initializeGL();
      void resizeGL( int, int );
      void paintGL();
      void timerEvent( QTimerEvent * );
      GLColour get_role_colour(Robot *r);
      void draw();

      int time_interval;

      GLField field;
      std::map<int, GLRobot *> gl_robots;
      std::map<int, GLBall *> gl_balls;
};
