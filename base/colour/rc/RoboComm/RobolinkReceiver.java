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


package RoboComm;

import java.io.*;
import RoboShare.*;
import RC.*;

public class RobolinkReceiver extends YUVReceiver
{
    public static final int YUVPLANE_TYPE = 3;
    public static final int CPLANE_TYPE = 2;

    StreamReader reader = null;

    public void receiveFromFile(String filename){
        StreamReader reader = new StreamReader(new File(filename));
        RobolinkHeader header = new RobolinkHeader ();
        BFL bfl = new BFL(false);
        while (reader.isGood() && StreamParser.parseHeader(reader, header)){
            if ( ! header.isGood() ){
                System.out.println("Header is bad. : " + header.getMagicHeader());
                break;
            }
            if (header.data_type == YUVPLANE_TYPE){
                if (!StreamParser.parseYUVPlane(reader, bfl))
                    break;
                try {
                    bfl.saveFile(RCUtils.generateUniqueName());
                    System.out.println("RobolinkReceiver: file saved <" +
                                       bfl.fileName + ">");
                }
                catch (FileNotFoundException ex) {
                }
                catch (IOException ex) {
                }

                notifyYUVListeners(bfl);
            }
            else{
                reader.skip(header.data_len);
            }
        }
    }

    public void receiveFromNet(String ip){
        reader = new StreamReader(ip,9000);
        (new Thread(new ConnectToRobot(reader))).start();
    }

    public void disconnect(){
        reader.disconnect();
    }

    public RobolinkReceiver() {
    }

    class ConnectToRobot implements Runnable{
        StreamReader netReader;
        ConnectToRobot(StreamReader reader){
            netReader = reader;
        }
        public void run(){
            boolean res;
            RobolinkHeader header = new RobolinkHeader();
            BFL bfl = new BFL(false);
            while (netReader.isGood()){
                res = StreamParser.parseHeader(netReader, header);
                if (!res) break;
                if (!header.isGood()){
                    System.out.println("Header doesn't match. " + header.getMagicHeader());
                    break;
                }
                if (header.data_type == YUVPLANE_TYPE) {
                    if (!StreamParser.parseYUVPlane(netReader, bfl))
                        break;
                    try {
                        bfl.saveFile(RCUtils.generateUniqueName());
                        System.out.println("RobolinkReceiver: file saved <" +
                                           bfl.fileName + ">");
                    }
                    catch (FileNotFoundException ex) {
                    }
                    catch (IOException ex) {
                    }

                    notifyYUVListeners(bfl);
                }
                else {
                    System.out.println("Skipping " + header.data_len + " bytes.");
                    netReader.skip(header.data_len);
                }
            }

            System.out.println("ConnectToRobot finishes");
            netReader.close();
        }
    }
}
