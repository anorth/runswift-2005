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


#include "worldviewer.moc"
#include "now.xpm"
#include "recent.xpm"
#include "old.xpm"
#include "rotleft.xpm"
#include "rotright.xpm"
#include <stdio.h>
#define OSAKA
char *roleToString(int role)
{
   static char *rolestring[] = {
      "No Role",
      "Face (back off)",
      "Attacker",
      "Winger",
      "Attacker (back off)",
      "Get behind (back off)",
      "Supporter",
      "Side (back off)",
      "Support (back off)",
      "Defender",
      "Defender (Goal Box)",
      "Stealth dog",
      "Stucker",
      "Goalie",
      "Goalie attack",
      "Goalie defend (see ball)",
      "Goalie defend (don't see ball)",
      "Back off"
   };

   if (role > 17)
      return "Invalid role?";
   else
      return rolestring[role];
}

// *******************************************************************************

wvMainWindow *wvMainWindow::instance = NULL;

wvMainWindow::wvMainWindow(QWidget * parent, const char *name, WFlags f) 
   : QMainWindow(parent, name, f)
{
   if (instance == NULL)
      instance = this;

   glview = new wvGLWidget(this);
   tabledock = new wvTableDockWindow(this);

   QPopupMenu *file = new QPopupMenu( this );
   file->insertItem("E&xit", qApp, SLOT(quit()));

   settings_menu = new QPopupMenu( this );
   settings_menu->insertItem("&Ports...", this, SLOT(do_config_ports()));
   oddIsRed_id = settings_menu->insertItem("&Odd teams are RED", this, SLOT(do_odd_red()));
   showText_id = settings_menu->insertItem("&Show text", this, SLOT(do_show_text()));
   
   //initialise shared ball array
   for(int i = 0; i < MAX_SHARE_BALL_TEAMS; i++)
   {
        shareBallSourceBot[i] = 900000000;
        shareBallVar[i] = 900000000;
   }
   
   menu = new QMenuBar(this);
   menu->insertItem( "&File", file);
   menu->insertItem( "&Settings", settings_menu);

   QToolBar *toolbar = new QToolBar( this );
   QPixmap rotleft_pix(rotleft_xpm);
   QToolButton *rotleft = new QToolButton(rotleft_pix, "Rotate Left", QString::null, this, SLOT(rotate_left()), toolbar);
   QPixmap rotright_pix(rotright_xpm);
   QToolButton *rotright = new QToolButton(rotright_pix, "Rotate Left", QString::null, this, SLOT(rotate_right()), toolbar);

   moveDockWindow( tabledock, Bottom);

   tabledock->show();
   
   setCentralWidget(glview);

   now_pix = QPixmap(now_xpm);
   old_pix = QPixmap(old_xpm);
   recent_pix = QPixmap(recent_xpm);
   allow_update = true;
   robotlist = new RobotList();

   load_settings();
   
   connect(qApp,SIGNAL(aboutToQuit()),this,SLOT(before_quit()));
   connect(tabledock,SIGNAL(show_robot(int, bool)),this,SLOT(show_robot(int, bool)));
   connect(tabledock,SIGNAL(show_ball(int, bool)),this,SLOT(show_ball(int, bool)));
   connect(tabledock,SIGNAL(show_ball_var(int, bool)),this,SLOT(show_ball_var(int, bool)));
   connect(tabledock,SIGNAL(show_robot_var(int, bool)),this,SLOT(show_robot_var(int, bool)));
   connect(tabledock,SIGNAL(show_obstacle(int, bool)),this,SLOT(show_obstacle(int, bool)));
   connect(this, SIGNAL(add_robot(int)), glview,SLOT(add_robot(int)));
   connect(this, SIGNAL(remove_robot(int)), glview,SLOT(remove_robot(int)));
   connect(this, SIGNAL(add_robot(int)),this,SLOT(robot_added(int)));
   connect(this, SIGNAL(remove_robot(int)),this,SLOT(robot_removed(int)));
   connect(this, SIGNAL(add_robot(int)),tabledock,SLOT(robot_added(int)));
   connect(this, SIGNAL(remove_robot(int)),tabledock,SLOT(robot_removed(int)));
   
   startTimer(1);

}

wvMainWindow::~wvMainWindow()
{
   if (glview != NULL)
      delete glview;

   if (robotlist != NULL)
      delete robotlist;
}

void wvMainWindow::timerEvent( QTimerEvent *t)
{
   if (allow_update == true)
      update_table();
}

