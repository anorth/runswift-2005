

/**
 * <p>Title: </p>
 * <p>Description: </p>
 * <p>Copyright: Copyright (c) 2004</p>
 * <p>Company: </p>
 * @author not attributable
 * @version 1.0
 */

import java.awt.*;
import javax.swing.*;
import javax.swing.border.*;
import java.awt.event.*;
import java.net.*;
import java.io.*;
import java.util.*;

public class WorldModelDisplay extends Thread {
    private static final boolean debugMsg = false;
    private static final boolean delayDebugMsg = false;
    private static final boolean hasTestButton = false;
    private static final boolean showInfoPanel = true;
    private static final boolean replaceLongCommandDebugMsg = false;

    private static int counter = 0;

    private int portNumber;
    private GLDisplay d;

    public WorldModelDisplay(String port) {
        try {
            portNumber = Integer.parseInt(port);
        } catch (NumberFormatException e) {
            System.err.println(e);
        }
    }

    public void run() {
        try {

            ServerSocket ss = new ServerSocket(portNumber);
            System.out.println("WorldModelDisplay (receive," + portNumber + "): ready");
            while (true){
                Socket s = ss.accept();
                System.out.println("WorldModelDisplay got new connection.");
                BufferedReader in = new BufferedReader(new InputStreamReader(s.
                    getInputStream()));

                double[] data = new double[200]; // temporary for now
                int loc = 0; // current loc in data array
                boolean inData = false;

                for (String line = in.readLine(); line != null;
                     line = in.readLine()) {

                    //line = line.trim();
                    if (!inData && line.startsWith(RoboConstant.RINFO_DELIMA_S)) { // start of data
                        if (debugMsg) {
                            System.out.println("Start of info");
                        }
                        inData = true;
                    }
                    else if (inData &&
                             line.startsWith(RoboConstant.RINFO_DELIMA_E)) { // end of data
                        if (debugMsg) {
                            System.out.println("end of info");
                        }
                        inData = false;
                        processData(data, loc);
                        loc = 0;
                    }
                    else if (inData) { // robot data
                        try {
                            data[loc++] = Double.parseDouble(line);
                        }
                        catch (NumberFormatException e) {
                            System.err.println(
                                "GLDisplay.Reader.run: failed for parse info: " +
                                line);
                            System.err.println(e);
                        }
                    }
                    else {
                        // execution should not reach here
                        // frame count data is sent through another port
                        System.err.println(
                            "RoboCommander: This should not get printed");
                    }
                }

                // ignore the rest
                System.out.println("WorldModelDisplay: Connection Closed");
                in.close();
                s.close();
            }
//            ss.close();

        } catch (IOException e) {
            System.err.println("GLDisplay.Reader.run: IOException");
            e.printStackTrace();
        }

        // close world model display
        if (d!=null) {
            d.dispose();
        }
    }

    // display the world model
    public void processData(double[] data, int size) {
        boolean first = true;
        if (RoboWirelessBase.worldModelDebug) {
            counter++;
        }
//            parent.getHumanControl().writeLog("WORLDMODELHERE\n", 3);
        /*
        for (int i = 0 ; i < size ; i++) {
            if (first) {
                first = false;

                parent.getHumanControl().data = data;
                long timeNow = System.currentTimeMillis();
                parent.getHumanControl().writeLog("#"+timeNow+":" + (int)data[i], 3);
            // add commas if not first
            } else {
                // print as an integer
                if (i == 1 || i == 2 || i == 6 || i == 46 || i == 56) {
                    parent.getHumanControl().writeLog("," + (int)data[i], 3);
                // print as a double
                } else {
                    parent.getHumanControl().writeLog("," + data[i], 3);
                }
            }
        }

        parent.getHumanControl().writeLog("\n", 3);
*/

        Integer playerNum = new Integer((int) data[0]);
        if (RoboWirelessBase.worldModelDebug) {
            System.out.println("Counter: "+ counter + ", Size = "+size);
            System.out.println("[0] = "+playerNum.intValue()+" (playerNum)");
        }
        if (d == null) {
            d = new GLDisplay(playerNum.intValue());
        }
        d.processData(playerNum.intValue(), data, 1, size - 1);
        if (RoboWirelessBase.worldModelDebug) {
            System.out.println("=======================================");
        }
    }

    public static void main(String [] args){
        if (args.length > 1){
            System.out.println("Usage: java WorldModelDisplay portNo");
            System.exit(1);
        }
        else if (args.length == 1){
            WorldModelDisplay wm = new WorldModelDisplay(args[0]);
            wm.start();
        }
        else if (args.length == 0){
            WorldModelDisplay wm1 = new WorldModelDisplay("5019");
            wm1.start();
            WorldModelDisplay wm2 = new WorldModelDisplay("5020");
            wm2.start();
            WorldModelDisplay wm3 = new WorldModelDisplay("5021");
            wm3.start();
            WorldModelDisplay wm4 = new WorldModelDisplay("5022");
            wm4.start();
        }

    }
}
