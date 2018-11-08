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


/**
 * <p>Title: </p>
 * <p>Description: </p>
 * <p>Copyright: Copyright (c) 2004</p>
 * <p>Company: </p>
 * @author not attributable
 * @version 1.1
 * CHANGE LOG
 * + version 1.1:
 *      - Add enable/disable functionality (only one tool is "enabled" at a time)
 */
import java.awt.event.*;
import javax.swing.*;
import java.awt.*;
import java.util.*;
import RoboShare.*;

public class GenericTool extends JPanel implements MouseMotionListener,
    MouseListener, Painter, Subject{
    RDRApplication parent;
    boolean enabled;

    public GenericTool(RDRApplication parent){
        this.parent = parent;
    }

    public BFL getBFL(){
        return parent.bfl;
    }

    public ArrayList getBlobList(){
        return parent.blobber.visibleBlobList;
    }

    public void setToolEnabled(boolean b){
        enabled = b;
    }

    //interested methods.
    public void mouseDragged(MouseEvent e){}
    public void mouseMoved(MouseEvent e){}
    public void mousePressed(MouseEvent e){}
    public void mouseClicked(MouseEvent e){}
    public void mouseEntered(MouseEvent e){}
    public void mouseExited(MouseEvent e){}
    public void mouseReleased(MouseEvent e){}
    public String name(){
        return "GenericTool";
    }

    ///////////////////////////// PAINTER INTERFACE ///////////////////////////
//    public void paint(Graphics g){}
    public void paintBFLPanel(Graphics g){}

    ///////////////////////////// SUBJECT INTERFACE ///////////////////////////
    private ArrayList observers = new ArrayList();

    public void addObserver(Observer o) {
        observers.add(o);
    }

    public void removeObserver(Observer o) {
        observers.remove(o);
    }

    protected void notifyObservers() {
        // loop through and notify each observer
        Iterator i = observers.iterator();
        while (i.hasNext()) {
            Observer o = (Observer) i.next();
            o.update(this);
        }
    }

}
