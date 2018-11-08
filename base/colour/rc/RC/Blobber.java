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
 * CHANGE LOG :
 *   +  version 1.1 :
 *         - More controllable painting stuff. In charge of painting highlighted and special blobs
 *         - getBlobAtCursor finds among displayBlobList (not visibleBlobList)
 *   + version 1.2:
*          - fix blobber getBlobAtCursor to get what ever is being displayed
*          - fix bug contain pixel in rorated bounding box.
 */

import java.util.*;

import java.awt.*;

import RoboShare.*;

/** @maybetodo
* seperate Blobber Painter from blobber */

public class Blobber implements Painter, Subject, Observer{
    private static final boolean debugPainting = false;
    private static final Color SPECIAL_COLOR = Color.CYAN;

    public static final  int PAINT_ALL_BLOB = 0;
    public static final  int PAINT_HIGHLIGHTED_ONLY = 1;
    public static final  int PAINT_NO_BLOB = 2;
    public static final  int PAINT_NON_HIGHLIGHTED = 3;


    BFLPanel panel = null;
    ArrayList [] blobList = null;

    int width, height;
    BFL bfl = null;
    int nextX [] = {-1,0,1,0};
    int nextY [] = {0,1,0,-1};
    int currentColor = CommonSense.ALLCOLOUR;
    public static boolean IS_WANTED_BLOB[] = {
            true,
            true,
            false, //GREEN
            true,
            true,
            true,
            true,
            false,  //GREEN FIELD
            false,  //GREY
            false,  //WHITE
            false,  //BACK_GROUND
            false,  //FIELD_LINE
            false,  //FIELD_BORDER
        };

    double roll;
    int iPaintOption = 0;

    ArrayList visibleBlobList = new ArrayList();

    ArrayList displayBlobList = new ArrayList();
    Color highlightColor = Color.MAGENTA;
    Blob firstBlob = null,
        secondBlob = null;

    public Blobber(BFLPanel panel) {
        this.panel = panel;
        bfl = panel.getBFL();
        width = bfl.WIDTH;
        height = bfl.HEIGHT;
        reset();
    }

    public Blobber(BFL  bfl) {
        this.bfl = bfl;
        width = bfl.WIDTH;
        height = bfl.HEIGHT;
        reset();
    }


    private void reset(){
        blobList =  new ArrayList[CommonSense.NUMBER_USABLE_COLOR];
        for (int i=0;i<blobList.length;i++)
            blobList[i] = new ArrayList();
        bfl.blobID = new int[width][height]; //filled with '0'

    }

    private boolean inBound(int x,int y){
        return x > 0 && x < width - 1 && y > 0 && y < height - 1;
    }

    private void fill4(int u,int v, int bid, Blob b ){
        bfl.blobID[u][v] = bid;
        b.add(u,v);
        for (int k=0;k<4;k++){
            int uk = u + nextX[k],
                vk = v + nextY[k];
            if (inBound(uk,vk) &&
                bfl.blobID[uk][vk] == 0 &&
                bfl.getC(uk,vk) == b.colour){
                fill4(uk, vk,bid,b);
            }
        }
    }

    private void fill4BFS(int u,int v, int bid, Blob b ){
//        System.out.println("Doing " + bid);
        Point2D rotatedUV = Utils.getRotatedPixel(u,v,roll);
        ArrayList queue = new ArrayList();
        bfl.blobID[u][v] = bid;
        b.add(u,v);
        b.addRotated(rotatedUV.getRawX(), rotatedUV.getRawY());
        queue.add(new interval(u,v) );
        while (queue.size() > 0){
            u = ( (interval)queue.get(0)).lo;
            v = ( (interval)queue.get(0)).hi;
            queue.remove(0);
            for (int k = 0; k < 4; k++) {
                int uk = u + nextX[k],
                    vk = v + nextY[k];
                if (inBound(uk, vk) &&
                    bfl.getC(uk, vk) == b.colour) {
                    if (bfl.blobID[uk][vk] == 0 ){
                        bfl.blobID[uk][vk] = bid;
//                        if (bid == 1) {
//                            System.out.println(" --- " + uk + " " + vk);
//                        }
                        b.add(uk, vk);
                        rotatedUV = Utils.getRotatedPixel(uk, vk, roll);
                        b.addRotated(rotatedUV.getRawX(), rotatedUV.getRawY());
                        queue.add(new interval(uk, vk));
                    }
                }
                else{
                    b.perimeter += 1;
                }
            }
        }
    }