void wvMainWindow::update_table()
{
   // Table updates happen here...
   robotlist->update();

   QTable *t = tabledock->get_table();

   std::set<int> removed = robotlist->get_removed();
   std::set<int> added = robotlist->get_added();
   std::map<int, Robot *> robots = robotlist->get_robot_list();

   // Remove rows from table
   int i;
   std::set<int>::iterator it;
   for (it = removed.begin(); it != removed.end(); it++)
   {
      int id = *it;
      emit remove_robot(id);
   }

   // Add rows to table

   for (it = added.begin(); it != added.end(); it++)
   {
      int id = *it;

      emit add_robot(id);
   }


   for (i = 0; i < t->numRows(); i++)
   {
      // Set pixmap
      int id = t->text(i,0).toInt();
      long elapsed = robots[id]->current_time;
      long prev_elapsed = robots[id]->prev_time;
      if ((elapsed < 200 && prev_elapsed < 0) || (elapsed < 200 && prev_elapsed >= 200))
         t->setPixmap(i,1, now_pix);
      else if (elapsed >= 200 && prev_elapsed < 200)
         t->setPixmap(i,1, recent_pix);
      else if (elapsed >= 2000 && prev_elapsed < 2000)
         t->setPixmap(i,1,old_pix);

      t->setText(i,10,QString::number(elapsed));
   }

   if (added.size() > 0)
   {
      ((wvTable *)t)->adjustColumns();
   }
}

void wvMainWindow::do_config_ports()
{
   wvPortWindow *dlg = new wvPortWindow(this);
   dlg->exec();
}

void wvMainWindow::do_odd_red()
{
   set_odd_red(!display_options.oddIsRed);
}

void wvMainWindow::do_show_text()
{
   set_show_text(!display_options.show_text);
}

void wvMainWindow::set_odd_red(bool odd_red)
{
   display_options.oddIsRed = odd_red;
   
   settings_menu->setItemChecked(oddIsRed_id, display_options.oddIsRed);

   emit change_odd_red(odd_red);
}

void wvMainWindow::set_show_text(bool show_text)
{
   printf("Changing showtext to %d!\n", show_text);
   
   display_options.show_text = show_text;

   settings_menu->setItemChecked(showText_id, display_options.show_text);

   emit change_show_text(show_text);
}

void wvMainWindow::robot_added(int id)
{
   RobotOption new_option;
   robot_options[id] = new_option;
   robot_options[id].show_ball = true;
   robot_options[id].show_ball_var = true;
   robot_options[id].show = true;
   robot_options[id].show_var = true;
   robot_options[id].show_obstacle = false;
}

void wvMainWindow::robot_removed(int id)
{
   robot_options.erase(id);
}

void wvMainWindow::show_info(int id)
{
#ifdef WM_DEBUG
   printf("Showing info for robot %d\n", id);
#endif

   // First, search through all info dock windows
   std::vector<wvInfoDockWindow *>::iterator idt;

   for (idt = info_list.begin(); idt != info_list.end(); idt++)
   {
      wvInfoDockWindow *w = *idt;
      if (w->get_robot() == id)
      {
         w->setFocus();
         return;
      }
   }
   
   
   // Otherwise, display window here
   wvInfoDockWindow *info_win = new wvInfoDockWindow(this);
   moveDockWindow(info_win, Right);

   // Populate it with robot list
   std::map<int, Robot *>robots = robotlist->get_robot_list();
   std::map<int, Robot *>::iterator it;

   for (it = robots.begin(); it != robots.end(); it++)
   {
      info_win->add_robot(it->first);
   }

   info_win->select_robot(id);

   connect(this, SIGNAL(add_robot(int)), info_win, SLOT(robot_added(int)));
   connect(this, SIGNAL(remove_robot(int)), info_win, SLOT(robot_removed(int)));
   
   info_win->show();

   info_list.push_back(info_win);
}

void wvMainWindow::load_settings()
{
   QSettings settings;
   settings.setPath("rUNSWift","worldviewer");

   QString port_string = settings.readEntry("/settings/ports","");
#ifdef WM_DEBUG
   printf("Loading port settings: %s\n", port_string.ascii());
#endif
   QStringList port_stringlist(QStringList::split( ",", port_string));
   QStringList::Iterator it;
   for (it = port_stringlist.begin(); it != port_stringlist.end(); it++)
   {
      bool success;
      int port = (*it).toInt(&success);

      if (success)
      {
         robotlist->add_socket(port);
#ifdef WM_DEBUG
         printf("Loading socket %d from settings\n", port);
#endif
      }
   }

   display_options.oddIsRed = settings.readBoolEntry("settings/oddIsRed", false);
   set_odd_red(display_options.oddIsRed);

   display_options.show_text = settings.readBoolEntry("settings/showText", true);
   set_show_text(display_options.show_text);
   
   display_options.orientation = settings.readNumEntry("settings/orientation", 0);
}


