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

public class icEdgePanel extends JPanel implements ItemListener, ChangeListener, PropertyChangeListener
{
    private static final int PIC_WIDTH    = icConstant.PIC_WIDTH;
    private static final int PIC_HEIGHT   = icConstant.PIC_HEIGHT;

    protected            int thresholdHi  = 175;
    protected            int thresholdLo  = 125;

    protected final static Dimension sliderSize = new Dimension(55, 200);

    protected icBasicBFLPanel bflPane;

    protected JLabel    label[]     = new JLabel[2];
    protected JSlider   slider[]    = new JSlider[2];
    protected String    planeName[] = icConstant.EdgePlaneName;
    protected JCheckBox plane[]     = new JCheckBox[icConstant.EdgePlaneName.length];

    protected int       CONSTANT    = icConstant.EDGE_PLANE;

    public icEdgePanel(icBasicBFLPanel bflPane)
    {
	this.bflPane = bflPane;
	bflPane.addPropertyChangeListener(this);

	setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));
	setBorder(BorderFactory.createEtchedBorder());

	addComponents();
	setEnabled(false);
	/*
	initEdgeOverlay(bflPane.rgbOverlay);
	findEdge(bflPane.YLL, threshold, 
		 bflPane.rgbOverlay, getMask(icConstant.EDGE_YPLANE));
	findEdge(bflPane.U, threshold, 
		 bflPane.rgbOverlay, getMask(icConstant.EDGE_UPLANE));
	findEdge(bflPane.V, threshold, 
		 bflPane.rgbOverlay, getMask(icConstant.EDGE_VPLANE));
	icFilter.equalise(bflPane.YLL, bflPane.filterY);
	icFilter.equalise(bflPane.U, bflPane.filterU);
	icFilter.equalise(bflPane.V, bflPane.filterV);
	findEdge(bflPane.filterY, threshold, 
		 bflPane.rgbOverlay, getMask(icConstant.EDGE_YEPLANE));
	findEdge(bflPane.filterU, threshold, 
		 bflPane.rgbOverlay, getMask(icConstant.EDGE_UEPLANE));
	findEdge(bflPane.filterV, threshold, 
		 bflPane.rgbOverlay, getMask(icConstant.EDGE_VEPLANE));
	*/
    }

    public void addComponents()
    {
	setLayout(new BorderLayout());

	label[0] = new JLabel("Hi");
	label[1] = new JLabel("Lo");
	slider[0]=new JSlider(SwingConstants.VERTICAL, 0, 255, thresholdHi);
	slider[1]=new JSlider(SwingConstants.VERTICAL, 0, 255, thresholdLo);

	JPanel sliderPane[] = new JPanel[slider.length];
	int halfLen = plane.length/sliderPane.length;

	for(int i=0; i<sliderPane.length; i++) {

	    sliderPane[i] = new JPanel();
	    sliderPane[i].setLayout(new BoxLayout(sliderPane[i], BoxLayout.Y_AXIS));
	    sliderPane[i].add(label[i]);

	    slider[i].setPreferredSize(sliderSize);
	    slider[i].setAlignmentX(LEFT_ALIGNMENT);
	    slider[i].setMajorTickSpacing(25);
	    slider[i].setMinorTickSpacing(5);
	    slider[i].setSnapToTicks(true);
	    slider[i].setPaintLabels(true);
	    slider[i].setPaintTicks(true);
	    slider[i].addChangeListener(this);
	    sliderPane[i].add(slider[i]);

	    for(int j=0; j<halfLen; j++) {
		plane[i*halfLen+j] = new JCheckBox(planeName[i*halfLen+j]);
		plane[i*halfLen+j].setName(""+(CONSTANT+i*halfLen+j));
		plane[i*halfLen+j].setAlignmentX(LEFT_ALIGNMENT);
		plane[i*halfLen+j].addItemListener(this);
		sliderPane[i].add(plane[i*halfLen+j]);
	    }
	}

	add(sliderPane[0], BorderLayout.WEST);
	add(sliderPane[1], BorderLayout.EAST);
    }

    public void setEnabled(boolean b)
    {
	for(int i=0; i<label.length; i++)
	    label[i].setEnabled(b);

	for(int i=0; i<slider.length; i++)
	    slider[i].setEnabled(b);

	for(int i=0; i<plane.length; i++)
	    plane[i].setEnabled(b);
    }

    public void propertyChange(PropertyChangeEvent e)
    {
	if(e.getPropertyName().equals(""+icConstant.FILE_OPEN)) {
	    if(e.getOldValue()==null)
		setEnabled(true);

	    int noise = findEdge(bflPane.C, 
		    bflPane.rgbOverlay, getMask(icConstant.EDGE_CPLANE));
	    System.out.println("C: "+noise);
	}
    }

    public void stateChanged(ChangeEvent e)
    {
	int noise;

	if(e.getSource() == slider[0] || e.getSource() == slider[1]) {
	    if(e.getSource() == slider[0]) {
		thresholdHi = slider[0].getValue();
		System.out.println("Hi: "+thresholdHi);

		System.out.println("Hi Noise: ");
		noise = findEdge(bflPane.filterY, thresholdHi, 
		     bflPane.rgbOverlay, getMask(icConstant.EDGE_YPLANE_HI));
		System.out.print("Y: "+noise+"\t");
		noise = findEdge(bflPane.filterU, thresholdHi, 
		     bflPane.rgbOverlay, getMask(icConstant.EDGE_UPLANE_HI));
		System.out.print("U: "+noise+"\t");
		noise = findEdge(bflPane.filterV, thresholdHi, 
		     bflPane.rgbOverlay, getMask(icConstant.EDGE_VPLANE_HI));
		System.out.print("V: "+noise+"\t");
	    }
	    else if(e.getSource() == slider[1]) {
		thresholdLo = slider[1].getValue();
		System.out.println("Lo: "+thresholdLo);

		System.out.println("Lo Noise: ");
		noise = findEdge(bflPane.filterY, thresholdLo, 
		     bflPane.rgbOverlay, getMask(icConstant.EDGE_YPLANE_LO));
		System.out.print("Y: "+noise+"\t");
		noise = findEdge(bflPane.filterU, thresholdLo, 
		     bflPane.rgbOverlay, getMask(icConstant.EDGE_UPLANE_LO));
		System.out.print("U: "+noise+"\t");
		noise = findEdge(bflPane.filterV, thresholdLo, 
		     bflPane.rgbOverlay, getMask(icConstant.EDGE_VPLANE_LO));
		System.out.print("V: "+noise+"\t");
	    }
	    noise = findEdge(bflPane.rgbOverlay, getMask(icConstant.EDGE_COMBO));
	    System.out.println("Combo"+noise);
	    display();
	}
    }

    /**
     * The action event handler of this object.
     */
    public void itemStateChanged(ItemEvent e)
    {
	String action = ((JCheckBox) e.getItemSelectable()).getName();

	//Toggle corresponding bit in mask
	int m = getMask(Integer.parseInt(action));
	bflPane.edgeMask = (e.getStateChange() == ItemEvent.SELECTED) ? 
	    bflPane.edgeMask|m : bflPane.edgeMask&~m;

	System.out.println(action+": "+Integer.toBinaryString(m)+"\t"+
			   Integer.toBinaryString(bflPane.edgeMask));

	display();
    }

    protected void display()
    {
	bflPane.display();
    }

    protected static int getMask(int n)
    {
	if(!icConstant.isEdgePlane(n))
	    return 0;
	return 0x1 << (n - icConstant.EDGE_PLANE);
    }

    protected void initEdgeOverlay(int[][] overlay)
    {
	for (int h = 0; h < PIC_HEIGHT; h++)
	    for (int w = 0; w < PIC_WIDTH; w++)
		overlay[w][h] = 0;
    }

    //Simple horizontal edge
    protected void findEdgeOld(byte[][] plane, int threshold, int[][] overlay, int mask)
    {
	for (int h = 0; h < PIC_HEIGHT; h++) {
	    overlay[0][h] |= mask;
	    for (int w = 1; w < PIC_WIDTH-1; w++) {
		overlay[w][h] = 
		    (Math.abs(icConstant.byte2UInt(plane[w][h]) - 
			      icConstant.byte2UInt(plane[w-1][h]))
		     >= threshold && 
		     Math.abs(icConstant.byte2UInt(plane[w][h]) - 
			      icConstant.byte2UInt(plane[w+1][h]))
		     <= threshold) ?
		    (overlay[w][h] | mask) : (overlay[w][h] & ~mask);
	    }
	    overlay[PIC_WIDTH-1][h] |= mask;
	}
    }

    //roberts cross
    protected int findEdgeCross(byte[][] plane, int threshold, int[][] overlay, int mask)
    {
	int noise = 0;

	for (int h = 0; h < PIC_HEIGHT; h++) {
	    for (int w = 0; w < PIC_WIDTH; w++) {
		//border = edge
		if(h==0 || w==0 || h==PIC_HEIGHT-1 || w==PIC_WIDTH-1)
		    overlay[0][h] |= mask;
		else {
		    if(Math.abs(icConstant.byte2UInt(plane[w][h]) - 
				icConstant.byte2UInt(plane[w+1][h+1]))
		       +
		       Math.abs(icConstant.byte2UInt(plane[w+1][h]) - 
				icConstant.byte2UInt(plane[w][h+1]))
		       >= threshold) {
			overlay[w][h] |= mask;
			noise++;
		    }
		    else
			overlay[w][h] &= ~mask;
		}
	    }
	}
	return noise;
    }

    protected int findEdge(byte[][] plane, int threshold, int[][] overlay, int mask)
    {
	int noise = 0;

	for (int h = 0; h < PIC_HEIGHT; h++) {
	    for (int w = 0; w < PIC_WIDTH; w++) {
		//border = edge
		if(icConstant.byte2UInt(plane[w][h]) >= threshold) {
		    overlay[w][h] = icConstant.addMask(overlay[w][h], mask);
		    noise++;
		}
		else
		    overlay[w][h] = icConstant.subtractMask(overlay[w][h], mask);
	    }
	}
	return noise;
    }

    //For c planes
    protected int findEdge(byte[][] plane, int[][] overlay, int mask)
    {
	return findEdgeCross(plane, 1, overlay, mask);
    }

    //Combo Edge
    protected int findEdge(int[][] overlay, int mask)
    {
	int solidEdge = getMask(icConstant.EDGE_YPLANE_HI) | 
	                getMask(icConstant.EDGE_UPLANE_HI);
	int maybeEdge = getMask(icConstant.EDGE_YPLANE_LO) | 
	                getMask(icConstant.EDGE_UPLANE_LO);
	int cEdge     = getMask(icConstant.EDGE_CPLANE);
	int noise = 0;

	for (int h = 0; h < PIC_HEIGHT; h++) {
	    for (int w = 0; w < PIC_WIDTH; w++) {
		if(icConstant.fitMask(overlay[w][h], solidEdge) ||
		   (icConstant.fitMask(overlay[w][h], cEdge) && 
		    icConstant.fitMask(overlay[w][h], maybeEdge))) {
		    overlay[w][h] = icConstant.addMask(overlay[w][h], mask);
		    noise++;
		}
		else
		    overlay[w][h] = icConstant.subtractMask(overlay[w][h], mask);
	    }
	}
	return noise;
    }

    public boolean fitMask(int val, int mask)
    {
	return icConstant.fitMask(val, mask);
    }

    public static void main(String[] args)
    {
	if(args.length > 1) {
	    System.err.println("java icEdgePanel [<file>]");
	    System.exit(1);
	}

	try {
	    JFrame frame = new JFrame("icEdgePanel");
	    frame.setSize(600, 400);
	    frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

	    icBasicBFLPanel bflPane = new icBasicBFLPanel();
	    icEdgePanel edgePane    = new icEdgePanel(bflPane);

	    frame.getContentPane().setLayout(new BorderLayout());
	    frame.getContentPane().add(bflPane,  BorderLayout.CENTER);
	    frame.getContentPane().add(edgePane, BorderLayout.EAST);
	    frame.setVisible(true);

	    if(args.length == 1)
		bflPane.readFile(args[0]);

	} catch (Exception e) {
	    System.err.println(e);
	}
    }
}
