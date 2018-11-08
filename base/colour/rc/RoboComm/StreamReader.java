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
import java.net.*;

public class StreamReader {
    InputStream  in;
    Socket socket;
    boolean bGood = true;



    public StreamReader(File file) {
        try {
            in = new FileInputStream(file);
        }
        catch (FileNotFoundException ex) {
            bGood = false;
        }
    }

    public StreamReader(String ipAddress, int port) {
        try {
            socket = new Socket(ipAddress, port);
            in = socket.getInputStream();
            System.out.println("Socket connected : " + socket.getRemoteSocketAddress());
        }
        catch (UnknownHostException ex) {
            System.out.println("Socket connection failed. UnknownHostException");
        }
        catch (IOException ex) {
            System.out.println("Socket connection failed. ");
        }
    }

    public int readBytes(byte[] data, int len) {
        if (in == null){
            System.out.println("Socket is not ready yet");
            return -1;
        }
        int total = 0;
        try {
            while (total < len){
                int got = in.read(data, total, len - total);
                total += got;
                if (got == 0) break;
            }
            return total;
        }
        catch (IOException ex) {
            bGood = false;
            return -1;
        }
    }

    public void skip(int len){
        byte [] data = new byte[len];
        int res = readBytes(data,len);
        bGood = res == len;
    }

    public boolean isGood() {
        return bGood;
    }

    public void close(){
        try {
            in.close();
        }
        catch (IOException ex) {
            System.out.println("StreamReader Warning : stream cannot be closed.");
        }
    }

    public void disconnect(){
        try {
            if (socket != null) {
                System.out.print("Closing socket...");
                socket.close();
                System.out.println("closed successfully");
            }
        }
        catch (IOException ex) {
            System.out.println("Error disconnecting socket");
        }
    }
}