void wvMainWindow::save_settings()
{
   QSettings settings;
   settings.setPath("rUNSWift","worldviewer");
   
   // Fill window
   std::set<int> port_list = wvMainWindow::robotlist->get_ports();
   std::set<int>::iterator it;

   QString port_string;

   for (it = port_list.begin(); it != port_list.end(); it++)
   {
      port_string += QString::number(*it) + ",";
#ifdef WM_DEBUG
      printf("Saving port %d to settings\n", *it);
#endif
   }

   settings.writeEntry("/settings/ports",port_string);
#ifdef WM_DEBUG
   printf("Wrote port setting: %s\n", port_string.ascii());
#endif

   settings.writeEntry("settings/oddIsRed", display_options.oddIsRed);
   settings.writeEntry("settings/showText", display_options.show_text);
   settings.writeEntry("settings/orientation", display_options.orientation);
}

void wvMainWindow::show_robot(int id, bool show)
{
#ifdef WM_DEBUG
   printf("Set show robot %d to %i\n",id, show);
#endif
   robot_options[id].show = show;
}

void wvMainWindow::show_ball(int id, bool show)
{
#ifdef WM_DEBUG
   printf("Set show robot ball %d to %i\n",id, show);
#endif
   robot_options[id].show_ball = show;
}

void wvMainWindow::show_robot_var(int id, bool show)
{
#ifdef WM_DEBUG
   printf("Set show robot var %d to %i\n",id, show);
#endif
   robot_options[id].show_var = show;
}

void wvMainWindow::show_ball_var(int id, bool show)
{
#ifdef WM_DEBUG
   printf("Set show ball var %d to %i\n",id, show);
#endif
   robot_options[id].show_ball_var = show;
}

void wvMainWindow::show_obstacle(int id, bool show)
{
#ifdef WM_DEBUG
   printf("Set show obstacle %d to %i\n",id, show);
#endif
   robot_options[id].show_obstacle = show;
}

void wvMainWindow::rotate_left()
{
#ifdef WM_DEBUG
   printf("Rotating left!\n");
#endif
   display_options.orientation = (display_options.orientation - 1) % 4;

   if (display_options.orientation < 0)
   {
      display_options.orientation += 4;
   }

#ifdef WM_DEBUG
   switch (display_options.orientation)
   {
      case UP:
         printf("Rotated to UP\n");
         break;
      case LEFT:
         printf("Rotated to LEFT\n");
         break;
      case RIGHT:
         printf("Rotated to RIGHT\n");
         break;
      case DOWN:
         printf("Rotated to DOWN\n");
         break;
      default:
         break;
   }
#endif
}

void wvMainWindow::rotate_right()
{
#ifdef WM_DEBUG
   printf("Rotating right!\n");
#endif
   display_options.orientation = (display_options.orientation + 1) % 4;

#ifdef WM_DEBUG
   switch (display_options.orientation)
   {
      case UP:
         printf("Rotated to UP\n");
         break;
      case LEFT:
         printf("Rotated to LEFT\n");
         break;
      case RIGHT:
         printf("Rotated to RIGHT\n");
         break;
      case DOWN:
         printf("Rotated to DOWN\n");
         break;
      default:
         break;
   }
#endif
}

std::map<int, RobotOption> wvMainWindow::get_robot_options()
{
   return robot_options;
}

DisplayOption wvMainWindow::get_display_options()
{
   return display_options;
}

void wvMainWindow::before_quit()
{
   save_settings();
}

// *******************************************************************************
wvTable::wvTable(int rows, int cols, QWidget *parent) :
   QTable(rows, cols, parent)
{
}

void wvTable::resizeEvent(QResizeEvent *e)
{
   adjustColumns();
}

void wvTable::adjustColumns()
{
#ifdef WM_DEBUG
   printf("Adjusting columns...\n");
#endif
   adjustColumn(1);
   adjustColumn(2);
   adjustColumn(3);
   adjustColumn(7);
}

// *******************************************************************************

