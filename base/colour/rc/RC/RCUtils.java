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
import java.text.*;
import java.io.*;
import Jama.*;

/**
 *
 * <p>Title: </p>
 * <p>Description: All utils function that belongs to Java RC offline tool </p>
 * <p>Copyright: Copyright (c) 2004</p>
 * <p>Company: </p>
 * @author not attributable
 * @version 1.0
 */
public class RCUtils {

    public static String indent(int level, String str) {
        String res = new String();
        for (int i = 0; i < level; i++) {
            res += "\t";
        }
        res += str;
        return res;
    }

    public static int counter = 0;

    public static String generateUniqueName() {
        Calendar c = Calendar.getInstance();
        String path = RDRApplication.appConf.getYUVDirectory();
        if ( ! new File(path).exists()) path = "";
        String s = new SimpleDateFormat("yyyy_MM_dd_").format(c.getTime());
        while (new File(path, s + new DecimalFormat("0000").format(counter) + ".bfl").exists()) {
            counter++;
        }
        return new File(path, s + new DecimalFormat("0000").format(counter++) + ".bfl").getAbsolutePath();
    }

    public static String generateUniqueName(String path) {
        Calendar c = Calendar.getInstance();
        if ( ! new File(path).exists()) path = "";
        String s = new SimpleDateFormat("yyyy_MM_dd_").format(c.getTime());
        while (new File(path, s + new DecimalFormat("0000").format(counter) + ".bfl").exists()) {
            counter++;
        }
        return new File(path, s + new DecimalFormat("0000").format(counter++) + ".bfl").getAbsolutePath();
    }

    public static byte [][] copyArray2DB(byte [][] a){
        if (a == null) return null;
        else{
            int m = a.length;
            int n = a[0].length;
            byte[][] b = new byte[m][n];
            for (int i = 0; i < m; i++) {
                for (int j = 0; j < n; j++) {
                    b[i][j] = a[i][j];
                }
            }
            return b;
        }
    }

    public static int [][] copyArray2DI(int [][] a){
        if (a == null) return null;
        else{
            int m = a.length;
            int n = a[0].length;
            int[][] b = new int[m][n];
            for (int i = 0; i < m; i++) {
                for (int j = 0; j < n; j++) {
                    b[i][j] = a[i][j];
                }
            }
            return b;
        }
    }

    public static Matrix stringToMatrix(String s){
        if (s.equals("null")) return null;
        StringTokenizer st = new StringTokenizer(s);
        double [][] arr = new double[4][4];

        for (int i=0; i<3; i++)
            for (int j=0; j<3 ;j++ ) {
                arr[i][j] = Double.parseDouble(st.nextToken());
            }
        arr[3][3] = 1;
        return new Matrix(arr);
    }

    public static String matrixToString(Matrix m){
        if (m == null) return null;
        double [][] arr = m.getArray();
        StringBuffer res = new StringBuffer();
        for (int i=0; i<3; i++)
            for (int j=0; j<3 ;j++ ) {
                res.append(arr[i][j]);
                res.append(' ');
            }
        return res.toString();
    }

    public static int compareTo(int a, int b){
        if (a < b) return -1;
        else if (a == 0) return 0;
        else return 1;
    }
}
