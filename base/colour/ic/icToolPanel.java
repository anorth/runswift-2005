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

public class icToolPanel extends JPanel implements ActionListener, ChangeListener, PropertyChangeListener
{
    protected icBFLPanel   bflPane;

    protected final static Dimension buttonSize = new Dimension(80, 30);
    protected final static Dimension sliderSize = new Dimension(100, 40);

    protected JButton      fileTool[]  = new JButton[icConstant.FileToolName.length];
    protected JRadioButton drawTool[]  = new JRadioButton[5];
    protected JButton      otherTool[] = new JButton[icConstant.OtherToolName.length];

    protected ButtonGroup  group     = new ButtonGroup();

    protected JSlider brushSlider;
    protected JSlider wandSlider;

    public icToolPanel(icBFLPanel bflPane)
    {
	this.bflPane = bflPane;
	bflPane.addPropertyChangeListener(this);

	setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));
	setBorder(BorderFactory.createEtchedBorder());

	addComponents();
	setEnabled(false);
    }

    protected void addComponents()
    {
	add(new JToolBar.Separator());

	//Add file tool buttons
	for(int i=0; i < icConstant.FileToolName.length; i++) {
	    fileTool[i] = new JButton(icConstant.FileToolName[i]);
	    fileTool[i].setAlignmentX(CENTER_ALIGNMENT);
	    fileTool[i].setAlignmentY(CENTER_ALIGNMENT);
	    fileTool[i].setMaximumSize(buttonSize);
	    fileTool[i].setMinimumSize(buttonSize);
	    fileTool[i].setActionCommand(""+(icConstant.FILE+i));
	    fileTool[i].addActionListener(this);
	    add(fileTool[i]);
	}

	add(new JToolBar.Separator());

	//set sliders properties
	brushSlider = new JSlider(SwingConstants.HORIZONTAL, 0, 10, 0);
	brushSlider.setMajorTickSpacing(2);
	brushSlider.setMinorTickSpacing(1);
	brushSlider.setSnapToTicks(true);
	brushSlider.setPaintTicks(true);
	brushSlider.setPaintLabels(true);
	brushSlider.setPreferredSize(sliderSize);
	brushSlider.addChangeListener(this);

	wandSlider = new JSlider(SwingConstants.HORIZONTAL, 0, 30, 25);
	wandSlider.setMajorTickSpacing(10);
	wandSlider.setMinorTickSpacing(5);
	wandSlider.setSnapToTicks(true);
	wandSlider.setPaintTicks(true);
	wandSlider.setPaintLabels(true);
	wandSlider.setPreferredSize(sliderSize);
	wandSlider.addChangeListener(this);

	//add radio buttons
	for(int i=0, j=0; i<icConstant.DrawToolName.length; i++) {
	    if(icConstant.DrawToolName[i].length()==0)
		add(new JToolBar.Separator());
	    else if(icConstant.DrawToolName[i].equals("bs"))
		add(brushSlider);
	    else if(icConstant.DrawToolName[i].equals("ws"))
		add(wandSlider);
	    else {
		drawTool[j] = new JRadioButton(icConstant.DrawToolName[i]);
		drawTool[j].setAlignmentX(CENTER_ALIGNMENT);
		drawTool[j].setAlignmentY(CENTER_ALIGNMENT);
		drawTool[j].setMaximumSize(buttonSize);
		drawTool[j].setMinimumSize(buttonSize);
		drawTool[j].setActionCommand(""+(icConstant.TOOL+j));
		drawTool[j].addActionListener(this);
		group.add(drawTool[j]);
		add(drawTool[j]);
		j++;
	    }
	}

	add(new JToolBar.Separator());

	//Add file tool buttons
	for(int i=0; i < icConstant.OtherToolName.length; i++) {
	    otherTool[i] = new JButton(icConstant.OtherToolName[i]);
	    otherTool[i].setAlignmentX(CENTER_ALIGNMENT);
	    otherTool[i].setAlignmentY(CENTER_ALIGNMENT);
	    otherTool[i].setMaximumSize(buttonSize);
	    otherTool[i].setMinimumSize(buttonSize);
	    otherTool[i].setActionCommand(""+(icConstant.TOOL_CLEAR+i));
	    otherTool[i].addActionListener(this);
	    add(otherTool[i]);
	}

	//select brush
	drawTool[0].setSelected(true);

	//enable brush slider
	brushSlider.setEnabled(drawTool[0].isSelected());

	//disable wand slider
	wandSlider.setEnabled(drawTool[3].isSelected());

	//disable undo button
	otherTool[1].setEnabled(false);
    }

    public void setEnabled(boolean b)
    {
	for(int i=1; i<fileTool.length; i++)
	    fileTool[i].setEnabled(b);

	for(int i=0; i<drawTool.length; i++)
	    drawTool[i].setEnabled(b);

	for(int i=0; i<otherTool.length; i++)
	    otherTool[i].setEnabled(b);

	brushSlider.setEnabled(drawTool[0].isSelected() && 
			       drawTool[0].isEnabled());
	wandSlider.setEnabled(drawTool[3].isSelected() && 
			      drawTool[3].isEnabled());
    }

    public void propertyChange(PropertyChangeEvent e)
    {
	if(e.getPropertyName().equals(""+icConstant.FILE_OPEN)) {

	   if(e.getOldValue()==null)
	    setEnabled(true);
	   else {
	    bflPane.canUndo = false;
	    otherTool[1].setEnabled(bflPane.canUndo);

	    bflPane.canClear = true;
	    otherTool[0].setEnabled(bflPane.canClear);
	   }
	}
	else if(e.getPropertyName().equals(""+icConstant.TOOL_UNDO))
	    otherTool[1].setEnabled(bflPane.canUndo);
	else if(e.getPropertyName().equals(""+icConstant.TOOL_CLEAR))
	    otherTool[0].setEnabled(bflPane.canClear);
    }

    public void stateChanged(ChangeEvent e)
    {
	if(e.getSource() == brushSlider)
	    bflPane.setBrushSize(brushSlider.getValue());
	else if(e.getSource() == wandSlider)
	    bflPane.setWandThreshold(wandSlider.getValue());
    }

    /**
     * The action event handler of this object.
     */
    public void actionPerformed(ActionEvent e)
    {
	int i = Integer.parseInt(e.getActionCommand());
	//System.out.println(action);

	if(icConstant.isTool(i)) {
	    bflPane.currentTool = i;
	}
	else {
	    switch(i) {
	    case icConstant.FILE_OPEN:
		open();
		break;
	    case icConstant.FILE_SAVE:
		save();
		break;
	    case icConstant.FILE_SAVEAS:
		saveAs();
		break;
	    case icConstant.TOOL_CLEAR:
		if(bflPane.canClear)
		    bflPane.clear();
		break;
	    case icConstant.TOOL_UNDO:
		if(bflPane.canUndo)
		    bflPane.undo();
		break;
	    }
	}

	//enable clear button
	otherTool[0].setEnabled(bflPane.canClear);

	//enable undo button
	otherTool[1].setEnabled(bflPane.canUndo);

	//enable slider(s)
	brushSlider.setEnabled(drawTool[0].isSelected());
	wandSlider.setEnabled(drawTool[3].isSelected());
    }

    public void open()
    {
	JFileChooser fc = new JFileChooser();
	File file;

	if(bflPane.getFileName()==null)
	    fc.setCurrentDirectory(new File("."));
	else {
	    file = new File(bflPane.getFileName());
	    fc.setSelectedFile(file);
	}

	int returnVal = fc.showOpenDialog(getTopLevelAncestor());
	if (returnVal == JFileChooser.CANCEL_OPTION)
	    return;

	file = fc.getSelectedFile();
	if(file.isFile())
	    open(file.getPath());
    }

    public void open(String filename)
    {
	//System.out.println("open: "+filename);
	bflPane.readFile(filename);
    }

    public void saveAs()
    {
	JFileChooser fc = new JFileChooser();
	File file;

	fc.setSelectedFile(new File(bflPane.getFileName()));

	int returnVal = fc.showSaveDialog(getTopLevelAncestor());
	if (returnVal == JFileChooser.CANCEL_OPTION)
	    return;

	file = fc.getSelectedFile();

	if(file.getPath().toLowerCase().endsWith(".bfl"))
	    bflPane.writeFile(file.getPath());
	else if(file.getPath().toLowerCase().endsWith(".jpg"))
	    bflPane.writeJpegFile(file.getPath());
	else if(file.getPath().toLowerCase().endsWith(".ftr"))
	    bflPane.writeFilterFile(file.getPath());
	else if(file.getPath().toLowerCase().endsWith(".cpl"))
	    bflPane.writeCPlaneLogFile(file.getPath());
	else
	    System.err.println("Unknown file type");
    }

    public void save()
    {
	bflPane.writeFile();
    }

    public static void main(String[] args)
    {
	if(args.length > 1) {
	    System.err.println("java icToolPanel [<file>]");
	    System.exit(1);
	}

	try {
	    JFrame frame = new JFrame("icToolPanel");
	    frame.setSize(650, 550);
	    frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

	    icBFLPanel bflPane     = new icBFLPanel();
	    icColorPanel colorPane = new icColorPanel(bflPane);
	    icToolPanel toolPane   = new icToolPanel(bflPane);

	    frame.getContentPane().setLayout(new BorderLayout());
	    frame.getContentPane().add(bflPane, BorderLayout.CENTER);
	    frame.getContentPane().add(colorPane, BorderLayout.SOUTH);
	    frame.getContentPane().add(toolPane, BorderLayout.WEST);

	    frame.setVisible(true);

	    if(args.length == 1) {
		toolPane.open(args[0]);
	    }

	} catch (Exception e) {
	    System.err.println(e);
	}
    }
}