wvTableDockWindow::wvTableDockWindow( QWidget *parent, const char *name, WFlags f) :
   QDockWindow(parent, name, f)
{
   table = new wvTable(0,11,this);
   setWidget(table);
   setResizeEnabled(true);
   setHorizontallyStretchable(true);
   setVerticallyStretchable(true);

   QHeader *header = table->horizontalHeader();
   header->setLabel(0, "ID");
   header->setLabel(1, "IP");
   header->setLabel(2, "T");
   header->setLabel(3, "P");
   header->setLabel(4, "Show");
   header->setLabel(5, "Show var");
   header->setLabel(6, "Show ball");
   header->setLabel(7, "Show ball var");
   header->setLabel(8, "Show obstacle");
   header->setLabel(9, "Show info");
   header->setLabel(10, "Since(ms)");
   header->setLabel(11, "Port");

   table->hideColumn(0);

   resize(QSize(size().width(), 10));

   table->verticalHeader()->hide();
   table->setSelectionMode(QTable::NoSelection);

   table->setColumnStretchable(4, true);
   table->setColumnStretchable(5, true);
   table->setColumnStretchable(6, true);
   table->setColumnStretchable(7, true);
   table->setColumnStretchable(8, true);
   table->setColumnStretchable(9, true);

   connect(table, SIGNAL(valueChanged(int, int)),this,SLOT(value_changed(int, int)));
   
   info_sigmap = new QSignalMapper(this);
   connect(info_sigmap,SIGNAL(mapped(int)),wvMainWindow::instance,SLOT(show_info(int)));
}

wvTableDockWindow::~wvTableDockWindow()
{
   if (table != NULL)
      delete table;
}

QTable *wvTableDockWindow::get_table()
{
   return table;
}

void wvTableDockWindow::value_changed(int row, int col)
{
   QCheckTableItem *item;
   switch (col)
   {
      case 4:
         item = (QCheckTableItem *)table->item(row, col);
         emit show_robot(table->item(row,0)->text().toInt(), item->isChecked());
         break;
      case 5:
         item = (QCheckTableItem *)table->item(row, col);
         emit show_robot_var(table->item(row,0)->text().toInt(), item->isChecked());
         break;
      case 6:
         item = (QCheckTableItem *)table->item(row, col);
         emit show_ball(table->item(row,0)->text().toInt(), item->isChecked());
         break;
      case 7:
         item = (QCheckTableItem *)table->item(row, col);
         emit show_ball_var(table->item(row,0)->text().toInt(), item->isChecked());
         break;
      case 8: 
         item = (QCheckTableItem *)table->item(row, col);
         emit show_obstacle(table->item(row,0)->text().toInt(), item->isChecked());
         break; 
   }
}

void wvTableDockWindow::robot_added(int id)
{
   std::map<int, Robot *> robots = wvMainWindow::instance->robotlist->get_robot_list();

   int row_num = table->numRows();
   table->insertRows(row_num);
   table->setText(row_num, 0, QString::number(id));

#ifdef WM_DEBUG
   printf("Adding row %d\n", row_num);
#endif

   table->setText(row_num,1,robots[id]->ip.c_str());
   table->setText(row_num,2,QString::number(robots[id]->team));
   table->setText(row_num,3,QString::number(robots[id]->playernum));
   table->setItem(row_num,4,new QCheckTableItem(table,""));
   table->setItem(row_num,5,new QCheckTableItem(table,""));
   table->setItem(row_num,6,new QCheckTableItem(table,""));
   table->setItem(row_num,7,new QCheckTableItem(table,""));
   table->setItem(row_num,8,new QCheckTableItem(table,""));

   ((QCheckTableItem *)table->item(row_num, 4))->setChecked(true);
   ((QCheckTableItem *)table->item(row_num, 5))->setChecked(true);
   ((QCheckTableItem *)table->item(row_num, 6))->setChecked(true);
   ((QCheckTableItem *)table->item(row_num, 7))->setChecked(true);
   ((QCheckTableItem *)table->item(row_num, 8))->setChecked(false);

   QPushButton *button = new QPushButton("Info",table);
   table->setCellWidget(row_num, 9, button);
   connect(table->cellWidget(row_num, 9), SIGNAL(clicked()), info_sigmap, SLOT(map()));
   info_sigmap->setMapping(button, id);
   
   table->setText(row_num,11,QString::number(robots[id]->port));
}

