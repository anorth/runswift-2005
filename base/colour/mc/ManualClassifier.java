/*

Copyright 2003 The University of New South Wales (UNSW) and National  
ICT Australia (NICTA).

This file is part of the 2003 team rUNSWift RoboCup entry.  You may  
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

// ricky:
// displays simulated cplane of bfl image 
// allows manual modifications to be made to the
// calibration files

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.SwingUtilities;
import java.io.*;

import java.awt.*;
import java.awt.event.*;
import javax.swing.event.*;
import javax.swing.*;
import javax.swing.SwingUtilities;
import java.io.*;



public class ManualClassifier extends JFrame {

    private ManualClassifierPanel mcp;
    private CloseUpDisplayPanel ccdp, ccdp2;
    private JLabel ylabel, ulabel, vlabel, clabel, mlabel, coorlabel;
    private byte selectedColour;
    private boolean maybeBit;
    private JRadioButton[] radioButtons;
    private JFileChooser fileChooser = new JFileChooser();
    private JTextField expandField = new JTextField();
    private JCheckBox maybeCheckBox = new JCheckBox("Maybe Colour");
    
    // constructor
    public ManualClassifier() {
        // init colour
        selectedColour = 0;
        maybeBit = false;
    
        // seting the mcp
        mcp = new ManualClassifierPanel(this);
        mcp.setBorder(BorderFactory.createEtchedBorder());
        mcp.setPreferredSize(new Dimension(800, 300));
        
        // setting the info bar
        JPanel infop = new JPanel();
        infop.setPreferredSize(new Dimension(800, 200));
        infop.setLayout(new GridLayout(1, 10));
        infop.setBorder(BorderFactory.createEtchedBorder());
        
        // setting yuv display
        JPanel yuvp = new JPanel();
        yuvp.setBorder(BorderFactory.createEtchedBorder());
        ylabel = new JLabel("    y    :    n/a ");
        ulabel = new JLabel("    u    :    n/a ");
        vlabel = new JLabel("    v    :    n/a ");
        clabel = new JLabel("  color  :    n/a ");
        mlabel = new JLabel("       ");
        coorlabel = new JLabel("  (x,y)  : (n/a, n/a) ");
	    JLabel expandlabel = new JLabel("Expansion Distance:");
        expandField.setText("0");
        expandField.setHorizontalAlignment(SwingConstants.RIGHT);
        expandField.setMaximumSize(new Dimension(70, 30));

        yuvp.setLayout(new GridLayout(12, 1));
        yuvp.add(ylabel);
        yuvp.add(ulabel);
        yuvp.add(vlabel);
        yuvp.add(clabel);
        yuvp.add(mlabel);
        yuvp.add(coorlabel);
        yuvp.add(expandlabel);
        yuvp.add(expandField);
        
        // setting up close up display
        ccdp = new CloseUpDisplayPanel();
        ccdp.setBorder(BorderFactory.createEtchedBorder());
        ccdp2 = new CloseUpDisplayPanel();
        ccdp2.setBorder(BorderFactory.createEtchedBorder());
        
        // setting up colour option
        JPanel radiop = new JPanel();
        radiop.setBorder(BorderFactory.createEtchedBorder());
        radiop.setLayout(new GridLayout(12, 2));
        ButtonGroup group = new ButtonGroup();
        radioButtons = new JRadioButton[mcp.classifyColours.length];
        for (int i = 0; i < radioButtons.length; i++) {
            JRadioButton rb = new JRadioButton(mcp.colourNames[i]);
            final int fake_i = i;
            rb.addActionListener(new java.awt.event.ActionListener() {
                public void actionPerformed(ActionEvent e) { setSelectedColour((byte)(fake_i)); }});
            radioButtons[i] = rb;
            group.add(rb);
            radiop.add(rb);
        }
        group.setSelected(radioButtons[(int)selectedColour].getModel(), true);
        radiop.add(maybeCheckBox);
        maybeCheckBox.addActionListener(new java.awt.event.ActionListener() {
                public void actionPerformed(ActionEvent e) { setMaybe(((JCheckBox)e.getSource()).isSelected()); }});
        

        // setting up buttons
        JPanel buttonp = new JPanel();
        radiop.setBorder(BorderFactory.createEtchedBorder());
        radiop.setLayout(new GridLayout(12, 1));
        JButton openImageButton = new JButton("Open Image");
        openImageButton.setHorizontalTextPosition(SwingConstants.CENTER);
        openImageButton.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(MouseEvent e) { openFile(false); }});
        buttonp.add(openImageButton);
        JButton openCalButton = new JButton("Open Calibration");
        openCalButton.setHorizontalTextPosition(SwingConstants.CENTER);
        openCalButton.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(MouseEvent e) { openFile(true); }});
        buttonp.add(openCalButton);
        JButton saveButton = new JButton("Save Calibration");
        saveButton.setHorizontalTextPosition(SwingConstants.CENTER);
        saveButton.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(MouseEvent e) { saveFile(); }});
        buttonp.add(saveButton);
        JButton sliceButton = new JButton("Use slicer");
        sliceButton.setHorizontalTextPosition(SwingConstants.CENTER);
        sliceButton.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(MouseEvent e) { 
              if (mcp.getUseSlicer())
              {
                ((JButton)(e.getSource())).setText("Use slicer");
              }
              else
              {
                ((JButton)(e.getSource())).setText("Use CPlane");
              }
              mcp.setUseSlicer(!mcp.getUseSlicer()); 
            }});
        buttonp.add(sliceButton);
        JButton classCube = new JButton("Classified slicer");
        classCube.setHorizontalTextPosition(SwingConstants.CENTER);
        classCube.addMouseListener(new java.awt.event.MouseAdapter() {
          public void mouseClicked(MouseEvent e)
          {
            if (mcp.getShowClassifiedCube())
            {
              ((JButton)e.getSource()).setText("Classified slicer");
            }
            else
            {
              ((JButton)e.getSource()).setText("YUV slicer");
            }
            mcp.setShowClassifiedCube(!mcp.getShowClassifiedCube());
          }
        });
        buttonp.add(classCube);
	JLabel sliceYLabel = new JLabel("ColorCube slices 0-127");
	buttonp.add(sliceYLabel);
	final JTextField sliceYVal = new JTextField("64");
        final JSlider sliceSlider = new JSlider(JSlider.VERTICAL, 0, 127, 64);
	sliceYVal.addActionListener(
				new java.awt.event.ActionListener()
				{
					public void actionPerformed(ActionEvent e)
					{
						int newYVal = (Integer.parseInt(((JTextField)e.getSource()).getText()));
						if (newYVal < 0)
						{
							newYVal = 0;
						}
						else if (newYVal > 127)
						{
							newYVal = 127;
						}
						((JTextField)e.getSource()).setText("" + newYVal);
						mcp.setYSlice(newYVal);
						sliceSlider.setValue(newYVal);
					}
				}
			);
	buttonp.add(sliceYVal);
        sliceSlider.addChangeListener(
          new javax.swing.event.ChangeListener()
          {
            public void stateChanged(ChangeEvent e)
            {
              mcp.setYSlice(((JSlider)e.getSource()).getValue());
	      sliceYVal.setText("" + mcp.getYSlice());
            }
          });
        sliceSlider.setMajorTickSpacing(16);
        sliceSlider.setMinorTickSpacing(1);
        sliceSlider.setPaintTicks(false);
        sliceSlider.setPaintLabels(true);
        sliceSlider.setEnabled(true);
                
        infop.add(yuvp);
        infop.add(ccdp2);
        infop.add(ccdp);
        infop.add(radiop);
        infop.add(buttonp);
	infop.add(sliceSlider);
        
        // setting current
	setSize(new Dimension(800, 600));
	setTitle("MoOoOoOoO");
        getContentPane().setLayout(new BorderLayout());
        getContentPane().add(mcp, BorderLayout.CENTER);
        getContentPane().add(infop, BorderLayout.SOUTH);
    }

    public void openFile(boolean cal) {
        int returnVal = fileChooser.showOpenDialog(this);
        boolean res = false;
        if (returnVal == JFileChooser.CANCEL_OPTION) {
            return;
        }
        String filename = fileChooser.getSelectedFile().getPath();
        if (cal) {
            res = mcp.readCalFile(filename);
        } else {
            res = mcp.readBflFile(filename);
        }
        if (!res) {
            JOptionPane.showMessageDialog(this, "Error opening file");
        } else {
            JOptionPane.showMessageDialog(this, "File opened");
        }
        mcp.repaintAll();
    }

    public void saveFile() {
        int returnVal = fileChooser.showOpenDialog(this);
        if (returnVal == JFileChooser.CANCEL_OPTION) {
            return;
        }
        String filename = fileChooser.getSelectedFile().getPath();
        boolean b = mcp.saveCalFile(filename);
        if (!b) {
            JOptionPane.showMessageDialog(this, "Error saving calibration file");
        } else {
            JOptionPane.showMessageDialog(this, "File saved");
        }
    }

    public boolean readFile(String calfile, String bflfile) {
        boolean c = mcp.readCalFile(calfile);
        boolean b = mcp.readBflFile(bflfile);
        return (b && c);
    }
    
    public void setYUVCInfo(int y, int u, int v, String c, boolean maybeCol) {
        if (y >= 0 && u >= 0 && v >= 0) {
            ylabel.setText("     y    :    "+y);
            ulabel.setText("     u    :    "+u);
            vlabel.setText("     v    :    "+v);
            clabel.setText("   color  : "+c);
            if (maybeCol)
	            mlabel.setText(" maybe ");
	        else
	        	mlabel.setText("       ");
        }
    }
    
    public void setXYInfo(int x, int y) {
    	if (x >= 0 && y >= 0) {
            coorlabel.setText("   (x,y)  : ("+x+","+y+")");
	    }
    }
    
    public void setCloseUpInfo(Color[] rgb, Color[] c, boolean[] maybe) {
        ccdp.set(rgb, null);
	ccdp2.set(c, maybe);
    }
    
    public byte getSelectedColour() {
        return selectedColour;
    }
    
    public void setSelectedColour(byte c) {
        selectedColour = c;
    }
    
    public void setMaybe(boolean m) {
    	maybeBit = m;
    }
    
    public boolean getMaybe() {
    	return maybeBit;
    }
    
    public int getExpandDistance() {
    	int ret = 0;
	    try {
    		ret = Integer.parseInt(expandField.getText());
    	} catch (Exception e) {
	    	expandField.setText("0");
	    }
	    return ret;
    }
    
    // close window
	protected void processWindowEvent(WindowEvent e) {
		super.processWindowEvent(e);
		if (e.getID() == WindowEvent.WINDOW_CLOSING) {
			System.exit(0);
		}
	}

	public static void main(String[] args) {
        ManualClassifier mc = new ManualClassifier();

        //Center the window
        mc.pack();
        Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
        Dimension frameSize = mc.getSize();
        if (frameSize.height > screenSize.height) {
            frameSize.height = screenSize.height;
        }
        if (frameSize.width > screenSize.width) {
            frameSize.width = screenSize.width;
        }
        mc.setLocation((screenSize.width - frameSize.width) / 2, (screenSize.height - frameSize.height) / 2);
        mc.setVisible(true);        
    }    
}
