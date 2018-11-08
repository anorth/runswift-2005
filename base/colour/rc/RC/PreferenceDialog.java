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
import javax.swing.border.*;
import java.awt.event.*;

/**
 * <p>Title: </p>
 * <p>Description: </p>
 * <p>Copyright: Copyright (c) 2004</p>
 * <p>Company: </p>
 * @author not attributable
 * @version 1.0
 */

public class PreferenceDialog
    extends JDialog {
    JPanel panel1 = new JPanel();
    Border border1;
    JLabel jLabel1 = new JLabel();
    JTextField jtfWorkingDir = new JTextField();
    JButton btBrowseWD = new JButton();
    GridBagLayout gridBagLayout2 = new GridBagLayout();
    JPanel jPanel1 = new JPanel();
    GridBagLayout gridBagLayout1 = new GridBagLayout();
    JLabel jLabel2 = new JLabel();
    JTextField jtfYUVDir = new JTextField();
    JButton btBrowseYUVDir = new JButton();
    JFileChooser jfcDirChooser = new JFileChooser();
    JPanel jPanel2 = new JPanel();
    JButton btOK = new JButton();
    GridBagLayout gridBagLayout3 = new GridBagLayout();
    JButton btCancel = new JButton();
    Component component1;
    Component component2;
    JLabel jLabel3 = new JLabel();

    public PreferenceDialog(Frame frame, String title, boolean modal) {
        super(frame, title, modal);
        try {
            jbInit();
            pack();
        }
        catch (Exception ex) {
            ex.printStackTrace();
        }
    }

    public PreferenceDialog() {
        this(null, "", false);
    }

    private void jbInit() throws Exception {
        border1 = BorderFactory.createEmptyBorder(12, 12, 12, 12);
        component1 = Box.createHorizontalStrut(8);
        component2 = Box.createVerticalStrut(8);
        panel1.setLayout(gridBagLayout2);
        panel1.setAlignmentX( (float) 0.5);
        panel1.setAlignmentY( (float) 0.5);
        panel1.setBorder(null);
        jLabel1.setText("Working Directory");
        btBrowseWD.setToolTipText("");
        btBrowseWD.setText("Browse");
        btBrowseWD.addActionListener(new PreferenceDialog_btBrowseWD_actionAdapter(this));
        jtfWorkingDir.setPreferredSize(new Dimension(100, 25));
        jtfWorkingDir.setRequestFocusEnabled(true);
        jtfWorkingDir.setText(RDRApplication.appConf.getWorkingDirectory());
        jPanel1.setLayout(gridBagLayout1);
        jLabel2.setToolTipText("");
        jLabel2.setText("Save Streamed YUV To");
        btBrowseYUVDir.setToolTipText("");
        btBrowseYUVDir.setText("Browse");
        btBrowseYUVDir.addActionListener(new PreferenceDialog_btBrowseYUVDir_actionAdapter(this));
        jtfYUVDir.setToolTipText("");
        jtfYUVDir.setText(RDRApplication.appConf.getYUVDirectory());
        btOK.setOpaque(true);
        btOK.setPreferredSize(new Dimension(65, 23));
        btOK.setText("OK");
        btOK.addActionListener(new PreferenceDialog_btOK_actionAdapter(this));
        jPanel2.setLayout(gridBagLayout3);
        btCancel.setPreferredSize(new Dimension(65, 23));
        btCancel.setToolTipText("");
        btCancel.setVerifyInputWhenFocusTarget(true);
        btCancel.setText("Cancel");
        btCancel.addActionListener(new PreferenceDialog_btCancel_actionAdapter(this));
        jPanel1.setBorder(border1);
        jPanel1.setPreferredSize(new Dimension(500, 200));
        jLabel3.setFont(new java.awt.Font("Dialog", 1, 16));
        jLabel3.setText("RC Preferences");
        panel1.add(jtfWorkingDir, new GridBagConstraints(1, 0, 1, 1, 1.0, 0.0
            , GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
            new Insets(5, 5, 5, 5), 0, 0));
        panel1.add(jLabel1, new GridBagConstraints(0, 0, 1, 1, 0.0, 0.0
            , GridBagConstraints.EAST, GridBagConstraints.NONE,
            new Insets(5, 5, 5, 5), 0, 0));
        panel1.add(btBrowseWD, new GridBagConstraints(2, 0, 1, 1, 0.0, 0.0
            , GridBagConstraints.CENTER, GridBagConstraints.NONE,
            new Insets(5, 5, 5, 5), 0, 0));
        panel1.add(jLabel2, new GridBagConstraints(0, 1, 1, 2, 0.0, 0.0
            , GridBagConstraints.EAST, GridBagConstraints.NONE,
            new Insets(5, 5, 5, 5), 0, 0));
        panel1.add(jtfYUVDir, new GridBagConstraints(1, 1, 1, 1, 1.0, 0.0
            , GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
            new Insets(5, 5, 5, 5), 0, 0));
        panel1.add(btBrowseYUVDir, new GridBagConstraints(2, 1, 1, 1, 0.0, 0.0
            , GridBagConstraints.CENTER, GridBagConstraints.NONE,
            new Insets(5, 5, 5, 5), 0, 0));
        jPanel1.add(jLabel3, new GridBagConstraints(0, 0, 1, 1, 0.0, 0.0
            , GridBagConstraints.CENTER, GridBagConstraints.NONE,
            new Insets(10, 10, 20, 10), 0, 0));
        jPanel1.add(component2, new GridBagConstraints(0, 2, 1, 1, 0.0, 1.0
            , GridBagConstraints.CENTER, GridBagConstraints.BOTH,
            new Insets(0, 0, 0, 0), 0, 0));
        jPanel1.add(jPanel2, new GridBagConstraints(0, 3, 1, 1, 1.0, 0.0
            , GridBagConstraints.SOUTH, GridBagConstraints.HORIZONTAL,
            new Insets(0, 0, 0, 0), 0, 0));
        jPanel2.add(btOK, new GridBagConstraints(1, 0, 1, 1, 0.0, 0.0
                                                 , GridBagConstraints.EAST,
                                                 GridBagConstraints.VERTICAL,
                                                 new Insets(5, 5, 5, 5), 20, 0));
        jPanel2.add(btCancel, new GridBagConstraints(2, 0, 1, 1, 0.0, 0.0
            , GridBagConstraints.CENTER, GridBagConstraints.NONE,
            new Insets(5, 5, 5, 5), 18, 0));
        jPanel2.add(component1, new GridBagConstraints(0, 0, 1, 1, 1.0, 0.0
            , GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
            new Insets(0, 0, 0, 0), 0, 0));
        this.getContentPane().add(jPanel1, BorderLayout.NORTH);
        jPanel1.add(panel1, new GridBagConstraints(0, 1, 1, 1, 1.0, 0.0
            , GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
            new Insets(0, 0, 5, 2), 0, 0));
        jfcDirChooser.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
    }

    public static void main(String[] args) {
        PreferenceDialog dialog = new PreferenceDialog();
        dialog.pack();
        dialog.setVisible(true);
    }

    void btBrowseWD_actionPerformed(ActionEvent e) {
        int ret = jfcDirChooser.showOpenDialog(this);
        if (ret != JFileChooser.CANCEL_OPTION){
            jtfWorkingDir.setText(jfcDirChooser.getSelectedFile().getPath());
        }
    }

    void btOK_actionPerformed(ActionEvent e) {
        RDRApplication.appConf.setWorkingDirectory(jtfWorkingDir.getText());
        RDRApplication.appConf.setYUVDirectory(jtfYUVDir.getText());
        setVisible(false);
    }

    void btBrowseYUVDir_actionPerformed(ActionEvent e) {
        int ret = jfcDirChooser.showOpenDialog(this);
        if (ret != JFileChooser.CANCEL_OPTION){
            jtfYUVDir.setText(jfcDirChooser.getSelectedFile().getPath());
        }

    }

    void btCancel_actionPerformed(ActionEvent e) {
        setVisible(false);
    }
}