void wvTableDockWindow::robot_removed(int id)
{
   std::map<int, Robot *> robots = wvMainWindow::instance->robotlist->get_robot_list();
   int i;
   for (i = 0; i < table->numRows(); i++)
   {
      if (table->text(i,0).toInt() == id)
      {
         info_sigmap->removeMappings(table->cellWidget(i,6));
         //delete info_sigmap;
         //info_sigmap = new QSignalMapper(this);
         table->clearCellWidget(i, 6);
         table->removeRow(i);

#ifdef WM_DEBUG
         printf("Removing row %d\n", i);
#endif
      }
   }
}

// *******************************************************************************

wvPortWindow::wvPortWindow(QWidget *parent, QString name) :
   QDialog(parent, name, TRUE)
{
   QSizePolicy button_policy(QSizePolicy::Fixed, QSizePolicy::Fixed, false);
   
   QBoxLayout *vbox1 = new QVBoxLayout(this, 5);

   QLabel *label = new QLabel("This is the list of UDP ports you are listening on.\n",this);
   vbox1->addWidget(label);
   
   QBoxLayout *hbox1 = new QHBoxLayout(vbox1);
   listbox = new QListBox(this);
   hbox1->addWidget(listbox);

   QBoxLayout *vbox2 = new QVBoxLayout(hbox1);
   add = new QPushButton("Add", this);
   remove = new QPushButton("Remove", this);
   add->setSizePolicy(button_policy);
   remove->setSizePolicy(button_policy);
   vbox2->addWidget(add,0,Qt::AlignTop);
   vbox2->addWidget(remove,0,Qt::AlignTop);
   vbox2->addStretch();

   vbox1->addSpacing(5);
   
   ok = new QPushButton("Ok", this);
   ok->setSizePolicy(button_policy);
   vbox2->addWidget(ok,0,Qt::AlignBottom);
   
   this->setSizeGripEnabled(true);

   // Fill window
   std::set<int> port_list = wvMainWindow::instance->robotlist->get_ports();
   std::set<int>::iterator it;

   for (it = port_list.begin(); it != port_list.end(); it++)
   {
      listbox->insertItem(QString::number(*it));
   }

   remove->setEnabled(false);
   //remove->setEnabled(true);

   connect(add, SIGNAL(clicked()),this,SLOT(add_port()));
   connect(remove, SIGNAL(clicked()),this,SLOT(remove_port()));
   connect(ok, SIGNAL(clicked()),this,SLOT(accept()));
   connect(listbox, SIGNAL(highlighted(int)),this,SLOT(highlight(int)));
}

void wvPortWindow::add_port()
{
#ifdef WM_DEBUG
   printf("Going to add port...\n");
#endif
   wvAddPortWindow *dlg = new wvAddPortWindow(this);
   if (dlg->exec())
   {
      QString port_string = dlg->port_edit->text();
      bool converted;

      int port_num = port_string.toInt(&converted);

      if (converted == TRUE)
      {
         if (wvMainWindow::instance->robotlist->add_socket(port_num))
         {
            listbox->insertItem(QString::number(port_num));
         }
      }
   }
}

void wvPortWindow::highlight(int x)
{
   remove->setEnabled(TRUE);
}

void wvPortWindow::remove_port()
{
#ifdef WM_DEBUG
   printf("Going to remove port...\n");
#endif
   if (listbox->currentItem() == -1)
      return;

   int port_num = listbox->item(listbox->currentItem())->text().toInt();
   wvMainWindow::instance->robotlist->remove_socket(port_num);
   listbox->removeItem(listbox->currentItem());
}
 
// *******************************************************************************

wvAddPortWindow::wvAddPortWindow( QWidget *parent, QString name) :
   QDialog(parent, name, TRUE)
{
   
   QBoxLayout *vbox1 = new QVBoxLayout(this, 5);

   
   QLabel *label1 = new QLabel("Enter the team number of the robots here.", this);
   ip_edit = new QLineEdit(this);
   QLabel *label2 = new QLabel("Alternatively, just enter the port used directly here.", this);
   port_edit = new QLineEdit(this);

   vbox1->addWidget(label1);
   vbox1->addWidget(ip_edit);
   vbox1->addWidget(label2);
   vbox1->addWidget(port_edit);

   QBoxLayout *hbox1 = new QHBoxLayout(vbox1);
   ok = new QPushButton("Ok", this);
   cancel = new QPushButton("Cancel", this);
   hbox1->addWidget(ok);
   hbox1->addWidget(cancel);

   connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
   connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
   connect(ip_edit, SIGNAL(textChanged(const QString&)), this, SLOT(edit_ip(const QString&)));
   connect(port_edit, SIGNAL(textChanged(const QString&)), this, SLOT(edit_port(const QString&)));

   QValidator* validator1 = new QIntValidator( 1, 255, this );
   QValidator* validator2 = new QIntValidator( 0, 65535, this );

   ip_edit->setValidator(validator1);
   port_edit->setValidator(validator2);
}

