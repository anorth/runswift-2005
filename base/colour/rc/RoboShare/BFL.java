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


package RoboShare;

import java.io.*;

import java.awt.*;
import java.awt.image.*;

import Jama.*;

import RC.*;

/**
 *
 * <p>Title: </p>
 * <p>Description: </p>
 * <p>Copyright: Copyright (c) 2004</p>
 * <p>Company: </p>
 * @author Created by Claude Sammut on Tue Aug 24 2004.
 * @version 1.1
* CHANGE LOG:
* +version 1.1 :
*     - Remove support for ERS210
*     - BFL can represent CPlane as well as YUVPlane
*     - Implement Subject (in Observer pattern)
*     - Added getElevation & getElevationRadians functions
 */

public class BFL extends  AbstractSubject{
    static public int WIDTH = RobotDefinition.CPLANE_WIDTH; // default to ERS 7
    static public int HEIGHT = RobotDefinition.CPLANE_HEIGHT; // frame size

    public String fileName = null; // Name of BFL file for external use
    boolean isCPlane = false;
    
    // arrays for colour data
    public byte[][] Y, U, V, C;
    public int[][] rgbPlane;
    public int [][] blobID ; //used to be in blobber, moved here to find perimeter of blob
    Matrix cameraToWorld;
    public void initializeForCPlane(){
        Y = null;
        U = null;
        V = null;
        C = new byte[HEIGHT][WIDTH];
        rgbPlane = new int[HEIGHT][WIDTH];
        isCPlane = true;
        fileName = null;
        cameraToWorld = null;
    }

    public void initializeForYUVPlane() {
        Y = new byte[HEIGHT][WIDTH];
        U = new byte[HEIGHT][WIDTH];
        V = new byte[HEIGHT][WIDTH];
        C = new byte[HEIGHT][WIDTH];
        rgbPlane = new int[HEIGHT][WIDTH];
        isCPlane = false;
        fileName = null;
        cameraToWorld = null;
    }

    // initialise everything depending on robot type
    public BFL(boolean isCPlane) {
        if (isCPlane){
            initializeForCPlane();
        }
        else{
            initializeForYUVPlane();
        }
    }

    public void copy(BFL otherBFL) {
        Y = RCUtils.copyArray2DB(otherBFL.Y);
        U = RCUtils.copyArray2DB(otherBFL.U);
        V = RCUtils.copyArray2DB(otherBFL.V);
        C = RCUtils.copyArray2DB(otherBFL.C);
        rgbPlane = RCUtils.copyArray2DI(otherBFL.rgbPlane);
        isCPlane = otherBFL.isCPlane;
        fileName = otherBFL.fileName;
        cameraToWorld = otherBFL.cameraToWorld;
        notifyObservers();
//        WIDTH = otherBFL.WIDTH;
//        HEIGHT = otherBFL.HEIGHT;
    }

    // reset all colour labels to NOCOLOUR

    public void clear_labels() {
        for (int i = 0; i < HEIGHT; i++) {
            for (int j = 0; j < WIDTH; j++) {
                C[i][j] = CommonSense.NOCOLOUR;
            }
        }
        notifyObservers();
    }

    public static int YUV2RGB(int y, int u, int v) {
        y = y & 0xff;
        u = u & 0xff;
        v = v & 0xff;

        // change range of u and v for rgb conversion
        u -= 128;
        v -= 128;

        int r = (int) ( (1.164 * y) + (1.596 * u));
        int g = (int) ( (1.164 * y) - (0.813 * u) - (0.391 * v));
        int b = (int) ( (1.164 * y) + (1.596 * v));

        if (r < 0) {
            r = 0;
        }
        else if (r > 255) {
            r = 255;
        }
        if (g < 0) {
            g = 0;
        }
        else if (g > 255) {
            g = 255;
        }
        if (b < 0) {
            b = 0;
        }
        else if (b > 255) {
            b = 255;

        }
        int rgb = ( (r << 16) | (g << 8) | b);
        return rgb;
    }

    // reads the BFL file and stores the YUVC information in four arrays.

