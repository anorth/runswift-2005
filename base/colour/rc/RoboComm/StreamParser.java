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

import RoboShare.*;
import RC.*;

public class StreamParser {
    static final int HEADER_SIZE = 24;
    static final int HEIGHT = RobotDefinition.CPLANE_HEIGHT ;
    static final int WIDTH = RobotDefinition.CPLANE_WIDTH * 6;
    static final int YUVPLANE_SIZE= HEIGHT * WIDTH;
    static byte [] yuvData = new byte[YUVPLANE_SIZE];

    public static boolean parseHeader(StreamReader reader, RobolinkHeader header){
        byte [] data = new byte[HEADER_SIZE];
        int len = reader.readBytes(data, data.length);
        int i = 0;
        header.magicNumber[i] = data[i++];
        header.magicNumber[i] = data[i++];
        header.magicNumber[i] = data[i++];
        header.magicNumber[i] = data[i++];

        if (len == HEADER_SIZE) {

            header.team_num = Utils.byteToIntLITTLE(data[i++],data[i++],data[i++],data[i++]);
            header.player_num = Utils.byteToIntLITTLE(data[i++],data[i++],data[i++],data[i++]);
            header.frame_num = Utils.byteToIntLITTLE(data[i++],data[i++],data[i++],data[i++]);
            header.data_type = Utils.byteToIntLITTLE(data[i++],data[i++],data[i++],data[i++]);
            header.data_len = Utils.byteToIntLITTLE(data[i++],data[i++],data[i++],data[i++]);
        }

        return len == HEADER_SIZE;
    }

    public static boolean parseCPlane(StreamReader reader, BFL bfl){
        return true;
    }

    public static boolean parseYUVPlane(StreamReader reader, BFL bfl){
        int len = reader.readBytes(yuvData, YUVPLANE_SIZE);
        if (len == YUVPLANE_SIZE){
            bfl.readYUVStream(yuvData);
        }
        else{
            System.out.println("YUVPlane is cut short.");
        }
        return len == YUVPLANE_SIZE;
    }

    public StreamParser() {
    }

    public static void main(String [] args){

    }

}