    ArrayList getSegments(int y){
        ArrayList res = new ArrayList();
        int color = bfl.getC(0,y);
        int startX = 0;
        for (int x = 1; x < width; x++){
            if (bfl.getC(x,y) != color){
                if (color < CommonSense.NUMBER_USABLE_COLOR && IS_WANTED_BLOB[color])
                    res.add(new DisjointSet(color, startX, x-1,y , roll, bfl));
                startX = x;
                color = bfl.getC(x,y);
            }
        }
        if (color < CommonSense.NUMBER_USABLE_COLOR && IS_WANTED_BLOB[color])
            res.add(new DisjointSet(color, startX, width - 1,y, roll, bfl));
        return res;
    }

    void addNewBlob(ArrayList prevSegments, ArrayList thisSegments){
        for (int i = 0; i < prevSegments.size() ; i++){
            DisjointSet ds = (DisjointSet) prevSegments.get(i);
            boolean finished = true;

            for (Iterator nextI = thisSegments.iterator() ; nextI.hasNext();){
                if ( ((DisjointSet)nextI.next() ).getroot() == ds.getroot()){
                    finished = false;
                    break;
                }
            }

            if (!finished) continue;

            for (int i2 = 0; i2 < i ; i2 ++){
                if ( ((DisjointSet)prevSegments.get(i2) ).getroot() == ds.getroot()){
                    finished = false;
                    break;
                }
            }

            if (finished){
                if (ds.getroot().b.area >= 10)
                    blobList[ds.colour].add(ds.getroot().b);
            }
        }
    }

//    void fillRow(int y, int x1, int x2, int c){
//        for (int i = x1 ; i <= x2 ; i++) bfl.C[y][i] = (byte)c;
//    }
//
//    void fillSquare(int y1, int y2, int x1, int x2, int c){
//        for (int y = y1 ; y <= y2 ; y++) fillRow(y, x1,x2,c);
//    }
//        fillSquare(0,159,0,207,2);
//        fillSquare(10, 159, 10, 207, 0);
//        fillSquare(7, 9, 0, 0, 0);
//        fillSquare(7, 9, 200, 200, 0);
//        fillSquare(0,80,0,103,0);
//        fillSquare(0,80,104,207,1);
//        fillSquare(81,159,0,103,1);
//        fillSquare(81,159,104,207,0);

    public void formBlobsFloodFill() {
        reset();
        int buid = 0;
        roll = bfl.getRollAngleRad();
        for (int i = 1; i  < width - 1; i++) {
            for (int j = 1; j < height - 1; j++) {
                int c = bfl.getC(i, j);
                if (c < blobList.length && IS_WANTED_BLOB[c] &&
                    bfl.blobID[i][j] == 0) {
                    buid ++;
                    Blob b = new Blob(c, bfl, roll);
                    b.id = buid;
                    //b.perimeter = 0;
                    fill4BFS(i, j, buid, b);
                    if (b.area > 10)
                        blobList[c].add(b);

                }
            }
        }
        updateVisibleBlobList();
    }

    /**
     *  Form blob using disjoint algorithms.
     */

    public void formBlobsDisjoint(){
        reset();
        roll = bfl.getRollAngleRad();

        ArrayList lastSegments = getSegments(1);

        for (int y=2; y<height-1; y++){

            ArrayList thisSegments = getSegments(y);

            int i = 0 , j = 0 ;
            while (i < lastSegments.size() && j < thisSegments.size() ){
                DisjointSet a = (DisjointSet) lastSegments.get(i);
                DisjointSet b = (DisjointSet) thisSegments.get(j);
                if (a.colour == b.colour && a.canBeJoined(b) && a.getroot() != b.getroot() ){
                    a.join(b);
                }
                if (b.endX > a.endX){
                    i++;
                }
                else if (a.endX > b.endX){
                    j ++;
                }
                else {
                    i++; j++;
                }
            }
            addNewBlob(lastSegments,thisSegments);
            lastSegments = thisSegments;
        }
        addNewBlob(lastSegments,new ArrayList());
        updateVisibleBlobList();
        System.out.println("number of blob  " + visibleBlobList.size());
    }



    public void formBlobs(){
        bfl.removeColourAtTheBoundary();
        formBlobsFloodFill();
//        formBlobsDisjoint
    }

    public void setPaintOption(int opt) {
        if (iPaintOption != opt) {
            iPaintOption = opt;
        }
    }

    public void clearDisplayBlobList(){
        displayBlobList.clear();
    }

    public void addDisplayingBlob(Blob b){
        displayBlobList.add(b);
    }

    public void removeDisplayingBlob(Blob b){
        displayBlobList.remove(b);
    }

