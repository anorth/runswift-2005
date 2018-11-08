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
import java.io.*;
import java.util.*;
import java.beans.*;

public class icColorPanel extends JPanel implements ActionListener, PropertyChangeListener
{
    protected icBasicBFLPanel bflPane;
    protected JPanel colorPane;
    protected JPanel planePane;

    protected final static Dimension buttonSize = new Dimension(100, 15);
    protected              boolean   selected   = true;

    protected ButtonGroup group;
    protected JRadioButton colorButton[] = new JRadioButton[icConstant.ColorName.length];
    protected JRadioButton planeButton[] = new JRadioButton[icConstant.PlaneName.length];

    public icColorPanel(icBasicBFLPanel bflPane)
    {
	this(bflPane, new ButtonGroup());
    }

    public icColorPanel(icBasicBFLPanel bflPane, ButtonGroup group)
    {
	this.bflPane = bflPane;
	bflPane.addPropertyChangeListener(this);
	this.group = group;
	addPropertyChangeListener(this);

	colorPane = new JPanel();
	colorPane.setLayout(new GridLayout(2,0));

	planePane = new JPanel();
	planePane.setLayout(new GridLayout(1,0));

	setBorder(BorderFactory.createEtchedBorder());
	setLayout(new BoxLayout(this,BoxLayout.Y_AXIS));
	setAlignmentX(LEFT_ALIGNMENT);

	addComponents();
	setEnabled(false);
    }

    protected void addComponents()
    {
	//addColorButtons
	int i;
	for(i=0; i<icConstant.ColorName.length; i++) {
	    colorButton[i] = new JRadioButton(icConstant.ColorName[i]);
	    //colorButton[i].setPreferredSize(buttonSize);
	    colorButton[i].setActionCommand(""+(icConstant.COLOR+i));
	    colorButton[i].addActionListener(this);
	    group.add(colorButton[i]);
	    colorPane.add(colorButton[i]);
	}
	colorButton[i-1].setActionCommand(""+icConstant.COLOR_NONE);

	//select None
	colorButton[i-1].setSelected(true);

	add(colorPane);

	//addPlaneButtons
	for(i=0; i<icConstant.PlaneName.length; i++) {
	    planeButton[i] = new JRadioButton(icConstant.PlaneName[i]);
	    //planeButton[i].setPreferredSize(buttonSize);
	    planeButton[i].setActionCommand(""+(icConstant.PLANE+i));
	    planeButton[i].addActionListener(this);
	    group.add(planeButton[i]);
	    planePane.add(planeButton[i]);
	}
	add(planePane);
    }

    public void addColorPane()
    {
	add(colorPane, 0);
    }

    public void removeColorPane()
    {
	remove(colorPane);
    }

    public void addPlanePane()
    {
	add(planePane);
    }

    public void removePlanePane()
    {
	remove(planePane);
    }

    public void setEnabled(boolean b)
    {
	for(int i=0; i<colorButton.length; i++)
	    colorButton[i].setEnabled(b);
	for(int i=0; i<planeButton.length; i++)
	    planeButton[i].setEnabled(b);
    }

    public void propertyChange(PropertyChangeEvent e)
    {
	if(e.getPropertyName().equals(""+icConstant.FILE_OPEN) && 
	   e.getOldValue()==null)
	    setEnabled(true);
	else if(e.getPropertyName().equals(""+icConstant.CHANGE_PLANE))
	    selected = (e.getSource() == this);
    }

    /**
     * The action event handler of this object.
     */
    public void actionPerformed(ActionEvent e)
    {
	if(!selected) {
	    selected = true;
	    firePropertyChange(""+icConstant.CHANGE_PLANE, null, null);
	    bflPane.isFilter = false;
	}

	String action = e.getActionCommand();
	bflPane.color = Byte.parseByte(action);
	bflPane.repaint();

	//System.out.println(action);
    }

    public static void main(String[] args)
    {
	if(args.length > 1) {
	    System.err.println("java icColorPanel [<file>]");
	    System.exit(1);
	}

	try {
	    JFrame frame = new JFrame("icColorPanel");
	    frame.setSize(550, 550);
	    frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

	    icBasicBFLPanel bflPane = new icBasicBFLPanel();
	    icColorPanel colorPane  = new icColorPanel(bflPane, new ButtonGroup());

	    frame.getContentPane().setLayout(new BorderLayout());
	    frame.getContentPane().add(bflPane, BorderLayout.CENTER);
	    frame.getContentPane().add(colorPane, BorderLayout.SOUTH);
	    frame.setVisible(true);

	    if(args.length == 1)
		bflPane.readFile(args[0]);
	    else
		colorPane.setEnabled(false);

	} catch (Exception e) {
	    System.err.println(e);
	}
    }
}
