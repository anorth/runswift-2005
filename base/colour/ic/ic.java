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

public class ic extends JFrame implements ItemListener
{
    protected icMenuBar      menuBar;
    protected icBFLPanel     bflPane;
    protected icColorPanel   colorPane;
    protected icToolPanel    toolPane;
    protected icFilterPanel  filterPane;
    protected icEdgePanel    edgePane;
    protected JPanel         eastPane;
    protected JPanel         southPane;

    protected ButtonGroup    planeButtonGroup = new ButtonGroup();

    public ic()
    {
	setTitle("ic");
	setSize(Toolkit.getDefaultToolkit().getScreenSize());
	setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

	addComponents();
	setVisible(true);
    }

    public ic(String file)
    {
	this();
	bflPane.readFile(file);
    }

    public void open(String file)
    {
	bflPane.readFile(file);
    }

    protected void addComponents()
    {
	bflPane      = new icBFLPanel();
	menuBar      = new icMenuBar();
	colorPane    = new icColorPanel(bflPane, planeButtonGroup);
	toolPane     = new icToolPanel(bflPane);
	filterPane   = new icFilterPanel(bflPane, planeButtonGroup);
	edgePane     = new icEdgePanel(bflPane);
	eastPane     = new JPanel();
	eastPane.setLayout(new BoxLayout(eastPane, BoxLayout.X_AXIS));
	southPane    = new JPanel();
	southPane.setLayout(new BoxLayout(southPane, BoxLayout.Y_AXIS));

	//Listen for new file opened
	bflPane.addPropertyChangeListener(menuBar);

	//Listen for plane changes
	colorPane.addPropertyChangeListener(filterPane);
	filterPane.addPropertyChangeListener(colorPane);
	toolPane.addPropertyChangeListener(filterPane);
	filterPane.addPropertyChangeListener(toolPane);

	//Listen for menu changes
	menuBar.addFileListener(toolPane);
	menuBar.addViewListener(this);

	getContentPane().setLayout(new    BorderLayout());
	getContentPane().add(menuBar,     BorderLayout.NORTH);
	getContentPane().add(bflPane,     BorderLayout.CENTER);
	getContentPane().add(toolPane,    BorderLayout.WEST);
	getContentPane().add(eastPane,    BorderLayout.EAST);
	getContentPane().add(southPane,   BorderLayout.SOUTH);

	eastPane.add(filterPane);
	eastPane.add(edgePane);
	southPane.add(colorPane);

	//Deselect colorPane - takes up too much space
	//menuBar.viewMenu[0].setSelected(false);
	//menuBar.viewMenu[1].setSelected(false);
	//Deselect filterPane
	//menuBar.viewMenu[3].setSelected(false);
	//Deselect edgePane
	menuBar.viewMenu[4].setSelected(false);
    }

    public void itemStateChanged(ItemEvent e)
    {
	int i = Integer.parseInt(((JCheckBoxMenuItem) e.getItemSelectable()).getName());
	switch(i)
	    {
	    case icConstant.VIEW_COLOR:
		if(e.getStateChange()==ItemEvent.SELECTED)
		    colorPane.addColorPane();
		else
		    colorPane.removeColorPane();
		break;
	    case icConstant.VIEW_PLANE:
		if(e.getStateChange()==ItemEvent.SELECTED)
		    colorPane.addPlanePane();
		else
		    colorPane.removePlanePane();
		break;
	    case icConstant.VIEW_TOOL:
		if(e.getStateChange()==ItemEvent.SELECTED)
		    getContentPane().add(toolPane, BorderLayout.WEST);
		else
		    getContentPane().remove(toolPane);
		break;
	    case icConstant.VIEW_FILTER:
		if(e.getStateChange()==ItemEvent.SELECTED)
		    eastPane.add(filterPane, 0);
		else
		    eastPane.remove(filterPane);
		break;
	    case icConstant.VIEW_EDGE:
		if(e.getStateChange()==ItemEvent.SELECTED)
		    eastPane.add(edgePane);
		else
		    eastPane.remove(edgePane);
		break;
	    }
	validate();
	getContentPane().repaint();
    }

    public static void main(String[] args)
    {
	if(args.length > 1) {
	    System.err.println("java ic [<file>]");
	    System.exit(1);
	}

	try {
	    ic frame = new ic();

	    if(args.length == 1)
		frame.open(args[0]);

	} catch (Exception e) {
	    System.err.println(e);
	}
    }
}