class PreferenceDialog_btBrowseWD_actionAdapter implements java.awt.event.ActionListener {
    PreferenceDialog adaptee;

    PreferenceDialog_btBrowseWD_actionAdapter(PreferenceDialog adaptee) {
        this.adaptee = adaptee;
    }
    public void actionPerformed(ActionEvent e) {
        adaptee.btBrowseWD_actionPerformed(e);
    }
}

class PreferenceDialog_btOK_actionAdapter implements java.awt.event.ActionListener {
    PreferenceDialog adaptee;

    PreferenceDialog_btOK_actionAdapter(PreferenceDialog adaptee) {
        this.adaptee = adaptee;
    }
    public void actionPerformed(ActionEvent e) {
        adaptee.btOK_actionPerformed(e);
    }
}

class PreferenceDialog_btBrowseYUVDir_actionAdapter implements java.awt.event.ActionListener {
    PreferenceDialog adaptee;

    PreferenceDialog_btBrowseYUVDir_actionAdapter(PreferenceDialog adaptee) {
        this.adaptee = adaptee;
    }
    public void actionPerformed(ActionEvent e) {
        adaptee.btBrowseYUVDir_actionPerformed(e);
    }
}

class PreferenceDialog_btCancel_actionAdapter implements java.awt.event.ActionListener {
    PreferenceDialog adaptee;

    PreferenceDialog_btCancel_actionAdapter(PreferenceDialog adaptee) {
        this.adaptee = adaptee;
    }
    public void actionPerformed(ActionEvent e) {
        adaptee.btCancel_actionPerformed(e);
    }
}