    public void readFile(String filename) throws FileNotFoundException,IOException{
        this.fileName = null; //HACK if reading is failed, fileName will be null;
        if (isCPlane){
            Y = new byte[HEIGHT][WIDTH];
            U = new byte[HEIGHT][WIDTH];
            V = new byte[HEIGHT][WIDTH];
            isCPlane = false;
        }
        FileInputStream file = new FileInputStream(filename);

        byte[] lineY = new byte[WIDTH];
        byte[] lineU = new byte[WIDTH];
        byte[] lineV = new byte[WIDTH];
        byte[] lineC = new byte[WIDTH];
        byte[] dummyLine = new byte[WIDTH];
        for (int i = 0; i < HEIGHT; i++) {
            file.read(lineY);
            file.read(lineU);
            file.read(lineV);

            file.read(dummyLine);
            file.read(dummyLine);
            file.read(dummyLine);

            file.read(lineC);

            for (int j = 0; j < WIDTH; j++) {
                Y[i][j] = lineY[j];
                U[i][j] = lineU[j];
                V[i][j] = lineV[j];
                C[i][j] = lineC[j];

                rgbPlane[i][j] = YUV2RGB(lineY[j], lineU[j], lineV[j]);
            }
        }
        file.close();
        this.fileName = filename;
        cameraToWorld = null;
        notifyObservers();

    }

    public void readStream(byte[] yuvStream, byte[] cplaneStream) {
        initializeForYUVPlane();
        for (int i = 0; i < HEIGHT; i++) {
            for (int j = 0; j < WIDTH; j++) {
                Y[i][j] = yuvStream[i * WIDTH * 6 + j];
                U[i][j] = yuvStream[i * WIDTH * 6 + WIDTH + j];
                V[i][j] = yuvStream[i * WIDTH * 6 + 2 * WIDTH + j];
                C[i][j] = cplaneStream[i * WIDTH + j];
                rgbPlane[i][j] = YUV2RGB(Y[i][j], U[i][j], V[i][j]);
            }
        }
        cameraToWorld = null;
        notifyObservers();
    }

    public void readYUVStream(byte[] yuvStream) {
        initializeForYUVPlane();
        for (int i = 0; i < HEIGHT; i++) {
            for (int j = 0; j < WIDTH; j++) {
                Y[i][j] = yuvStream[i * WIDTH * 6 + j];
                U[i][j] = yuvStream[i * WIDTH * 6 + WIDTH + j];
                V[i][j] = yuvStream[i * WIDTH * 6 + 2 * WIDTH + j];
                rgbPlane[i][j] = YUV2RGB(Y[i][j], U[i][j], V[i][j]);
            }
        }
        cameraToWorld = null;
        notifyObservers();
    }

    // saves the image

    public void saveFile()  throws FileNotFoundException,IOException {
        if (fileName != null) {
            saveFile(fileName);
        }
    }

//    public static void saveFile(String fileName, byte[] yuvPlane, byte[] cPlane) throws
//        IOException, FileNotFoundException {
//        File file = new File(fileName);
//        FileOutputStream out = new FileOutputStream(file);
//        for (int i = 0; i < HEIGHT; i++) {
//            out.write(yuvPlane, i * WIDTH * 6, WIDTH * 6);
//            out.write(cPlane, i * WIDTH, WIDTH);
//        }
//        out.close();
//
//    }

    public void saveFile(String filename) throws FileNotFoundException,IOException {
        this.fileName = null;
        saveToFile(filename);
        this.fileName = filename;
        System.out.println("BFL file saved successfully [" + filename + "]");
    }

    public void saveToFile(String filename) throws FileNotFoundException,
        IOException {
        FileOutputStream file = new FileOutputStream(filename);

        byte[] lineY = new byte[WIDTH];
        byte[] lineU = new byte[WIDTH];
        byte[] lineV = new byte[WIDTH];
        byte[] lineC = new byte[WIDTH];
        byte[] dummyLine = new byte[WIDTH];

        for (int i = 0; i < HEIGHT; i++) {
            for (int j = 0; j < WIDTH; j++) {
                lineY[j] = Y[i][j];
                lineU[j] = U[i][j];
                lineV[j] = V[i][j];
                lineC[j] = C[i][j];
            }

            file.write(lineY);
            file.write(lineU);
            file.write(lineV);

            file.write(dummyLine);
            file.write(dummyLine);
            file.write(dummyLine);

            file.write(lineC);

        }
    }

    // Median filter

    public void median_filter() {
        byte tmp[][] = new byte[HEIGHT][WIDTH];

        for (int i = 1; i < HEIGHT - 1; i++) {
            for (int j = 1; j < WIDTH - 1; j++) {
                int count[] = new int[CommonSense.NOCOLOUR + 1];
                int max = 0;
                byte colour = CommonSense.NOCOLOUR;

                for (int k = i - 1; k <= i + 1; k++) {
                    for (int l = j - 1; l <= j + 1; l++) {
                        if (i != k || j != l) {
                            if (++count[C[k][l]] > max) {
                                max = count[C[k][l]];
                                colour = C[k][l];
                            }
                        }
                    }
                }

                tmp[i][j] = colour;
            }
        }

        for (int i = 1; i < HEIGHT - 1; i++) {
            for (int j = 1; j < WIDTH - 1; j++) {
                C[i][j] = tmp[i][j];
            }
        }
    }

