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


package Tool;

import java.util.*;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.event.*;

import RC.*;

/**
 * <p>Title: </p>
 * <p>Description: </p>
 * <p>Copyright: Copyright (c) 2004</p>
 * <p>Company: </p>
 * @author not attributable
 * @version 1.0
 */

public class BlobViewer extends JFrame {
    GridBagLayout gridBagLayout1 = new GridBagLayout();
    JScrollPane jScrollPane1 = new JScrollPane();
    JList lstBlob = new JList();
    JButton btBringToFront = new JButton();
    JButton btExit = new JButton();

    RDRApplication app = null;
    public BlobViewer(RDRApplication parent) {
        super("Blob Viewer");
        app = parent;
        try {
            jbInit();
            showBlobs();
        }
        catch(Exception ex) {
            ex.printStackTrace();
        }
    }

    void jbInit() throws Exception {
        this.getContentPane().setLayout(gridBagLayout1);
        btBringToFront.setText("Bring to Front");
        btBringToFront.addActionListener(new BlobViewer_btBringToFront_actionAdapter(this));
        lstBlob.getSelectionModel().addListSelectionListener(new
            ListSelectionListener() {
            public void valueChanged(ListSelectionEvent e) {
                lstBlob_selectionChanged(e);
            }
        }
        );
        btExit.setText("Close");
        btExit.addActionListener(new BlobViewer_btExit_actionAdapter(this));
        getContentPane().add(jScrollPane1,    new GridBagConstraints(0, 0, 1, 3, 1.0, 1.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));
        getContentPane().add(btBringToFront,   new GridBagConstraints(1, 0, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL, new Insets(0, 0, 0, 0), 0, 0));
        getContentPane().add(btExit,  new GridBagConstraints(1, 1, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL, new Insets(0, 0, 0, 0), 0, 0));
        jScrollPane1.getViewport().add(lstBlob, null);
    }

    public void showBlobs(){
        ArrayList blobList = app.blobber.getDisplayingBlobList();
        String [] blobData = new String[blobList.size()];
        for (int i=0;i<blobList.size();i++){
            Blob b = (Blob)blobList.get(i);
            blobData[i] = "" + ( b.getWidth() * b.getHeight() ) ;
        }
        lstBlob.setListData(blobData);
        setVisible(true);
    }

    void btExit_actionPerformed(ActionEvent e) {
        app.blobber.setSpecialBlob(null,null);
        app.repaint();
        this.dispose();
    }

    void btBringToFront_actionPerformed(ActionEvent e) {
        if (lstBlob.getSelectedIndex() != -1){
            ArrayList blobList = app.blobber.getDisplayingBlobList();
            int i = lstBlob.getSelectedIndex();
            Object b = blobList.get(i);
            blobList.remove(b);
            blobList.add(0,b);
        }
    }

   void lstBlob_selectionChanged(ListSelectionEvent e){
       int i = lstBlob.getSelectedIndex();
       ArrayList blobList = app.blobber.getDisplayingBlobList();
       app.blobber.setSpecialBlob((Blob) blobList.get(i),null);
       app.repaint();
   }
}

class BlobViewer_btExit_actionAdapter implements java.awt.event.ActionListener {
    BlobViewer adaptee;

    BlobViewer_btExit_actionAdapter(BlobViewer adaptee) {
        this.adaptee = adaptee;
    }
    public void actionPerformed(ActionEvent e) {
        adaptee.btExit_actionPerformed(e);
    }
}

class BlobViewer_btBringToFront_actionAdapter implements java.awt.event.ActionListener {
    BlobViewer adaptee;

    BlobViewer_btBringToFront_actionAdapter(BlobViewer adaptee) {
        this.adaptee = adaptee;
    }
    public void actionPerformed(ActionEvent e) {
        adaptee.btBringToFront_actionPerformed(e);
    }
}
