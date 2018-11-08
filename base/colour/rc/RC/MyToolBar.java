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

/**
 * <p>Title: </p>
 * <p>Description: </p>
 * <p>Copyright: Copyright (c) 2004</p>
 * <p>Company: </p>
 * @author not attributable
 * @version 1.1
 * CHANGE LOG
 * + version 1.1:
 *      - Enhanced interface. Support "Activate" Next/Previous.
 */

/** @non urgent todo
 * Fix scrollbar focus */

import java.util.*;
import java.awt.event.*;

public class MyToolBar extends JPanel {
    public static final String CPLANE_LOG_PREFIX = "CPLANE_LOG";
    BorderLayout borderLayout1 = new BorderLayout();
    JScrollPane jScrollPane1 = new JScrollPane();
    JToolBar jToolBar1 = new JToolBar();
    JLabel jLabel1 = new JLabel();
    ButtonGroup buttonGroup = new ButtonGroup();

    ArrayList imageLabelList = new ArrayList();
    ArrayList radioButtonList = new ArrayList();
    MyToolBar_jLabel1_mouseAdapter mouseAdapter = new MyToolBar_jLabel1_mouseAdapter(this);

    RDRFrame parent;
    JPanel jPanel1 = new JPanel();
    JButton btClearAll = new JButton();

    int currentSelection = 0;
    public MyToolBar(RDRFrame parent) {
        this.parent = parent;
        try {
            jbInit();
        }
        catch(Exception ex) {
            ex.printStackTrace();
        }
    }

    void jbInit() throws Exception {
        this.setLayout(borderLayout1);
        jLabel1.setHorizontalAlignment(SwingConstants.LEADING);
        jLabel1.setHorizontalTextPosition(SwingConstants.TRAILING);
        jLabel1.setText("Thumbnail");
//        jLabel1.addMouseListener(mouseAdapter);
        jScrollPane1.setPreferredSize(new Dimension(77, 70));
        jPanel1.setMaximumSize(new Dimension(58, 32767));
        jPanel1.setPreferredSize(new Dimension(58, 25));
        btClearAll.setToolTipText("");
        btClearAll.setText("Clear");
        btClearAll.addActionListener(new MyToolBar_btClearAll_actionAdapter(this));
        this.add(jScrollPane1, BorderLayout.CENTER);
        jScrollPane1.getViewport().add(jToolBar1, null);
        jScrollPane1.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_NEVER );
        jToolBar1.add(jPanel1, null);
        jPanel1.add(jLabel1, null);
        jPanel1.add(btClearAll, null);

    }

    public void addNewThumbnail(Image image, String description){
        JLabel label = new JLabel(new ImageIcon(image.getScaledInstance(65,50,Image.SCALE_FAST)));
        label.addMouseListener(mouseAdapter);
        label.setToolTipText(description);
        imageLabelList.add(label);
        jToolBar1.add(label);

        JRadioButton jRadioButton1 = new JRadioButton();
        jRadioButton1.setSelected(true);
        jToolBar1.add(jRadioButton1, null);
        radioButtonList.add(jRadioButton1);
        buttonGroup.add(jRadioButton1);
        currentSelection = radioButtonList.size() - 1;

        jToolBar1.repaint();
    }

    public void removeThumbnail(String fileName){
        int i = 0;
        Iterator itRB = radioButtonList.iterator();
        for (Iterator it = imageLabelList.iterator(); it.hasNext() ; i++) {
            JLabel label = (JLabel) it.next();
            if (  label.getToolTipText().equals(fileName)){
                jToolBar1.remove( label);
                jToolBar1.remove( (JRadioButton) radioButtonList.get(i));
            }
        }
        jToolBar1.repaint();
    }

    public String [] getAllBFLFiles(){
        int countBFL = 0;

        for (Iterator it = imageLabelList.iterator();it.hasNext();){
            String l = ( (JLabel) it.next()).getToolTipText();
            if (!l.startsWith(CPLANE_LOG_PREFIX)) countBFL ++;
        }

        String [] listStr = new String[countBFL];
        int i=0;
        for (Iterator it = imageLabelList.iterator();it.hasNext();){
            String l = ( (JLabel) it.next()).getToolTipText();
            if (!l.startsWith(CPLANE_LOG_PREFIX)){
                listStr[i++] = l;
            }
        }
        return listStr;
    }

    public void setSelectedThumbnail(int pos){
//        System.out.println("Min = " + jScrollPane1.getHorizontalScrollBar().getMaximum() +
//                           "Max = " + jScrollPane1.getHorizontalScrollBar().getMaximum() +
//                           "size = " + imageList.size());

        int total = jScrollPane1.getHorizontalScrollBar().getMaximum();
        jScrollPane1.getHorizontalScrollBar().setValue( total * pos / imageLabelList.size() );
//                System.out.println("Set value " +  total * i / imageList.size());
        ((JRadioButton) radioButtonList.get(pos)).setSelected(true);

    }

    public void activateThumbnail(int pos){
        if (this.count() > 0){
            setSelectedThumbnail(pos);
            String filename = ( (JLabel) imageLabelList.get(pos)).
                getToolTipText();
            parent.openBFL(filename);
            currentSelection = pos;
        }
    }

    private int getNextPosition(){
        if (imageLabelList.size() == 0){
            return 0;
        }
        else
            return (currentSelection + 1) % imageLabelList.size();
    }

    private int getPreviousPosition(){
        int n =imageLabelList.size();
        if ( n == 0 ){
            return 0;
        }
        else
            return (currentSelection + n - 1) % n;
    }


    public void activateNextThumbnail(){
        activateThumbnail(getNextPosition());
    }

    public void activatePreviousThumbnail(){
        activateThumbnail(getPreviousPosition());
    }


    void jLabel1_mouseClicked(MouseEvent e) {
//        System.out.println("From " + e.getSource().getClass().getName() );
        JLabel label = (JLabel) e.getSource();
        int i=0;

        for (Iterator it = imageLabelList.iterator();it.hasNext();){
            if ( it.next() == label) {
                activateThumbnail(i);
                return;
            }
            i++;
        }
    }

    void btClearAll_actionPerformed(ActionEvent e) {
        clearAllThumbnail();
    }

    public void clearAllThumbnail() {
        Iterator itRB = radioButtonList.iterator();
        for (Iterator it = imageLabelList.iterator() ; it.hasNext();){
            jToolBar1.remove((JLabel)it.next());
            jToolBar1.remove((JRadioButton)itRB.next());
        }
        jToolBar1.repaint();
        imageLabelList.clear();
        radioButtonList.clear();
        System.gc();
    }

    public int count(){
        return imageLabelList.size();
    }

}

class MyToolBar_jLabel1_mouseAdapter extends java.awt.event.MouseAdapter {
    MyToolBar adaptee;

    MyToolBar_jLabel1_mouseAdapter(MyToolBar adaptee) {
        this.adaptee = adaptee;
    }
    public void mouseClicked(MouseEvent e) {
        adaptee.jLabel1_mouseClicked(e);
    }
}

class MyToolBar_btClearAll_actionAdapter implements java.awt.event.ActionListener {
    MyToolBar adaptee;

    MyToolBar_btClearAll_actionAdapter(MyToolBar adaptee) {
        this.adaptee = adaptee;
    }
    public void actionPerformed(ActionEvent e) {
        adaptee.btClearAll_actionPerformed(e);
    }
}
