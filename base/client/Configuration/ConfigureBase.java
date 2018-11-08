/*

Copyright 2003 The University of New South Wales (UNSW) and National  
ICT Australia (NICTA).

This file is part of the 2003 team rUNSWift RoboCup entry.  You may  
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
must be included in that publication.

This rUNSWift source is distributed in the hope that it will be useful,  
but WITHOUT ANY WARRANTY; without even the implied warranty of  
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  
General Public License for more details.

You should have received a copy of the GNU General Public License along  
with this source code; if not, write to the Free Software Foundation,  
Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

/*
 * @author UNSW 2003 Robocup (Nicodemus Sutanto)
 *
 * Last modification background information
 * $Id: ConfigureBase.java 1953 2003-08-21 03:51:39Z eileenm $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 *
 * sets configuration file of the robot
 *
**/
 
import java.io.*;
import java.util.*;

public class ConfigureBase {
    public static BufferedReader in = new BufferedReader(new InputStreamReader(System.in));

    // reads a file into a vector
    protected Vector readFile(String fileName) throws IOException {
        try {
            BufferedReader in  = new BufferedReader(new FileReader(fileName));
            Vector         ret = new Vector();
            for(String line = in.readLine(); line != null; line = in.readLine()) {
                //System.out.println(line);
                ret.addElement(line);
            }
            in.close();
            return ret;
        } catch (IOException e) {
            throw new IOException("file: <" + fileName + "> cannot be open");
        }
   }

   // writes a string vector into a file
   protected void writeFile(Vector content, String fileName) throws IOException {
        try {
            BufferedWriter out = new BufferedWriter(new FileWriter(fileName));
            for (int i = 0; i < content.size(); i++) {
                out.write(content.elementAt(i) + "\n");
            }
            out.close();
        } catch (IOException e) {
            throw new IOException("file: <" + fileName + "> cannot be open");
        }
    }

    // gets a line of info from user
    protected String getInfo(String msg, String errMsg, String defaultValue) throws IOException {
        System.out.print(msg);
        String info = in.readLine();

        if (info == null)
            throw new IOException(errMsg);

        if (info.trim().equals(""))
            info = defaultValue;
        return info;
    }

    // replace the stuff in a vector
    protected void replace(Vector vec, String pre, String rep) {
        String line;
        for (int i = 0; i < vec.size(); i++) {
            line = (String) vec.elementAt(i);
            if (line.startsWith(pre)) {
                //System.out.println(vec.elementAt(i));
                //System.out.println("yes");
                vec.setElementAt(rep,i);
                return;
            }
        }
    }

    protected String replace(String line, int num, String rep) {
	String tag = "/" + num + "/";
	int index = line.indexOf(tag);
	//System.out.println(line + " " + index);
	if (index < 0)
	    return line;
	return replace(line.substring(0,index) +
		       rep +
		       line.substring(index + tag.length()), num, rep);
    }

    protected String replace(String line, int num, int rep) {
	return replace(line,num,rep + "");
    }

    protected BufferedWriter writeFile(Vector content, String fileName, String lastLine)
        throws IOException {
	try {
            BufferedWriter out = new BufferedWriter(new FileWriter(fileName));
            for (int i = 0; i < content.size(); i++) {
		String line = (String) content.elementAt(i);
                out.write(line + "\n");
		if (line.startsWith(lastLine))
		    return out;
            }
	    return out;
        } catch (IOException e) {
            throw new IOException("file: <" + fileName + "> cannot be open");
        }
    }

}
