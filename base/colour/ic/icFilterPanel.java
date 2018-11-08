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

/**
 * filter constants and static methods
 */
public class icFilterPanel extends JPanel implements ActionListener, PropertyChangeListener
{
    protected icBasicBFLPanel bflPane;

    protected static final int PIC_WIDTH        = icConstant.PIC_WIDTH;
    protected static final int PIC_HEIGHT       = icConstant.PIC_HEIGHT;

    protected static boolean selected           = false;

    protected final static Dimension buttonSize = new Dimension(100, 15);

    protected JButton        filter[]           = new JButton[14];

    protected final static String labelName[]   = 
    { "Plane to filter: ", "Filters: ", "Results" };
    JLabel label[] = new JLabel[labelName.length];

    JPanel    planePane = new JPanel();
    protected JRadioButton plane[] = new JRadioButton[icConstant.FilterPlaneName.length];

    JPanel    resultPane = new JPanel();
    protected JRadioButton result[] = new JRadioButton[icConstant.FilterResultName.length];
    
    protected ButtonGroup planeGroup            = new ButtonGroup();
    protected ButtonGroup resultGroup;

    public icFilterPanel(icBasicBFLPanel bflPane)
    {
	this(bflPane, new ButtonGroup());
    }

    public icFilterPanel(icBasicBFLPanel bflPane, ButtonGroup group)
    {
	this.bflPane = bflPane;
	bflPane.addPropertyChangeListener(this);
	resultGroup = group;
	addPropertyChangeListener(this);

	setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));
	setBorder(BorderFactory.createEtchedBorder());

	planePane.setLayout(new FlowLayout());
	planePane.setAlignmentX(CENTER_ALIGNMENT);
	planePane.setAlignmentY(CENTER_ALIGNMENT);

	resultPane.setLayout(new GridLayout(2,3));
	resultPane.setAlignmentX(CENTER_ALIGNMENT);
	resultPane.setAlignmentY(CENTER_ALIGNMENT);

	addComponents();
	setEnabled(false);
    }

    protected void addComponents()
    {
	for(int i=0; i<labelName.length; i++) {
	    label[i] = new JLabel(labelName[i]);
	    label[i].setAlignmentX(CENTER_ALIGNMENT);
	    label[i].setAlignmentY(CENTER_ALIGNMENT);
	}

	add(label[0]);
	for(int i=0; i<icConstant.FilterPlaneName.length; i++) {
	    plane[i] = new JRadioButton(icConstant.FilterPlaneName[i]);
	    plane[i].setAlignmentX(CENTER_ALIGNMENT);
	    plane[i].setAlignmentY(CENTER_ALIGNMENT);
	    //plane[i].setMaximumSize(buttonSize);
	    //plane[i].setMinimumSize(buttonSize);
	    plane[i].setActionCommand(""+(icConstant.FILTER_PLANE+i));
	    plane[i].addActionListener(this);
	    planeGroup.add(plane[i]);
	    planePane.add(plane[i]);
	}
	add(planePane);
	add(new JToolBar.Separator());

	add(label[1]);

	for(int i=0, j=0; i<icConstant.FilterName.length; i++) {
	    if(icConstant.FilterName[i].length()==0)
		add(new JToolBar.Separator());
	    else {
		filter[j] = new JButton(icConstant.FilterName[i]);
		filter[j].setAlignmentX(CENTER_ALIGNMENT);
		filter[j].setAlignmentY(CENTER_ALIGNMENT);
		filter[j].setMaximumSize(buttonSize);
		filter[j].setMinimumSize(buttonSize);
		filter[j].setActionCommand(""+(icConstant.FILTER+j));
		filter[j].addActionListener(this);
		add(filter[j]);
		j++;
	    }
	}
	add(new JToolBar.Separator());

	add(label[2]);
	for(int i=0; i<icConstant.FilterResultName.length; i++) {
	    result[i] = new JRadioButton(icConstant.FilterResultName[i]);
	    result[i].setAlignmentX(CENTER_ALIGNMENT);
	    result[i].setAlignmentY(CENTER_ALIGNMENT);
	    //result[i].setMaximumSize(buttonSize);
	    //result[i].setMinimumSize(buttonSize);
	    result[i].setActionCommand(""+(icConstant.FILTER_RESULT+i));
	    result[i].addActionListener(this);
	    resultGroup.add(result[i]);
	    resultPane.add(result[i]);
	}
	add(resultPane);

	//Select YPlane
	plane[0].setSelected(true);
    }

    public void setEnabled(boolean b)
    {
	for(int i=0; i<label.length; i++)
	    label[i].setEnabled(b);

	for(int i=0; i<plane.length; i++)
	    plane[i].setEnabled(selected);

	for(int i=0; i<filter.length; i++)
	    filter[i].setEnabled(selected);

	for(int i=0; i<result.length; i++)
	    result[i].setEnabled(b);
    }

    public void setSelected(boolean b)
    {
	if(selected == b)
	    return;

	selected = b;

	for(int i=0; i<plane.length; i++)
	    plane[i].setEnabled(b);

	for(int i=0; i<filter.length; i++)
	    filter[i].setEnabled(b);
    }

    public void propertyChange(PropertyChangeEvent e)
    {
	if(e.getPropertyName().equals(""+icConstant.FILE_OPEN) && 
	   e.getOldValue()==null)
	    setEnabled(true);
	else if(e.getPropertyName().equals(""+icConstant.CHANGE_PLANE))
	    setSelected(e.getSource() == this);
    }

    /**
     * The action event handler of this object.
     */
    public void actionPerformed(ActionEvent e)
    {
	int i = Integer.parseInt(e.getActionCommand());

	if(icConstant.isFilterPlane(i)) {
	    if(i==icConstant.FILTER_CPLANE) {
		filter[9].setEnabled(true);
		filter[10].setEnabled(true);
		result[0].setEnabled(false);
		result[1].setEnabled(false);
		result[2].setEnabled(false);
		result[3].setEnabled(false);
	    }
	    else {
		filter[9].setEnabled(false);
		filter[10].setEnabled(false);
		result[0].setEnabled(true);
		result[1].setEnabled(true);
		result[2].setEnabled(true);
		result[3].setEnabled(true);
	    }
	    bflPane.filterPlane = i;
	}
	else if(icConstant.isFilterResult(i)) {
	    if(!selected) {
		firePropertyChange(""+icConstant.CHANGE_PLANE, null, null);
		bflPane.isFilter = true;
	    }

	    bflPane.isFilter = true;
	    bflPane.filterResult = i;
	    bflPane.display();
	}
	else if(icConstant.isFilter(i))
	    bflPane.applyFilter(i);
    }

    public static void main(String[] args)
    {
	if(args.length > 1) {
	    System.err.println("java icFilterPanel [<file>]");
	    System.exit(1);
	}

	try {
	    JFrame frame = new JFrame("FilterPanel");
	    frame.setSize(Toolkit.getDefaultToolkit().getScreenSize());
	    frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

	    ButtonGroup group = new ButtonGroup();

	    icBasicBFLPanel bflPane  = new icBasicBFLPanel();
	    icColorPanel colorPane = new icColorPanel(bflPane, group);
	    icFilterPanel filterPane = new icFilterPanel(bflPane, group);

	    colorPane.addPropertyChangeListener(filterPane);
	    filterPane.addPropertyChangeListener(colorPane);

	    frame.getContentPane().setLayout(new   BorderLayout());
	    frame.getContentPane().add(bflPane,    BorderLayout.CENTER);
	    frame.getContentPane().add(colorPane,  BorderLayout.SOUTH);
	    frame.getContentPane().add(filterPane, BorderLayout.EAST);
	    frame.setVisible(true);

	    if(args.length == 1)
		bflPane.readFile(args[0]);

	} catch (Exception e) {
	    System.err.println(e);
	}
    }
}