void wvAddPortWindow::edit_ip(const QString &qs)
{
   if (!qs.isEmpty())
   {
#ifdef OSAKA
      port_edit->setText( QString::number(ip_edit->text().toInt() * 10 + 10200) );
#else
      port_edit->setText( QString::number(ip_edit->text().toInt() * 10 + 10000) );
#endif   
   }
}
 
void wvAddPortWindow::edit_port(const QString &qs)
{
}


// *******************************************************************************

wvInfoDockWindow::wvInfoDockWindow( QWidget *parent, QString name, WFlags f) :
   QDockWindow(parent, name, f)
{
   QBoxLayout *vbox = new QVBoxLayout(boxLayout());
   QLabel *label = new QLabel("Info window:", this);
   combobox = new QComboBox(this);
   textedit = new QTextEdit(this);
   vbox->addWidget(label);
   vbox->addWidget(combobox);
   vbox->addWidget(textedit);

   textedit->setWordWrap(QTextEdit::NoWrap);
   
   label->show();
   combobox->show();
   textedit->show();
   textedit->setReadOnly(TRUE);
   textedit->setTextFormat(RichText);

   setCloseMode(Always);
   setResizeEnabled(true);
   setHorizontallyStretchable(true);
   setVerticallyStretchable(true);

   startTimer( 200 );
}

void wvInfoDockWindow::add_robot(int id)
{
   std::map<int, Robot *> robot_map = wvMainWindow::instance->robotlist->get_robot_list();

   QString itemtext = QString("%1-%2-%3")
      .arg(robot_map[id]->team)
      .arg(robot_map[id]->playernum)
      .arg(robot_map[id]->ip.c_str());
   
   combobox->insertItem( itemtext );
   id_map[id] = itemtext;
   str_map[itemtext] = id;
}

void wvInfoDockWindow::remove_robot(int id)
{
   std::map<int, Robot *> robot_map = wvMainWindow::instance->robotlist->get_robot_list();
   QString itemtext = id_map[id];

   if (combobox->currentText() == itemtext)
   {
      combobox->removeItem(combobox->currentItem());
      textedit->clear();
   }
   
   int i;
   for (i = 0; i < combobox->count();)
   {
      if (combobox->text(i) == itemtext)
      {
         combobox->removeItem(i);
      }
      else
         i++;
   }

   id_map.erase(id);
   str_map.erase(itemtext);
}

void wvInfoDockWindow::select_robot(int id)
{
   std::map<int, Robot *> robot_map = wvMainWindow::instance->robotlist->get_robot_list();
   QString itemtext = id_map[id];

   int i;
   for (i = 0; i < combobox->count();i++)
   {
      if (combobox->text(i) == itemtext)
      {
         combobox->setCurrentItem(i);
      }
   }
}

void wvInfoDockWindow::robot_removed(int id)
{
   remove_robot(id);
}

void wvInfoDockWindow::robot_added(int id)
{
   add_robot(id);
}

int wvInfoDockWindow::get_robot()
{
   QString current_text = combobox->currentText();

   return str_map[current_text];
   
   /*std::map<int, QString>::iterator it;

   for (it = id_map.begin(); it != id_map.end(); it++)
   {
      if (it->second == current_text)
         return it->first;
   }

   return -1;*/
}

void wvInfoDockWindow::hideEvent(QHideEvent *h)
{
   close();
}

void wvInfoDockWindow::closeEvent(QCloseEvent *c)
{
#ifdef WM_DEBUG
   printf("We're being closed! Going to check list of size %d\n",wvMainWindow::instance->info_list.size());
#endif
   std::vector<wvInfoDockWindow *> *info_list = &(wvMainWindow::instance->info_list);

   std::vector<wvInfoDockWindow *>::iterator it;
   for (it = info_list->begin(); it != info_list->end();)
   {
#ifdef WM_DEBUG
      printf("Checking list to remove info window...\n");
#endif

      if (*it == this)
      {
         info_list->erase(it);
#ifdef WM_DEBUG
         printf("Removing info window from list\n");
#endif
      }
      else
         it++;
   }
}