    public byte getRadius(int x, int y) {
        return (byte) (int) Math.sqrt( (x - WIDTH / 2) * (x - WIDTH / 2) +
                                      (y - HEIGHT / 2) * (y - HEIGHT / 2));
    }

    public boolean outOfBound(int x, int y) {
        return x <= 0 || y <= 0 || x >= WIDTH -1 || y >= HEIGHT -1;
    }

    public byte getY(int x, int y) {
        if (isCPlane)
            return -1;
        else
            return Y[y][x];
    }

    public byte getU(int x, int y) {
        if (isCPlane)
            return -1;
        else
            return U[y][x];
    }

    public byte getV(int x, int y) {
        if (isCPlane)
            return -1;
        else
            return V[y][x];
    }

    public byte getC(int x, int y) {
        return C[y][x];
    }

    public Image toImage(){
        Image image = new BufferedImage(WIDTH,HEIGHT,BufferedImage.TYPE_INT_RGB);
        Graphics g = image.getGraphics();
        for (int x=0;x<WIDTH;x++)
            for (int y=0;y<HEIGHT;y++){
                g.setColor(new Color(rgbPlane[y][x]));
                g.fillRect(x,y,1,1);
            }
        image.flush();
        return image;
    }

    public boolean cplaneToRGB(){
        isCPlane = true;
        boolean res = true;
        for (int i=0;i<HEIGHT;i++){
            for (int j=0;j<WIDTH;j++){
                if (C[i][j] >= 0 && C[i][j] < CommonSense.NUMBER_USABLE_COLOR)
                    rgbPlane[i][j] = CommonSense.indexToColour[ C[i][j] ].getRGB();
                else
                    rgbPlane[i][j] = 0; // alexn:Color.GRAY.getRGB();
            }
        }
        return res;
    }

    public boolean isCPlaneBFL(){
        return isCPlane;
    }


    public int [] getHackedSensorData(){
        int [] res = new int[34];
        byte [] data;
        if (isCPlane){
            data = C[HEIGHT-1];
        }
        else{
            data = Y[0];
        }
        for (int i=0;i<res.length;i++){
            res[i] = Utils.byteToIntLITTLE(data[i*4],data[i*4+1],data[i*4+2],data[i*4+3]);
        }

        return res;
    }

    public Matrix getCamera2WorldMatrix(){
        if (cameraToWorld == null)
            cameraToWorld = getCamera2WorldMatrixSlow();
        return cameraToWorld;
    }

    public Matrix getCamera2WorldMatrixSlow(){
        int[] sensorData = getHackedSensorData();
        double pan = Utils.micro2rad(sensorData[SensorDefinition.ssHEAD_PAN]);
        double tilt = Utils.micro2rad(sensorData[SensorDefinition.ssHEAD_TILT1]);
        double crane = Utils.micro2rad(sensorData[SensorDefinition.ssHEAD_CRANE]);
        return TransformationMatrix.cameraToWorld(pan, tilt,
            crane, RobotDefinition.neckHeight, Utils.radians(RobotDefinition.bodyTilt) );
    }

    public double getElevation(int x,int y){
        x = Utils.toImageCoordinateX(x);
        y = Utils.toImageCoordinateY(y);
        return Utils.getElevation(x,y,getCamera2WorldMatrix());
    }

    public double getElevationRadians(int x,int y){
        x = Utils.toImageCoordinateX(x);
        y = Utils.toImageCoordinateY(y);
        return Utils.getElevationRadians(x,y,getCamera2WorldMatrix());
    }

    public double getRollAngleRad(){
        return HeadGeometry.getRollAngleRad(getCamera2WorldMatrix());
    }

    public void removeColourAtTheBoundary() {
        for (int i = 0; i < WIDTH; i++) {
            C[0][i] = CommonSense.NOCOLOUR;
            C[HEIGHT - 1][i] = CommonSense.NOCOLOUR;
        }
        for (int i = 0; i < HEIGHT; i++) {
            C[i][0] = CommonSense.NOCOLOUR;
            C[i][WIDTH - 1] = CommonSense.NOCOLOUR;
        }

    }

    public void printDebugging(){
        int yuvSum = 0;
        int cpSum = 0 ;
        for (int i=1;i<HEIGHT-1;i++){
            for (int j = 1; j < WIDTH-1; j++) {
                yuvSum += Utils.unsignedByteToInt( Y[i][j] )
                        + Utils.unsignedByteToInt( U[i][j] )
                        + Utils.unsignedByteToInt( V[i][j]);
                cpSum += C[i][j];
            }
        }
        System.out.println("YUV SUM = " + yuvSum);
        System.out.println("CP SUM = " + cpSum);
    }
}
