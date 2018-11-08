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


package RC;

import java.awt.*;
import javax.swing.*;
import java.awt.event.*;

import RoboShare.*;

/**
 * <p>Title: </p>
 * <p>Description: </p>
 * <p>Copyright: Copyright (c) 2004</p>
 * <p>Company: </p>
 * @author not attributable
 * @version 1.0
 */

public class ColourSelectionDialog extends JDialog {
    JPanel panel1 = new JPanel();
    GridLayout gridLayout1 = new GridLayout();
    public boolean [] colourSelection;
    JButton btOK = new JButton();
    JButton btCancel = new JButton();
    JCheckBox [] cboxPool = null;
    public ColourSelectionDialog(boolean [] selection, Frame frame, String title, boolean modal) {
        super(frame, title, modal);
        colourSelection = new boolean[selection.length];
        for (int i = 0 ; i < selection.length ; i ++ ) {
            colourSelection[i] = selection[i];
        }

        try {
            jbInit();
            pack();
        }
        catch(Exception ex) {
            ex.printStackTrace();
        }
    }

    public ColourSelectionDialog() {
        this(new boolean[0], null, "", false);
    }

    private void jbInit() throws Exception {
        panel1.setLayout(gridLayout1);
        btOK.setText("OK");
        btOK.addActionListener(new ColourSelectionDialog_btOK_actionAdapter(this));

        btCancel.setText("Cancel");
        btCancel.addActionListener(new ColourSelectionDialog_btCancel_actionAdapter(this));
        panel1.setMaximumSize(new Dimension(50, 2000));
        panel1.setMinimumSize(new Dimension(50, 200));
        panel1.setPreferredSize(new Dimension(50, 300));
        gridLayout1.setColumns(1);
        gridLayout1.setRows(15);
        cboxPool = new JCheckBox[colourSelection.length ];
        for (int i=0; i < colourSelection.length ; i++ ) {
             JCheckBox jCheckBox = new JCheckBox();
             jCheckBox.setText(CommonSense.COLOUR_NAME[i]);
             jCheckBox.setSelected(colourSelection[i]);
             panel1.add(jCheckBox,null);
             cboxPool[i] = jCheckBox;
        }

        panel1.add(btOK, null);
        panel1.add(btCancel, null);
        this.getContentPane().add(panel1, BorderLayout.CENTER);
    }

    void btOK_actionPerformed(ActionEvent e) {
        for (int i =0; i < colourSelection.length ;i++ ) {
            colourSelection[i] = cboxPool[i].isSelected();
        }
        setVisible(false);
    }

    void btCancel_actionPerformed(ActionEvent e) {
        setVisible(false);
    }

}

class ColourSelectionDialog_btOK_actionAdapter implements java.awt.event.ActionListener {
    ColourSelectionDialog adaptee;

    ColourSelectionDialog_btOK_actionAdapter(ColourSelectionDialog adaptee) {
        this.adaptee = adaptee;
    }
    public void actionPerformed(ActionEvent e) {
        adaptee.btOK_actionPerformed(e);
    }
}

class ColourSelectionDialog_btCancel_actionAdapter implements java.awt.event.ActionListener {
    ColourSelectionDialog adaptee;

    ColourSelectionDialog_btCancel_actionAdapter(ColourSelectionDialog adaptee) {
        this.adaptee = adaptee;
    }
    public void actionPerformed(ActionEvent e) {
        adaptee.btCancel_actionPerformed(e);
    }
}