void wvInfoDockWindow::timerEvent( QTimerEvent *t)
{
   //printf("Current id: %d\n", combobox->currentItem());
   if (!combobox->currentText().isEmpty())
   {
      int id = str_map[combobox->currentText()];
      std::map<int, Robot *> robot_map = wvMainWindow::instance->robotlist->get_robot_list();

      QString info;

      info.sprintf(\
"\
<b>IP:</b> %s<BR>\
<b>Team:</b> %d<BR>\
<b>Player:</b> %d<BR>\
<b>Pos:</b> (%.1f, %.1f)<BR>\
<b>PosH:</b> %.1f<BR>\
<b>PosVar:</b> %.1f<BR>\
<b>Ball:</b> (%.1f, %.1f)<BR>\
<b>BallVar:</b> %.1f<BR>\
<b>Time to reach ball</b>: %d<BR>\
<b>Has grabbed ball</b>: %d<BR>\
<b>Has seen ball</b>: %d<BR>\
<b>Has lost ball</b>: %d<BR>\
<b>Role</b>: %s (%d)<BR>\
<b>Role counter</b>: %u\
",
            robot_map[id]->ip.c_str(), robot_map[id]->team,robot_map[id]->playernum,
            robot_map[id]->posx, robot_map[id]->posy,robot_map[id]->posh,robot_map[id]->posvar,
            robot_map[id]->ballx,robot_map[id]->bally,robot_map[id]->ballvar,robot_map[id]->ballTime,
            robot_map[id]->hasGrabbedBall, robot_map[id]->hasSeenBall, robot_map[id]->hasLostBall,
            roleToString(robot_map[id]->role), robot_map[id]->role, robot_map[id]->roleCounter);

      textedit->setText(info);
   }
}

// *******************************************************************************

wvGLWidget::wvGLWidget( QWidget *parent, const char *name )
     : QGLWidget( parent, name )
{
   time_interval = 1000/30;
   startTimer( time_interval );
}

void wvGLWidget::initializeGL()
{
   GLText::start("profont.raw");
}

void wvGLWidget::resizeGL(int width, int height)
{
    glViewport( 0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, (float)width, 0, (float)height, -1.0f, 1.0f);

    glMatrixMode(GL_MODELVIEW);
}

void wvGLWidget::paintGL()
{
   draw();
}

void wvGLWidget::timerEvent( QTimerEvent *t)
{
   updateGL();
}

void wvGLWidget::add_robot(int id)
{
   std::map<int, RobotOption> r_o = wvMainWindow::instance->get_robot_options();
   std::map<int, Robot *> robot_list = wvMainWindow::instance->robotlist->get_robot_list();
   GLRobot *r = new GLRobot();
   gl_robots[id] = r;

   GLBall *b = new GLBall();
   gl_balls[id] = b;

   r->set_text("%d", robot_list[id]->playernum);
   b->set_text("%d", robot_list[id]->playernum);

   /*QString s;
   s.sprintf("#%d", r_o[id].playernum);
   QPixmap pix();
   QPainter painter(pix);
   QRect bbox = painter.boundingRect(0,0,500,500,);*/
}

void wvGLWidget::remove_robot(int id)
{
   delete gl_robots[id];
   gl_robots.erase(id);

   delete gl_balls[id];
   gl_balls.erase(id);
}

GLColour wvGLWidget::get_role_colour(Robot *r)
{
   if (r->role == SUPPORTER)
      return GLColour::PURPLE;

   if (r->role == ATTACKER)
      return GLColour::YELLOW;
   
   if (r->role == WINGER)
      return GLColour::SKYBLUE;

   if (r->role == DEFENDER)
      return GLColour::ORANGE;
   
   return GLColour::WHITE;
}

static const int OBSTACLE_THRESHOLD = 20;