    public void setDisplayColor(Color c){
        highlightColor = c;
    }

    public void setSpecialBlob(Blob first,Blob second){
        firstBlob = first;
        secondBlob = second;
    }

    void updateVisibleBlobList(){
        visibleBlobList = new ArrayList();
        for (int color = 0; color < blobList.length; color++)
            if (color == currentColor || currentColor == CommonSense.ALLCOLOUR) {
                ArrayList l = blobList[color];

                for (Iterator i = l.iterator(); i.hasNext(); ) {
                    Blob b = (Blob) i.next();
                    visibleBlobList.add(b);
                    System.out.println("Got blob " + b.toString());
                }
            }
        displayBlobList.clear();
        displayBlobList.addAll(visibleBlobList);
        notifyObservers();
    }

    private void showVisibleBlobs() {
        for (Iterator i = visibleBlobList.iterator(); i.hasNext(); ) {
            Blob b = (Blob) i.next();
//                    System.out.println("- " + b);
            panel.drawRotatedBoundingBox(b);
        }
    }

    private void displayBlobs() {
        for (Iterator i = displayBlobList.iterator(); i.hasNext(); ) {
            Blob b = (Blob) i.next();
//                    System.out.println("- " + b);
            panel.drawRotatedBoundingBox(b,highlightColor);
        }
    }

    public void paintBFLPanel(Graphics g) {
        if (iPaintOption == PAINT_ALL_BLOB || iPaintOption == PAINT_NON_HIGHLIGHTED){
            showVisibleBlobs();
        }

        if (iPaintOption == PAINT_ALL_BLOB || iPaintOption == PAINT_HIGHLIGHTED_ONLY){
            displayBlobs();
        }

        if (firstBlob != null){
            panel.drawRotatedBoundingBox(firstBlob,SPECIAL_COLOR);
        }

        if (secondBlob != null){
            panel.drawRotatedBoundingBox(secondBlob,SPECIAL_COLOR);
        }

        if (debugPainting) System.out.println("Blobber : Done painting ");
    }

    public Blob getBlobAtCursor(int x, int y){
        ArrayList blobList = getDisplayingBlobList();
        for (Iterator i = blobList.iterator(); i.hasNext(); ) {
            Blob b = (Blob) i.next();
            //System.out.println("- " + b);
            if (b.contain(x, y))
                return b;
        }
        return null;
    }

    public Blob [] getAllBlobsAtCursor(int x, int y) {
        ArrayList blobList = getDisplayingBlobList();
        int count = 0;
        for (Iterator i = blobList.iterator(); i.hasNext(); ) {
            Blob b = (Blob) i.next();
            //System.out.println("- " + b);
            if (b.contain(x, y))
                count ++;
        }
        Blob [] result = new Blob[count];
        count = 0;
        for (Iterator i = blobList.iterator(); i.hasNext(); ) {
            Blob b = (Blob) i.next();
            //System.out.println("- " + b);
            if (b.contain(x, y))
                result[count ++] = b;
        }

        return result;
    }

    public ArrayList getBlobList() {
        return visibleBlobList;
    }

    public ArrayList getDisplayingBlobList() {
        switch (iPaintOption){
            case PAINT_ALL_BLOB:
            case PAINT_NON_HIGHLIGHTED:
                return visibleBlobList;
            case PAINT_HIGHLIGHTED_ONLY:
                return displayBlobList;
            case PAINT_NO_BLOB:
            default:
                return new ArrayList();
        }
    }

    public void setWantedColour(boolean [] colourSelection){
        for (int i = 0 ; i < IS_WANTED_BLOB.length ; i ++ ) {
            IS_WANTED_BLOB[i] = colourSelection[i];
        }
    }
    public void showOnlyBlobColor(int color, String colorName){
        if (colorName.equals("ALL")){
            currentColor = CommonSense.ALLCOLOUR;
        }
        else{
            currentColor = color;
        }
        updateVisibleBlobList();
    }

    ///////////////////////////// SUBJECT INTERFACE ///////////////////////////
    private ArrayList observers = new ArrayList();

    public void addObserver(Observer o) {
        observers.add(o);
    }

    public void removeObserver(Observer o) {
        observers.remove(o);
    }

    private void notifyObservers() {
        // loop through and notify each observer
        Iterator i = observers.iterator();
        while (i.hasNext()) {
            Observer o = (Observer) i.next();
            o.update(this);
        }
    }

    ///////////////////////////// OBSERVER INTERFACE ///////////////////////////
    public void update(Subject s){
        formBlobs();
    }
}
