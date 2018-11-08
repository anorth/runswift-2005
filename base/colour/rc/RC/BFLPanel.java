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

import java.util.*;
import RoboShare.*;
import Jama.*;
import java.awt.*;
import java.awt.event.*;

/**
 *
 * <p>Title: </p>
 * <p>Description: </p>
 * <p>Copyright: Copyright (c) 2004</p>
 * <p>Company: </p>
 * @author not attributable
 * @version 1.1
* CHANGE LOG :
* + version 1.1
*       - Add drawRotatedBoundingBox : rotated by roll angle (roll is calculated from horizon)
*       - add check in registerPainter to avoid duplicate
 */

public class BFLPanel
    extends DoubleBufferPanel implements Paintee{
    static private final boolean debug = false;
    RDRFrame parent;

    int SCALE = 3;
    int OLDSCALE = -1;

    //for scaling/scrolling
    int STARTX = 0;
    int STARTY = 0;

    int ENDX = BFL.WIDTH;
    int ENDY = BFL.HEIGHT;

    BFLPanel(RDRFrame parent) {
        super();
        this.parent = parent;
        jbInit();
    }

    void jbInit() {
        this.setBackground(SystemColor.control);
        setDoubleBuffered(false);
        this.setMinimumSize(new Dimension(10, 10));
        setPreferredSize(new Dimension(632, 480));
        setToolTipText("Left click to classify");
        setSize(632, 480);
        this.addComponentListener(new BFLPanel_this_componentAdapter(this));

    }



    public void paintBuffer(Graphics g) {
        paintBFL(g);
        notifyPainter(g);
        if (debug) System.out.println("Finisehd painting");

    }

    public BFL getBFL(){
        return parent.bfl;
    }

    private void paintHorizon(Graphics g){
        int [] res = HeadGeometry.calculateHorizon(getBFL().getCamera2WorldMatrix() );
        if (res != null){
            int x1 = Utils.toImagePositionX(res[0]),
                y1 = Utils.toImagePositionY(res[1]),
                x2 = Utils.toImagePositionX(res[2]),
                y2 = Utils.toImagePositionY(res[3]);
            g.setColor(Color.lightGray);
            g.drawLine(x1 * SCALE, y1 * SCALE, x2 * SCALE, y2 * SCALE);
        }
    }

    private void paintBFL(Graphics g){
        if (getBFL() == null) return;


        for (int y = STARTY; y < ENDY; y++) {
            for (int x = STARTX; x < ENDX; x++) {
                drawSquare(x, y, g);
            }
        }
        paintHorizon(g);
        //draw buffered image to screen
        if (debug) System.out.println("Drawn BFL");
    }

    // draw a pixel square with black border

    public void drawSquare(int x, int y, Graphics picg) {
        int xd = x - STARTX;
        int yd = y - STARTY;

        //draw pixel in correct rgb color
        //draw rest of pixel in correct rgb
        picg.setColor(new Color(getBFL().rgbPlane[y][x]));
        picg.fillRect(xd * SCALE, yd * SCALE, SCALE, SCALE);
        //draw black border
        //picg.setColor(Color.black);
        picg.drawRect(xd * SCALE, yd * SCALE, SCALE, SCALE);

    }
    //INFO : try to work around the buffer but doesn't work
    public void drawBoundingBoxDirect(int min_x, int min_y, int max_x,
                                      int max_y) {
        if (directGraphics == null) return;
        System.out.println("drawing");
        int width = max_x - min_x + 1;
        int height = max_y - min_y + 1;
        directGraphics.setColor(Color.black);
        directGraphics.drawRect(min_x * SCALE, min_y * SCALE, width * SCALE,
                                height * SCALE);
    }

    // draw a pixel square with black border
    public void drawBoundingBox(String label, int min_x, int min_y, int max_x,
                                int max_y) {
        drawBoundingBox(label,min_x, min_y, max_x, max_y, Color.black);
    }

    public void drawBoundingBox(String label, int min_x, int min_y, int max_x,
                                int max_y,Color c) {
        Graphics g = getGraphics();
        int width = max_x - min_x + 1;
        int height = max_y - min_y + 1;

        g.setColor(c);
        g.drawRect(min_x * SCALE, min_y * SCALE, width * SCALE, height * SCALE);
//		g.drawLine(SCALE * (min_x + (max_x - min_x)/2), SCALE * (min_y + (max_y - min_y)/2), SCALE * (min_x + (max_x - min_x)/2), SCALE * BFL.HEIGHT);
        g.drawString(label, min_x * SCALE + 1, min_y * SCALE + 1);
    }

    public void drawRotatedBoundingBox(double min_x, double min_y, double max_x, double max_y, double roll) {
        drawRotatedBoundingBox(min_x, min_y, max_x, max_y, roll, Color.black);
    }

    public void drawRotatedBoundingBox(Blob b) {
        drawRotatedBoundingBox(b.xRotMin, b.yRotMin, b.xRotMax, b.yRotMax, b.roll, Color.black);
    }

    public void drawRotatedBoundingBox(Blob b, Color c) {
        drawRotatedBoundingBox(b.xRotMin, b.yRotMin, b.xRotMax, b.yRotMax, b.roll, c);
    }



    public void drawRotatedBoundingBox(double min_x, double min_y, double max_x,
                                       double max_y,double roll, Color c) {
        Graphics g = getGraphics();
        Polygon poly = new Polygon();
        double [][] corners = {
            {min_x, min_y},
            {min_x, max_y},
            {max_x, max_y},
            {max_x, min_y}
        };
        for (int i=0;i<corners.length;i++){
            Point2D p = new Point2D(corners[i][0], corners[i][1]);
            p.rotate(-roll);
            p.toImagePosition();
            poly.addPoint(p.getX() * SCALE, p.getY() * SCALE);
        }

        g.setColor(c);
        g.drawPolygon(poly);
    }



    private boolean isInsideViewingArea(int x,int y){
        return x >= STARTX && x < ENDX && y >= STARTY && y < ENDY;
    }

    public void drawColourBox(int x, int y) {
        drawColourBox(x, y, getGraphics() );
    }

    public void drawColourBox(int x, int y, Graphics g) {
        if (isInsideViewingArea(x,y)){
            int xd = x - STARTX;
            int yd = y - STARTY;
            int color = getBFL().getC(x, y);//C[y][x];
            switch (color) {
                case CommonSense.ORANGE:
                case CommonSense.BLUE:
                case CommonSense.GREEN:
                case CommonSense.YELLOW:
                case CommonSense.PINK:
                case CommonSense.BLUE_ROBOT:
                case CommonSense.RED_ROBOT:
                case CommonSense.GREEN_FIELD:
                case CommonSense.GREY:
                case CommonSense.WHITE:
                case CommonSense.BLACK:
                    g.setColor(CommonSense.indexToColour[color]);
                    break;
                default:
                    g.setColor(CommonSense.indexToColour[CommonSense.NUMBER_USABLE_COLOR]);
                    break;
            }

            g.drawRect(xd * SCALE + 1, yd * SCALE + 1, SCALE - 2, SCALE - 2);
        }

    }

    // draw magenta box indicating classified
    public void drawBox(int x, int y) {
        drawBox(x,y,getGraphics());
    }

    public void drawBox(int x, int y, Graphics picg) {
        if (isInsideViewingArea(x,y)){
            int xd = x - STARTX;
            int yd = y - STARTY;

            switch (getBFL().getC(x, y)/*C[y][x]*/) {
                case CommonSense.GREEN:
                case CommonSense.RED_ROBOT:
                case CommonSense.BLUE_ROBOT:
                case CommonSense.GREY:
//                case CommonSense.GREEN_FIELD:
                    picg.setColor(Color.cyan);
                    break;
                default:
                    picg.setColor(Color.black);
                    break;
            }

            picg.drawRect(xd * SCALE + 1, yd * SCALE + 1, SCALE - 2, SCALE - 2);
        }
    }

    public void drawLine(int x1,int y1, int x2, int y2 , Color c){
        Graphics g = getGraphics();
        g.setColor(c);
        g.drawLine(x1 * SCALE ,y1 * SCALE ,x2 * SCALE ,y2 * SCALE );
    }

    //convert windows view coordinate to the real image coordinate
    public int getPosX(int x){
        return x / SCALE + STARTX;
    }

    public int getPosY(int y){
        return y / SCALE + STARTY;
    }


    ArrayList painterList = new ArrayList();

    public void registerPainter(Painter p){
        if (painterList.indexOf(p) == -1)
            painterList.add(p);
    }

//    public void unregisterPainter(Painter p){
//        if (painterList.indexOf(p) != -1)
//            painterList.remove(p);
//    }

    public void notifyPainter(Graphics g){
        for (Iterator i=painterList.iterator();i.hasNext();){
            ((Painter)i.next() ).paintBFLPanel(g);
        }
    }

    void this_componentResized(ComponentEvent e) {
        int scaleX = getWidth() / getBFL().WIDTH;
        int scaleY = getHeight() / getBFL().HEIGHT;
        SCALE = Math.min(scaleX,scaleY);
        repaint();
    }

}

class BFLPanel_this_componentAdapter extends java.awt.event.ComponentAdapter {
    BFLPanel adaptee;

    BFLPanel_this_componentAdapter(BFLPanel adaptee) {
        this.adaptee = adaptee;
    }
    public void componentResized(ComponentEvent e) {
        adaptee.this_componentResized(e);
    }
}