void wvGLWidget::draw()
{
   DisplayOption d_o = wvMainWindow::instance->get_display_options();
   std::map<int, RobotOption> r_o = wvMainWindow::instance->get_robot_options();

   Rectangle r(Position(0,20),Position(20,0));
   r.set_colour(GLColour::WHITE);
   
   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();

   float scale, t_x, t_y,ro;
   float height_diff, width_diff;
   float field_width, field_height;

   // Orientation transform
   switch(d_o.orientation)
   {
      case UP:
         field_height = GLField::FULL_LENGTH;
         field_width  = GLField::FULL_WIDTH;
         ro = 0;
         break;
      case RIGHT:
         field_height = GLField::FULL_WIDTH;
         field_width  = GLField::FULL_LENGTH;
         //scale = ((float)height()) / GLField::FULL_WIDTH;
         ro = -90;
         break;
      case LEFT:
         field_height = GLField::FULL_WIDTH;
         field_width  = GLField::FULL_LENGTH;
         ro = 90;
         break;
      case DOWN:
         field_height = GLField::FULL_LENGTH;
         field_width  = GLField::FULL_WIDTH;
         ro = 180;
         break;
      default:
         printf("What the hell? Orientation is buggered! %d\n", d_o.orientation);
   }
   

   height_diff = height() - field_height;
   width_diff = width() - field_width;

   if (height_diff < width_diff)
      scale = ((float)height()) / field_height;
   else
      scale = ((float)width()) / field_width;
      
         
   // Translate field into the center?
   glTranslatef(width() / 2.0, height() / 2.0, 0.0f);
   glRotatef(ro, 0.0,0.0,1.0);
   glScalef(scale, scale, 0);

   field.set_beacon_angle(-ro);
   field.draw();

   std::map<int, Robot *> robot_map = wvMainWindow::instance->robotlist->get_robot_list();

   std::map<int, GLRobot *>::iterator it;

   GLRobot::set_show_text(d_o.show_text);
   GLBall::set_show_text(d_o.show_text);

   for (it = gl_robots.begin(); it != gl_robots.end(); it++)
   {
      int id = it->first;
      Robot *r = robot_map[id];

      bool we_are_blue  = ((r->team % 2 && (!d_o.oddIsRed)) || (!(r->team % 2) && d_o.oddIsRed));
      bool wirelessball = false;
      
      //printf("Roboto options %d...show is %d!\n", id, r_o[id].show);
      if (r_o[id].show_obstacle)
      {
         for (int i = 0; i < NUM_OBSTACLE_SHARE; i++) {
 
            unsigned int obstacleVal = r->obstacleVals[i];
            int x     = (obstacleVal & 0x000003FF) >> 0;
            int y     = (obstacleVal & 0x000FFC00) >> 10;
            int count = (obstacleVal & 0xFFF00000) >> 20;
            
            if (count > OBSTACLE_THRESHOLD) {
                GLObstacle obs;
                
                obs.set_position(Position(x,y)); 
                if (!we_are_blue)
                {
                    obs.flip();
                }
                
                if (count > 100) { 
                    count = 100;
                }
                
                obs.set_colour(1.0*count/100,0,0);
                obs.draw();
            }     
         }      
      }
      
      
      if (r_o[id].show == true)
      {
         GLRobot *glr = it->second;
         
         glr->set_show_variance(r_o[id].show_var);
               
         glr->set_position(Position(r->posx, r->posy));
         glr->set_heading(r->posh);
         glr->set_variance(r->posvar);

         if (we_are_blue)
         {
            glr->set_colour(GLColour::BLUE);
         }
         else
         {
            glr->flip();
            glr->set_colour(GLColour::RED);
         }

         glr->set_arrow_colour(get_role_colour(r));
         
         glr->draw();       

      }     
      

      
      if ((r_o[id].show_ball) && (r->ballvar < 2000000))
      {
         GLBall *glball = gl_balls[id];
         
         if (r->team < (MAX_SHARE_BALL_TEAMS - 1))
         {
            if ((r->playernum == wvMainWindow::instance->shareBallSourceBot[r->team]) || 
                (r->ballvar < wvMainWindow::instance->shareBallVar[r->team]))
            {
                wvMainWindow::instance->shareBallSourceBot[r->team] = r->playernum;
                wvMainWindow::instance->shareBallVar[r->team] = r->ballvar;
                wirelessball = true;
            }
         }         

         glball->set_show_variance(r_o[id].show_ball_var);

         glball->set_position(Position(r->ballx, r->bally));
         glball->set_variance(r->ballvar);

         glball->set_ball_colour(GLColour::ORANGE);
         
         if (we_are_blue)
         {
            glball->set_var_colour(GLColour::BLUE);
            if (wirelessball)
            {
                glball->set_core_colour(GLColour::DARKBLUE);
            }             
         }
         else
         {
            glball->flip();
            glball->set_var_colour(GLColour::RED);
            if (wirelessball)
            {
                glball->set_core_colour(GLColour::DARKRED);
            }            
         }
         
         glball->draw();
      }
            
   }


}

// *******************************************************************************

int main(int argc, char *argv[])
{

   QApplication app( argc, argv );
   wvMainWindow mainwindow;

   // Setup
   app.setMainWidget( &mainwindow );
   mainwindow.show();
  
   return app.exec();
}
