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
 * @version 1.0
 */

import java.util.*;
import java.io.*;

public class MyConfiguration {

    public static String CONFIG_FILE_NAME = "rc.conf";

    Properties properties = new Properties();
    File file = new File(CONFIG_FILE_NAME);

    public static  String propBallImageDir = "BallImageDir";
    static public String propGoalImageDir = "GoalImageDir";
    static public String propBeaconImageDir = "BeaconImageDir";

    public static  String propBallRDR = "BallRDR";
    static public String propGoalRDR = "GoalRDR";
    static public String propBeaconRDR = "BeaconRDR";
    static public String propBlobRDR = "BlobRDR";

    static public String propOutputRDR = "BlobOutputRDR";
    static public String propEvaluateBFL = "EvaluateBFL";
    static public String propTestingBFL = "TestingBFL";

    public MyConfiguration() {
        if (!file.exists()) {
            try {
                file.createNewFile();
            }
            catch (IOException ex) {
                System.err.println("Cannot create file " +
                                   file.getAbsolutePath());
            }
        }
        try {
            InputStream input = new FileInputStream(file);
            properties.load(input);
            input.close();
            properties.list(System.out);
        }
        catch (FileNotFoundException ex1) {
            ex1.printStackTrace();
        }
        catch (IOException ex1) {
            ex1.printStackTrace();
        }
    }

    public void shutDown() {
        try {
            OutputStream output = new FileOutputStream(file);
            properties.store(output, "RC Tool Configuration file");
            output.close();
        }
        catch (FileNotFoundException ex) {
            ex.printStackTrace();
        }
        catch (IOException ex) {
            ex.printStackTrace();
        }
    }

    ////////////////////////////// public methods /////////////////////////////
    public String[] getRecentBFLFiles() {
        return null;
    }

    public String[] getRecentRDRFiles() {
        return null;
    }

    public String getRecentOpenDirectory() {
        return properties.getProperty("RecentDirectory", "");
    }

    public void setRecentOpenDirectory(String dir) {
        properties.setProperty("RecentDirectory", dir);
    }

    public String getWorkingDirectory() {
        return properties.getProperty("WorkingDirectory", "");
    }

    public void setWorkingDirectory(String dir) {
        properties.setProperty("WorkingDirectory", dir);
    }

    public String getYUVDirectory() {
        return properties.getProperty("YUVDirectory", "");
    }

    public void setYUVDirectory(String dir) {
        properties.setProperty("YUVDirectory", dir);
    }

    public String getLastBlobIntervalRDR() {
        return properties.getProperty("BlobIntervalRDR", "");
    }

    public void setLastBlobIntervalRDR(String file) {
        properties.setProperty("BlobIntervalRDR", file);
    }

    public String getLastBlobRelationRDR() {
        return properties.getProperty("BlobIntervalRDR", "");
    }

    public void setLastBlobRelationRDR(String file) {
        properties.setProperty("BlobIntervalRDR", file);
    }


    public String getLastColourRDR() {
        return properties.getProperty("LastColourRDR", "");
    }

    public void setLastColourRDR(String file) {
        properties.setProperty("LastColourRDR", file);
    }

    public String getProperty(String s){
        return properties.getProperty(s,"");
    }

    public void setProperty(String key, String value){
        properties.setProperty(key,value);
    }

    public String getLastBFLFileChooser() {
        return properties.getProperty("LastBFLFileChooser", "");
    }

    public void setLastBFLFileChooser(String dir) {
        properties.setProperty("LastBFLFileChooser", dir);
    }

    public String getLastRDRFileChooser() {
        return properties.getProperty("LastRDRFileChooser", "");
    }

    public void setLastRDRFileChooser(String dir) {
        properties.setProperty("LastRDRFileChooser", dir);
    }

    public String getLastBFLDirectory() {
        return properties.getProperty("LastBFLDirectory", "");
    }

    public void setLastBFLDirectory(String dir) {
        properties.setProperty("LastBFLDirectory", dir);
    }

    public String getLastObjectRDRFile() {
        return properties.getProperty("LastObjectRDRFile", "");
    }

    public void setLastObjectRDRFile(String dir) {
        properties.setProperty("LastObjectRDRFile", dir);
    }

    public void setRLOGDirectory(String dir) {
        properties.setProperty("RLOGDirectory", dir);
    }

    public String getRLOGDirectory() {
        return properties.getProperty("RLOGDirectory", "");
    }


    public void setDefaultDirectory(String dir) {
        properties.setProperty("DefaultDirectory", dir);
    }

    public String getDefaultDirectory() {
        return properties.getProperty("DefaultDirectory", "");
    }

    public static void main(String[] args) {
    }
}
