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

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.event.*;
import java.io.*;
import java.util.*;
import java.beans.*;

public class icMenuBar extends JMenuBar implements ActionListener, PropertyChangeListener
{
    public JMenu menu[] = new JMenu[icConstant.MenuName.length];

    public JMenuItem fileMenu[] = new JMenuItem[4];
    protected static String fileMenuName[][] = icConstant.FileMenuName;

    public JCheckBoxMenuItem viewMenu[] = new JCheckBoxMenuItem[5];
    protected static String viewMenuName[][] = icConstant.ViewMenuName;

    public icMenuBar()
    {
	addComponents();
	setEnabled(false);
    }

    protected void addComponents()
    {
	int i=0, j=0;

	//add File Menu
	menu[0] = new JMenu(icConstant.MenuName[0]);
	add(menu[0]);

	for(i=j=0; i<fileMenuName.length; i++) {
	    if(fileMenuName[i].length==0)
		menu[0].addSeparator();
	    else {
		fileMenu[j] = new JMenuItem(fileMenuName[i][0]);
		fileMenu[j].setMnemonic(fileMenuName[i][1].charAt(0));
		fileMenu[j].setActionCommand(""+(icConstant.FILE+j));
		menu[0].add(fileMenu[j]);
		j++;
	    }
	}

	fileMenu[j-1].addActionListener(this);

	//add View Menu
	menu[1] = new JMenu(icConstant.MenuName[1]);
	add(menu[1]);

	for(i=j=0; i<viewMenuName.length; i++) {
	    if(viewMenuName[i].length==0)
		menu[1].addSeparator();
	    else {
		viewMenu[j] = new JCheckBoxMenuItem(viewMenuName[i][0]);
		viewMenu[j].setMnemonic(viewMenuName[i][1].charAt(0));
		viewMenu[j].setName(""+(icConstant.VIEW+j));
		viewMenu[j].setState(true);
		menu[1].add(viewMenu[j]);
		j++;
	    }
	}
    }

    public void addFileListener(ActionListener l)
    {
	for(int i=0; i<fileMenu.length; i++)
	    fileMenu[i].addActionListener(l);
    }

    public void addViewListener(ItemListener l)
    {
	for(int i=0; i<viewMenu.length; i++)
	    viewMenu[i].addItemListener(l);
    }

    public void setEnabled(boolean b)
    {
	fileMenu[1].setEnabled(b); //Save
	fileMenu[2].setEnabled(b); //Save As
    }

    public void propertyChange(PropertyChangeEvent e)
    {
	if(e.getPropertyName().equals(""+icConstant.FILE_OPEN) && 
	   e.getOldValue()==null)
	    setEnabled(true);
    }

    public void actionPerformed(ActionEvent e)
    {
	//String action = e.getActionCommand();
	switch(Integer.parseInt(e.getActionCommand()))
	    {
	    case icConstant.FILE_EXIT:
		System.exit(0);
	    }
    }
}
