/*

Copyright 2004 The University of New South Wales (UNSW) and National  
ICT Australia (NICTA).

This file is part of the 2004 team rUNSWift RoboCup entry.  You may  
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

public class icCal
{

    public static byte getCPlane(int y, int u, int v)
    {

if (u <= 74)
{
    if (u <= 60)
    {
        if (v <= 63)
        {
            if (y <= 50)
            {
                if (v <= 60)
                {
                    if (u <= 58)
                    {
                    	return 7;
                    }
                    else
                    {
                        if (y <= 33)
                        {
                        	return 7;
                        }
                        else
                        {
                            if (y > 39)
                            {
                            	return 9;
                            }
                            else
                            {
                                if (u <= 59)
                                {
                                	return 7;
                                }
                                else
                                {
                                    if (v <= 49)
                                    {
                                    	return 7;
                                    }
                                    else
                                    {
                                    	return 9;
                                    }
                                }
                            }
                        }
                    }
                }
                else
                {
                    if (y <= 13)
                    {
                        if (y <= 11)
                        {
                        	return 5;
                        }
                        else
                        {
                            if (v <= 62)
                            {
                            	return 7;
                            }
                            else
                            {
                            	return 5;
                            }
                        }
                    }
                    else
                    {
                        if (u <= 56)
                        {
                            if (y <= 23)
                            {
                                if (y <= 16)
                                {
                                	return 7;
                                }
                                else
                                {
                                    if (v <= 61)
                                    {
                                        if (y <= 20)
                                        {
                                        	return 1;
                                        }
                                        else
                                        {
                                        	return 7;
                                        }
                                    }
                                    else
                                    {
                                        if (y <= 22)
                                        {
                                        	return 1;
                                        }
                                        else
                                        {
                                            if (u <= 54)
                                            {
                                            	return 1;
                                            }
                                            else
                                            {
                                            	return 7;
                                            }
                                        }
                                    }
                                }
                            }
                            else
                            {
                                if (v <= 62)
                                {
                                	return 7;
                                }
                                else
                                {
                                    if (y > 34)
                                    {
                                    	return 1;
                                    }
                                    else
                                    {
                                        if (y > 26)
                                        {
                                        	return 7;
                                        }
                                        else
                                        {
                                            if (u <= 52)
                                            {
                                            	return 1;
                                            }
                                            else
                                            {
                                            	return 7;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                            if (y > 34)
                            {
                            	return 1;
                            }
                            else
                            {
                                if (y > 19)
                                {
                                	return 7;
                                }
                                else
                                {
                                    if (u > 58)
                                    {
                                    	return 7;
                                    }
                                    else
                                    {
                                        if (v <= 61)
                                        {
                                        	return 7;
                                        }
                                        else
                                        {
                                            if (y <= 15)
                                            {
                                            	return 7;
                                            }
                                            else
                                            {
                                                if (v > 62)
                                                {
                                                	return 1;
                                                }
                                                else
                                                {
                                                    if (u <= 57)
                                                    {
                                                    	return 1;
                                                    }
                                                    else
                                                    {
                                                    	return 7;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                if (v <= 45)
                {
                    if (y <= 65)
                    {
                    	return 3;
                    }
                    else
                    {
                        if (v <= 36)
                        {
                        	return 3;
                        }
                        else
                        {
                        	return 9;
                        }
                    }
                }
                else
                {
                    if (y > 67)
                    {
                    	return 9;
                    }
                    else
                    {
                        if (u > 55)
                        {
                        	return 9;
                        }
                        else
                        {
                            if (u <= 50)
                            {
                            	return 1;
                            }
                            else
                            {
                                if (y <= 60)
                                {
                                    if (u <= 54)
                                    {
                                    	return 7;
                                    }
                                    else
                                    {
                                        if (y <= 56)
                                        {
                                        	return 9;
                                        }
                                        else
                                        {
                                        	return 7;
                                        }
                                    }
                                }
                                else
                                {
                                    if (u <= 52)
                                    {
                                    	return 9;
                                    }
                                    else
                                    {
                                        if (v <= 58)
                                        {
                                        	return 9;
                                        }
                                        else
                                        {
                                        	return 7;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        else
        {
            if (u <= 55)
            {
                if (y <= 51)
                {
                    if (v > 67)
                    {
                    	return 1;
                    }
                    else
                    {
                        if (y <= 25)
                        {
                        	return 1;
                        }
                        else
                        {
                            if (u <= 52)
                            {
                                if (v > 65)
                                {
                                	return 1;
                                }
                                else
                                {
                                    if (u <= 51)
                                    {
                                    	return 1;
                                    }
                                    else
                                    {
                                        if (v > 64)
                                        {
                                        	return 1;
                                        }
                                        else
                                        {
                                            if (y <= 37)
                                            {
                                            	return 7;
                                            }
                                            else
                                            {
                                            	return 1;
                                            }
                                        }
                                    }
                                }
                            }
                            else
                            {
                                if (y <= 32)
                                {
                                    if (v <= 64)
                                    {
                                    	return 7;
                                    }
                                    else
                                    {
                                        if (y <= 26)
                                        {
                                        	return 1;
                                        }
                                        else
                                        {
                                        	return 7;
                                        }
                                    }
                                }
                                else
                                {
                                    if (y <= 49)
                                    {
                                    	return 1;
                                    }
                                    else
                                    {
                                    	return 7;
                                    }
                                }
                            }
                        }
                    }
                }
                else
                {
                    if (y <= 59)
                    {
                        if (v <= 71)
                        {
                        	return 1;
                        }
                        else
                        {
                        	return 5;
                        }
                    }
                    else
                    {
                        if (v <= 64)
                        {
                        	return 5;
                        }
                        else
                        {
                        	return 9;
                        }
                    }
                }
            }
            else
            {
                if (y > 57)
                {
                	return 9;
                }
                else
                {
                    if (y <= 14)
                    {
                        if (y <= 13)
                        {
                        	return 5;
                        }
                        else
                        {
                            if (v <= 65)
                            {
                            	return 5;
                            }
                            else
                            {
                                if (u <= 59)
                                {
                                    if (v > 72)
                                    {
                                    	return 5;
                                    }
                                    else
                                    {
                                        if (u <= 57)
                                        {
                                        	return 5;
                                        }
                                        else
                                        {
                                        	return 1;
                                        }
                                    }
                                }
                                else
                                {
                                    if (v <= 68)
                                    {
                                    	return 5;
                                    }
                                    else
                                    {
                                        if (v <= 72)
                                        {
                                        	return 10;
                                        }
                                        else
                                        {
                                        	return 1;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        if (y <= 21)
                        {
                            if (u <= 59)
                            {
                                if (v <= 65)
                                {
                                    if (y <= 15)
                                    {
                                        if (v <= 64)
                                        {
                                        	return 7;
                                        }
                                        else
                                        {
                                        	return 1;
                                        }
                                    }
                                    else
                                    {
                                        if (y <= 18)
                                        {
                                        	return 1;
                                        }
                                        else
                                        {
                                            if (u <= 58)
                                            {
                                            	return 1;
                                            }
                                            else
                                            {
                                            	return 7;
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    if (v <= 74)
                                    {
                                    	return 1;
                                    }
                                    else
                                    {
                                        if (y <= 16)
                                        {
                                        	return 5;
                                        }
                                        else
                                        {
                                            if (u <= 58)
                                            {
                                            	return 1;
                                            }
                                            else
                                            {
                                                if (y <= 17)
                                                {
                                                	return 5;
                                                }
                                                else
                                                {
                                                    if (v <= 76)
                                                    {
                                                    	return 1;
                                                    }
                                                    else
                                                    {
                                                    	return 10;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            else
                            {
                                if (v <= 64)
                                {
                                	return 7;
                                }
                                else
                                {
                                    if (v <= 73)
                                    {
                                        if (y <= 19)
                                        {
                                        	return 1;
                                        }
                                        else
                                        {
                                        	return 10;
                                        }
                                    }
                                    else
                                    {
                                        if (v <= 76)
                                        {
                                        	return 5;
                                        }
                                        else
                                        {
                                            if (y <= 19)
                                            {
                                            	return 10;
                                            }
                                            else
                                            {
                                                if (v <= 79)
                                                {
                                                	return 5;
                                                }
                                                else
                                                {
                                                	return 10;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                            if (y <= 32)
                            {
                                if (v <= 70)
                                {
                                    if (u <= 58)
                                    {
                                        if (y > 25)
                                        {
                                        	return 7;
                                        }
                                        else
                                        {
                                            if (v <= 65)
                                            {
                                                if (y > 22)
                                                {
                                                	return 7;
                                                }
                                                else
                                                {
                                                    if (u <= 56)
                                                    {
                                                    	return 1;
                                                    }
                                                    else
                                                    {
                                                    	return 7;
                                                    }
                                                }
                                            }
                                            else
                                            {
                                                if (y <= 22)
                                                {
                                                	return 1;
                                                }
                                                else
                                                {
                                                    if (u <= 56)
                                                    {
                                                    	return 1;
                                                    }
                                                    else
                                                    {
                                                        if (v <= 67)
                                                        {
                                                        	return 7;
                                                        }
                                                        else
                                                        {
                                                            if (y <= 23)
                                                            {
                                                            	return 1;
                                                            }
                                                            else
                                                            {
                                                            	return 7;
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    else
                                    {
                                        if (y <= 30)
                                        {
                                        	return 7;
                                        }
                                        else
                                        {
                                            if (v <= 68)
                                            {
                                            	return 5;
                                            }
                                            else
                                            {
                                            	return 7;
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    if (u <= 57)
                                    {
                                    	return 1;
                                    }
                                    else
                                    {
                                        if (v <= 72)
                                        {
                                            if (v <= 71)
                                            {
                                                if (y <= 24)
                                                {
                                                	return 1;
                                                }
                                                else
                                                {
                                                	return 5;
                                                }
                                            }
                                            else
                                            {
                                                if (u <= 58)
                                                {
                                                	return 7;
                                                }
                                                else
                                                {
                                                    if (y <= 27)
                                                    {
                                                    	return 1;
                                                    }
                                                    else
                                                    {
                                                    	return 7;
                                                    }
                                                }
                                            }
                                        }
                                        else
                                        {
                                            if (u <= 58)
                                            {
                                            	return 1;
                                            }
                                            else
                                            {
                                                if (u <= 59)
                                                {
                                                    if (y > 28)
                                                    {
                                                    	return 5;
                                                    }
                                                    else
                                                    {
                                                        if (v <= 73)
                                                        {
                                                        	return 1;
                                                        }
                                                        else
                                                        {
                                                            if (v > 79)
                                                            {
                                                            	return 1;
                                                            }
                                                            else
                                                            {
                                                                if (y > 26)
                                                                {
                                                                	return 7;
                                                                }
                                                                else
                                                                {
                                                                    if (y <= 22)
                                                                    {
                                                                    	return 7;
                                                                    }
                                                                    else
                                                                    {
                                                                        if (v <= 74)
                                                                        {
                                                                        	return 10;
                                                                        }
                                                                        else
                                                                        {
                                                                        	return 1;
                                                                        }
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                                else
                                                {
                                                    if (v > 76)
                                                    {
                                                    	return 1;
                                                    }
                                                    else
                                                    {
                                                        if (y > 26)
                                                        {
                                                        	return 1;
                                                        }
                                                        else
                                                        {
                                                            if (y <= 23)
                                                            {
                                                            	return 5;
                                                            }
                                                            else
                                                            {
                                                            	return 10;
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            else
                            {
                                if (u <= 58)
                                {
                                    if (y <= 38)
                                    {
                                        if (y <= 34)
                                        {
                                            if (v <= 72)
                                            {
                                            	return 7;
                                            }
                                            else
                                            {
                                            	return 1;
                                            }
                                        }
                                        else
                                        {
                                            if (v <= 71)
                                            {
                                            	return 1;
                                            }
                                            else
                                            {
                                                if (v <= 72)
                                                {
                                                	return 5;
                                                }
                                                else
                                                {
                                                	return 1;
                                                }
                                            }
                                        }
                                    }
                                    else
                                    {
                                        if (y > 44)
                                        {
                                        	return 5;
                                        }
                                        else
                                        {
                                            if (u <= 57)
                                            {
                                            	return 1;
                                            }
                                            else
                                            {
                                                if (y <= 40)
                                                {
                                                	return 5;
                                                }
                                                else
                                                {
                                                    if (v <= 67)
                                                    {
                                                    	return 1;
                                                    }
                                                    else
                                                    {
                                                    	return 5;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    if (v > 68)
                                    {
                                    	return 5;
                                    }
                                    else
                                    {
                                        if (y <= 41)
                                        {
                                        	return 1;
                                        }
                                        else
                                        {
                                            if (v <= 66)
                                            {
                                            	return 1;
                                            }
                                            else
                                            {
                                            	return 5;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        if (y <= 31)
        {
            if (u <= 67)
            {
                if (v <= 61)
                {
                    if (y <= 21)
                    {
                        if (y <= 13)
                        {
                            if (u <= 63)
                            {
                                if (y <= 10)
                                {
                                    if (y > 8)
                                    {
                                    	return 5;
                                    }
                                    else
                                    {
                                        if (v <= 59)
                                        {
                                        	return 5;
                                        }
                                        else
                                        {
                                        	return 10;
                                        }
                                    }
                                }
                                else
                                {
                                    if (v <= 58)
                                    {
                                        if (y <= 11)
                                        {
                                        	return 5;
                                        }
                                        else
                                        {
                                            if (v > 55)
                                            {
                                            	return 7;
                                            }
                                            else
                                            {
                                                if (y <= 12)
                                                {
                                                	return 5;
                                                }
                                                else
                                                {
                                                    if (v <= 51)
                                                    {
                                                    	return 5;
                                                    }
                                                    else
                                                    {
                                                    	return 7;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    else
                                    {
                                        if (u <= 62)
                                        {
                                        	return 7;
                                        }
                                        else
                                        {
                                            if (y <= 12)
                                            {
                                            	return 7;
                                            }
                                            else
                                            {
                                            	return 10;
                                            }
                                        }
                                    }
                                }
                            }
                            else
                            {
                                if (u <= 66)
                                {
                                    if (v <= 57)
                                    {
                                    	return 5;
                                    }
                                    else
                                    {
                                        if (y <= 11)
                                        {
                                            if (v <= 60)
                                            {
                                            	return 5;
                                            }
                                            else
                                            {
                                                if (y > 10)
                                                {
                                                	return 10;
                                                }
                                                else
                                                {
                                                    if (y <= 8)
                                                    {
                                                    	return 10;
                                                    }
                                                    else
                                                    {
                                                    	return 5;
                                                    }
                                                }
                                            }
                                        }
                                        else
                                        {
                                            if (v > 58)
                                            {
                                            	return 10;
                                            }
                                            else
                                            {
                                                if (y <= 12)
                                                {
                                                	return 5;
                                                }
                                                else
                                                {
                                                	return 10;
                                                }
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    if (v <= 56)
                                    {
                                    	return 5;
                                    }
                                    else
                                    {
                                        if (y > 12)
                                        {
                                        	return 10;
                                        }
                                        else
                                        {
                                            if (y <= 11)
                                            {
                                            	return 6;
                                            }
                                            else
                                            {
                                                if (v <= 59)
                                                {
                                                	return 6;
                                                }
                                                else
                                                {
                                                	return 10;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                            if (u <= 63)
                            {
                                if (v <= 58)
                                {
                                	return 7;
                                }
                                else
                                {
                                    if (u <= 62)
                                    {
                                    	return 7;
                                    }
                                    else
                                    {
                                        if (y <= 16)
                                        {
                                        	return 10;
                                        }
                                        else
                                        {
                                            if (v > 60)
                                            {
                                            	return 10;
                                            }
                                            else
                                            {
                                                if (y <= 20)
                                                {
                                                	return 7;
                                                }
                                                else
                                                {
                                                	return 10;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            else
                            {
                                if (v <= 54)
                                {
                                    if (v <= 51)
                                    {
                                        if (y <= 14)
                                        {
                                        	return 5;
                                        }
                                        else
                                        {
                                        	return 7;
                                        }
                                    }
                                    else
                                    {
                                        if (u <= 66)
                                        {
                                            if (u <= 64)
                                            {
                                            	return 7;
                                            }
                                            else
                                            {
                                                if (v <= 53)
                                                {
                                                    if (y > 18)
                                                    {
                                                    	return 7;
                                                    }
                                                    else
                                                    {
                                                        if (y <= 15)
                                                        {
                                                        	return 7;
                                                        }
                                                        else
                                                        {
                                                            if (v > 52)
                                                            {
                                                            	return 10;
                                                            }
                                                            else
                                                            {
                                                                if (y <= 16)
                                                                {
                                                                	return 10;
                                                                }
                                                                else
                                                                {
                                                                	return 7;
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                                else
                                                {
                                                    if (u > 65)
                                                    {
                                                    	return 10;
                                                    }
                                                    else
                                                    {
                                                        if (y <= 17)
                                                        {
                                                        	return 10;
                                                        }
                                                        else
                                                        {
                                                        	return 7;
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                        else
                                        {
                                            if (v <= 53)
                                            {
                                            	return 5;
                                            }
                                            else
                                            {
                                            	return 7;
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    if (y <= 19)
                                    {
                                    	return 10;
                                    }
                                    else
                                    {
                                        if (v <= 56)
                                        {
                                        	return 7;
                                        }
                                        else
                                        {
                                            if (u <= 65)
                                            {
                                                if (v <= 59)
                                                {
                                                    if (y <= 20)
                                                    {
                                                    	return 10;
                                                    }
                                                    else
                                                    {
                                                        if (u <= 64)
                                                        {
                                                        	return 7;
                                                        }
                                                        else
                                                        {
                                                        	return 8;
                                                        }
                                                    }
                                                }
                                                else
                                                {
                                                    if (y > 20)
                                                    {
                                                    	return 10;
                                                    }
                                                    else
                                                    {
                                                        if (u <= 64)
                                                        {
                                                        	return 8;
                                                        }
                                                        else
                                                        {
                                                        	return 10;
                                                        }
                                                    }
                                                }
                                            }
                                            else
                                            {
                                                if (u <= 66)
                                                {
                                                    if (v <= 58)
                                                    {
                                                    	return 10;
                                                    }
                                                    else
                                                    {
                                                    	return 8;
                                                    }
                                                }
                                                else
                                                {
                                                    if (v <= 59)
                                                    {
                                                    	return 8;
                                                    }
                                                    else
                                                    {
                                                    	return 10;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        if (u <= 63)
                        {
                            if (y <= 28)
                            {
                            	return 7;
                            }
                            else
                            {
                                if (u <= 62)
                                {
                                    if (v > 58)
                                    {
                                    	return 7;
                                    }
                                    else
                                    {
                                        if (y <= 29)
                                        {
                                        	return 7;
                                        }
                                        else
                                        {
                                            if (v <= 53)
                                            {
                                            	return 7;
                                            }
                                            else
                                            {
                                            	return 9;
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    if (v <= 53)
                                    {
                                    	return 7;
                                    }
                                    else
                                    {
                                        if (v > 59)
                                        {
                                        	return 8;
                                        }
                                        else
                                        {
                                            if (v > 55)
                                            {
                                            	return 9;
                                            }
                                            else
                                            {
                                                if (y <= 30)
                                                {
                                                	return 9;
                                                }
                                                else
                                                {
                                                	return 8;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                            if (v <= 50)
                            {
                                if (y <= 27)
                                {
                                	return 7;
                                }
                                else
                                {
                                	return 3;
                                }
                            }
                            else
                            {
                                if (y <= 25)
                                {
                                    if (v <= 54)
                                    {
                                        if (y <= 24)
                                        {
                                        	return 7;
                                        }
                                        else
                                        {
                                            if (u > 65)
                                            {
                                            	return 8;
                                            }
                                            else
                                            {
                                                if (u > 64)
                                                {
                                                	return 7;
                                                }
                                                else
                                                {
                                                    if (v <= 52)
                                                    {
                                                    	return 7;
                                                    }
                                                    else
                                                    {
                                                    	return 8;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    else
                                    {
                                        if (u > 64)
                                        {
                                        	return 8;
                                        }
                                        else
                                        {
                                            if (v <= 56)
                                            {
                                                if (y <= 24)
                                                {
                                                	return 7;
                                                }
                                                else
                                                {
                                                	return 9;
                                                }
                                            }
                                            else
                                            {
                                                if (v <= 57)
                                                {
                                                	return 8;
                                                }
                                                else
                                                {
                                                    if (y <= 24)
                                                    {
                                                    	return 10;
                                                    }
                                                    else
                                                    {
                                                        if (v <= 59)
                                                        {
                                                        	return 7;
                                                        }
                                                        else
                                                        {
                                                        	return 9;
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    if (u <= 64)
                                    {
                                        if (y <= 27)
                                        {
                                            if (v > 59)
                                            {
                                            	return 7;
                                            }
                                            else
                                            {
                                                if (y > 26)
                                                {
                                                	return 7;
                                                }
                                                else
                                                {
                                                    if (v > 55)
                                                    {
                                                    	return 8;
                                                    }
                                                    else
                                                    {
                                                        if (v <= 53)
                                                        {
                                                        	return 7;
                                                        }
                                                        else
                                                        {
                                                        	return 9;
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                        else
                                        {
                                            if (v <= 56)
                                            {
                                            	return 9;
                                            }
                                            else
                                            {
                                                if (v <= 58)
                                                {
                                                	return 8;
                                                }
                                                else
                                                {
                                                	return 9;
                                                }
                                            }
                                        }
                                    }
                                    else
                                    {
                                        if (y <= 28)
                                        {
                                            if (v > 52)
                                            {
                                            	return 8;
                                            }
                                            else
                                            {
                                                if (u <= 65)
                                                {
                                                    if (v <= 51)
                                                    {
                                                    	return 7;
                                                    }
                                                    else
                                                    {
                                                    	return 9;
                                                    }
                                                }
                                                else
                                                {
                                                    if (y <= 26)
                                                    {
                                                    	return 3;
                                                    }
                                                    else
                                                    {
                                                    	return 8;
                                                    }
                                                }
                                            }
                                        }
                                        else
                                        {
                                            if (u > 65)
                                            {
                                            	return 8;
                                            }
                                            else
                                            {
                                                if (v <= 53)
                                                {
                                                	return 8;
                                                }
                                                else
                                                {
                                                    if (v <= 58)
                                                    {
                                                    	return 9;
                                                    }
                                                    else
                                                    {
                                                    	return 8;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                else
                {
                    if (y <= 11)
                    {
                        if (u <= 64)
                        {
                            if (y <= 7)
                            {
                            	return 10;
                            }
                            else
                            {
                            	return 5;
                            }
                        }
                        else
                        {
                            if (u <= 66)
                            {
                                if (y <= 10)
                                {
                                	return 5;
                                }
                                else
                                {
                                	return 10;
                                }
                            }
                            else
                            {
                                if (v <= 65)
                                {
                                	return 6;
                                }
                                else
                                {
                                	return 5;
                                }
                            }
                        }
                    }
                    else
                    {
                        if (y <= 22)
                        {
                            if (v <= 67)
                            {
                                if (u > 62)
                                {
                                	return 10;
                                }
                                else
                                {
                                    if (y <= 14)
                                    {
                                        if (v <= 63)
                                        {
                                            if (u <= 61)
                                            {
                                            	return 7;
                                            }
                                            else
                                            {
                                            	return 10;
                                            }
                                        }
                                        else
                                        {
                                            if (v > 64)
                                            {
                                            	return 5;
                                            }
                                            else
                                            {
                                                if (y <= 13)
                                                {
                                                	return 5;
                                                }
                                                else
                                                {
                                                	return 10;
                                                }
                                            }
                                        }
                                    }
                                    else
                                    {
                                        if (v > 64)
                                        {
                                        	return 10;
                                        }
                                        else
                                        {
                                            if (u > 61)
                                            {
                                            	return 10;
                                            }
                                            else
                                            {
                                                if (v <= 63)
                                                {
                                                	return 7;
                                                }
                                                else
                                                {
                                                    if (y <= 16)
                                                    {
                                                    	return 10;
                                                    }
                                                    else
                                                    {
                                                    	return 7;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            else
                            {
                                if (y <= 16)
                                {
                                    if (u <= 63)
                                    {
                                        if (y <= 14)
                                        {
                                            if (v <= 75)
                                            {
                                            	return 5;
                                            }
                                            else
                                            {
                                            	return 10;
                                            }
                                        }
                                        else
                                        {
                                            if (y > 15)
                                            {
                                            	return 5;
                                            }
                                            else
                                            {
                                                if (v <= 73)
                                                {
                                                	return 10;
                                                }
                                                else
                                                {
                                                	return 5;
                                                }
                                            }
                                        }
                                    }
                                    else
                                    {
                                        if (v <= 68)
                                        {
                                        	return 10;
                                        }
                                        else
                                        {
                                            if (u > 65)
                                            {
                                            	return 5;
                                            }
                                            else
                                            {
                                                if (u > 64)
                                                {
                                                	return 10;
                                                }
                                                else
                                                {
                                                    if (y <= 13)
                                                    {
                                                    	return 5;
                                                    }
                                                    else
                                                    {
                                                        if (v > 73)
                                                        {
                                                        	return 10;
                                                        }
                                                        else
                                                        {
                                                            if (v <= 71)
                                                            {
                                                            	return 10;
                                                            }
                                                            else
                                                            {
                                                            	return 5;
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    if (v <= 73)
                                    {
                                        if (y > 18)
                                        {
                                        	return 10;
                                        }
                                        else
                                        {
                                            if (u <= 65)
                                            {
                                            	return 10;
                                            }
                                            else
                                            {
                                            	return 5;
                                            }
                                        }
                                    }
                                    else
                                    {
                                        if (v > 79)
                                        {
                                        	return 10;
                                        }
                                        else
                                        {
                                            if (y <= 18)
                                            {
                                            	return 5;
                                            }
                                            else
                                            {
                                                if (v > 74)
                                                {
                                                	return 5;
                                                }
                                                else
                                                {
                                                    if (u > 64)
                                                    {
                                                    	return 5;
                                                    }
                                                    else
                                                    {
                                                        if (u <= 61)
                                                        {
                                                        	return 1;
                                                        }
                                                        else
                                                        {
                                                        	return 10;
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                            if (v <= 66)
                            {
                                if (u <= 62)
                                {
                                    if (y > 24)
                                    {
                                    	return 7;
                                    }
                                    else
                                    {
                                        if (v <= 65)
                                        {
                                        	return 7;
                                        }
                                        else
                                        {
                                            if (y <= 23)
                                            {
                                            	return 10;
                                            }
                                            else
                                            {
                                            	return 5;
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    if (u <= 65)
                                    {
                                        if (v <= 64)
                                        {
                                            if (v <= 63)
                                            {
                                                if (u > 63)
                                                {
                                                	return 10;
                                                }
                                                else
                                                {
                                                    if (y <= 25)
                                                    {
                                                    	return 10;
                                                    }
                                                    else
                                                    {
                                                    	return 8;
                                                    }
                                                }
                                            }
                                            else
                                            {
                                                if (u > 64)
                                                {
                                                	return 10;
                                                }
                                                else
                                                {
                                                    if (y <= 24)
                                                    {
                                                        if (y <= 23)
                                                        {
                                                        	return 7;
                                                        }
                                                        else
                                                        {
                                                        	return 8;
                                                        }
                                                    }
                                                    else
                                                    {
                                                        if (y <= 26)
                                                        {
                                                        	return 5;
                                                        }
                                                        else
                                                        {
                                                        	return 7;
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                        else
                                        {
                                            if (u > 64)
                                            {
                                            	return 10;
                                            }
                                            else
                                            {
                                                if (v > 65)
                                                {
                                                	return 7;
                                                }
                                                else
                                                {
                                                    if (y <= 24)
                                                    {
                                                    	return 10;
                                                    }
                                                    else
                                                    {
                                                    	return 5;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    else
                                    {
                                        if (y <= 23)
                                        {
                                            if (v <= 62)
                                            {
                                            	return 5;
                                            }
                                            else
                                            {
                                            	return 10;
                                            }
                                        }
                                        else
                                        {
                                            if (u <= 66)
                                            {
                                                if (y <= 27)
                                                {
                                                	return 8;
                                                }
                                                else
                                                {
                                                	return 5;
                                                }
                                            }
                                            else
                                            {
                                                if (y <= 25)
                                                {
                                                	return 10;
                                                }
                                                else
                                                {
                                                	return 8;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            else
                            {
                                if (y <= 29)
                                {
                                    if (u <= 64)
                                    {
                                        if (v <= 76)
                                        {
                                            if (y <= 28)
                                            {
                                                if (y <= 27)
                                                {
                                                    if (y <= 23)
                                                    {
                                                        if (v <= 71)
                                                        {
                                                        	return 5;
                                                        }
                                                        else
                                                        {
                                                            if (u <= 62)
                                                            {
                                                            	return 5;
                                                            }
                                                            else
                                                            {
                                                            	return 10;
                                                            }
                                                        }
                                                    }
                                                    else
                                                    {
                                                        if (y > 26)
                                                        {
                                                        	return 5;
                                                        }
                                                        else
                                                        {
                                                            if (v > 73)
                                                            {
                                                            	return 5;
                                                            }
                                                            else
                                                            {
                                                                if (v > 67)
                                                                {
                                                                	return 10;
                                                                }
                                                                else
                                                                {
                                                                    if (y > 25)
                                                                    {
                                                                    	return 7;
                                                                    }
                                                                    else
                                                                    {
                                                                        if (u <= 62)
                                                                        {
                                                                        	return 7;
                                                                        }
                                                                        else
                                                                        {
                                                                        	return 5;
                                                                        }
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                                else
                                                {
                                                    if (v > 74)
                                                    {
                                                    	return 5;
                                                    }
                                                    else
                                                    {
                                                        if (v <= 73)
                                                        {
                                                        	return 7;
                                                        }
                                                        else
                                                        {
                                                        	return 10;
                                                        }
                                                    }
                                                }
                                            }
                                            else
                                            {
                                                if (v <= 69)
                                                {
                                                	return 7;
                                                }
                                                else
                                                {
                                                	return 5;
                                                }
                                            }
                                        }
                                        else
                                        {
                                            if (u <= 61)
                                            {
                                            	return 1;
                                            }
                                            else
                                            {
                                            	return 10;
                                            }
                                        }
                                    }
                                    else
                                    {
                                        if (v <= 75)
                                        {
                                        	return 10;
                                        }
                                        else
                                        {
                                        	return 5;
                                        }
                                    }
                                }
                                else
                                {
                                    if (v <= 67)
                                    {
                                    	return 7;
                                    }
                                    else
                                    {
                                    	return 5;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                if (y <= 24)
                {
                    if (y <= 21)
                    {
                        if (u <= 70)
                        {
                            if (y <= 14)
                            {
                                if (v <= 64)
                                {
                                    if (y <= 12)
                                    {
                                    	return 6;
                                    }
                                    else
                                    {
                                        if (u <= 68)
                                        {
                                        	return 10;
                                        }
                                        else
                                        {
                                        	return 6;
                                        }
                                    }
                                }
                                else
                                {
                                    if (u > 69)
                                    {
                                    	return 6;
                                    }
                                    else
                                    {
                                        if (u > 68)
                                        {
                                        	return 5;
                                        }
                                        else
                                        {
                                            if (y <= 12)
                                            {
                                            	return 6;
                                            }
                                            else
                                            {
                                            	return 5;
                                            }
                                        }
                                    }
                                }
                            }
                            else
                            {
                                if (v <= 61)
                                {
                                    if (u <= 68)
                                    {
                                        if (v <= 52)
                                        {
                                        	return 7;
                                        }
                                        else
                                        {
                                        	return 10;
                                        }
                                    }
                                    else
                                    {
                                        if (u > 69)
                                        {
                                        	return 6;
                                        }
                                        else
                                        {
                                            if (v > 56)
                                            {
                                            	return 10;
                                            }
                                            else
                                            {
                                                if (v <= 55)
                                                {
                                                    if (y <= 19)
                                                    {
                                                    	return 5;
                                                    }
                                                    else
                                                    {
                                                    	return 6;
                                                    }
                                                }
                                                else
                                                {
                                                    if (y <= 16)
                                                    {
                                                    	return 5;
                                                    }
                                                    else
                                                    {
                                                    	return 8;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    if (v <= 70)
                                    {
                                    	return 10;
                                    }
                                    else
                                    {
                                    	return 5;
                                    }
                                }
                            }
                        }
                        else
                        {
                            if (y <= 16)
                            {
                                if (v <= 64)
                                {
                                	return 6;
                                }
                                else
                                {
                                	return 5;
                                }
                            }
                            else
                            {
                                if (v <= 62)
                                {
                                	return 6;
                                }
                                else
                                {
                                    if (u <= 72)
                                    {
                                    	return 10;
                                    }
                                    else
                                    {
                                    	return 6;
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        if (u <= 71)
                        {
                            if (v <= 53)
                            {
                                if (u <= 69)
                                {
                                	return 7;
                                }
                                else
                                {
                                    if (u <= 70)
                                    {
                                    	return 6;
                                    }
                                    else
                                    {
                                    	return 3;
                                    }
                                }
                            }
                            else
                            {
                                if (v > 60)
                                {
                                	return 10;
                                }
                                else
                                {
                                    if (u <= 70)
                                    {
                                        if (y > 22)
                                        {
                                        	return 8;
                                        }
                                        else
                                        {
                                            if (u <= 69)
                                            {
                                                if (u <= 68)
                                                {
                                                    if (v <= 56)
                                                    {
                                                    	return 8;
                                                    }
                                                    else
                                                    {
                                                    	return 10;
                                                    }
                                                }
                                                else
                                                {
                                                    if (v <= 58)
                                                    {
                                                    	return 10;
                                                    }
                                                    else
                                                    {
                                                    	return 6;
                                                    }
                                                }
                                            }
                                            else
                                            {
                                                if (v <= 55)
                                                {
                                                	return 10;
                                                }
                                                else
                                                {
                                                	return 8;
                                                }
                                            }
                                        }
                                    }
                                    else
                                    {
                                        if (y <= 23)
                                        {
                                            if (v > 58)
                                            {
                                            	return 10;
                                            }
                                            else
                                            {
                                                if (v <= 57)
                                                {
                                                	return 6;
                                                }
                                                else
                                                {
                                                	return 9;
                                                }
                                            }
                                        }
                                        else
                                        {
                                            if (v <= 55)
                                            {
                                            	return 3;
                                            }
                                            else
                                            {
                                            	return 8;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                            if (v <= 59)
                            {
                            	return 6;
                            }
                            else
                            {
                                if (u > 73)
                                {
                                	return 6;
                                }
                                else
                                {
                                    if (u > 72)
                                    {
                                    	return 10;
                                    }
                                    else
                                    {
                                        if (y <= 22)
                                        {
                                        	return 6;
                                        }
                                        else
                                        {
                                            if (y <= 23)
                                            {
                                            	return 7;
                                            }
                                            else
                                            {
                                            	return 8;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                else
                {
                    if (v <= 53)
                    {
                        if (v <= 51)
                        {
                        	return 3;
                        }
                        else
                        {
                            if (u <= 69)
                            {
                                if (v <= 52)
                                {
                                    if (y <= 27)
                                    {
                                    	return 8;
                                    }
                                    else
                                    {
                                    	return 3;
                                    }
                                }
                                else
                                {
                                    if (u > 68)
                                    {
                                    	return 8;
                                    }
                                    else
                                    {
                                        if (y <= 26)
                                        {
                                        	return 10;
                                        }
                                        else
                                        {
                                        	return 8;
                                        }
                                    }
                                }
                            }
                            else
                            {
                                if (y <= 30)
                                {
                                	return 3;
                                }
                                else
                                {
                                    if (v <= 52)
                                    {
                                    	return 3;
                                    }
                                    else
                                    {
                                    	return 8;
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        if (v <= 60)
                        {
                            if (v <= 58)
                            {
                                if (y <= 26)
                                {
                                    if (u <= 70)
                                    {
                                    	return 8;
                                    }
                                    else
                                    {
                                        if (v <= 55)
                                        {
                                        	return 3;
                                        }
                                        else
                                        {
                                            if (y > 25)
                                            {
                                            	return 8;
                                            }
                                            else
                                            {
                                                if (u <= 72)
                                                {
                                                	return 8;
                                                }
                                                else
                                                {
                                                	return 6;
                                                }
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    if (y <= 27)
                                    {
                                        if (v <= 54)
                                        {
                                        	return 3;
                                        }
                                        else
                                        {
                                            if (v <= 57)
                                            {
                                            	return 8;
                                            }
                                            else
                                            {
                                                if (u <= 69)
                                                {
                                                	return 8;
                                                }
                                                else
                                                {
                                                	return 3;
                                                }
                                            }
                                        }
                                    }
                                    else
                                    {
                                        if (v <= 56)
                                        {
                                        	return 8;
                                        }
                                        else
                                        {
                                            if (u <= 68)
                                            {
                                            	return 9;
                                            }
                                            else
                                            {
                                            	return 8;
                                            }
                                        }
                                    }
                                }
                            }
                            else
                            {
                                if (u <= 73)
                                {
                                	return 8;
                                }
                                else
                                {
                                	return 6;
                                }
                            }
                        }
                        else
                        {
                            if (v > 63)
                            {
                            	return 10;
                            }
                            else
                            {
                                if (y <= 27)
                                {
                                	return 10;
                                }
                                else
                                {
                                    if (u <= 70)
                                    {
                                    	return 8;
                                    }
                                    else
                                    {
                                        if (u <= 71)
                                        {
                                        	return 3;
                                        }
                                        else
                                        {
                                        	return 6;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        else
        {
            if (v <= 44)
            {
                if (y <= 70)
                {
                    if (v <= 39)
                    {
                    	return 3;
                    }
                    else
                    {
                        if (y <= 52)
                        {
                        	return 3;
                        }
                        else
                        {
                            if (u <= 72)
                            {
                                if (v <= 41)
                                {
                                    if (v <= 40)
                                    {
                                    	return 3;
                                    }
                                    else
                                    {
                                        if (u <= 65)
                                        {
                                        	return 3;
                                        }
                                        else
                                        {
                                            if (y <= 62)
                                            {
                                                if (u > 71)
                                                {
                                                	return 3;
                                                }
                                                else
                                                {
                                                    if (y <= 53)
                                                    {
                                                    	return 3;
                                                    }
                                                    else
                                                    {
                                                    	return 9;
                                                    }
                                                }
                                            }
                                            else
                                            {
                                                if (y <= 65)
                                                {
                                                	return 3;
                                                }
                                                else
                                                {
                                                    if (y <= 66)
                                                    {
                                                    	return 9;
                                                    }
                                                    else
                                                    {
                                                    	return 3;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    if (v > 43)
                                    {
                                    	return 9;
                                    }
                                    else
                                    {
                                        if (u <= 71)
                                        {
                                            if (u > 65)
                                            {
                                            	return 9;
                                            }
                                            else
                                            {
                                                if (v <= 42)
                                                {
                                                	return 3;
                                                }
                                                else
                                                {
                                                	return 9;
                                                }
                                            }
                                        }
                                        else
                                        {
                                            if (y <= 56)
                                            {
                                            	return 9;
                                            }
                                            else
                                            {
                                            	return 3;
                                            }
                                        }
                                    }
                                }
                            }
                            else
                            {
                                if (y <= 65)
                                {
                                	return 3;
                                }
                                else
                                {
                                    if (u > 73)
                                    {
                                    	return 3;
                                    }
                                    else
                                    {
                                        if (v <= 40)
                                        {
                                        	return 3;
                                        }
                                        else
                                        {
                                        	return 9;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                else
                {
                    if (v > 39)
                    {
                    	return 9;
                    }
                    else
                    {
                        if (v <= 35)
                        {
                        	return 3;
                        }
                        else
                        {
                        	return 9;
                        }
                    }
                }
            }
            else
            {
                if (y > 51)
                {
                	return 9;
                }
                else
                {
                    if (u <= 69)
                    {
                        if (v <= 48)
                        {
                            if (y <= 41)
                            {
                                if (v <= 47)
                                {
                                    if (u > 62)
                                    {
                                    	return 3;
                                    }
                                    else
                                    {
                                        if (y <= 36)
                                        {
                                        	return 7;
                                        }
                                        else
                                        {
                                        	return 3;
                                        }
                                    }
                                }
                                else
                                {
                                    if (y <= 35)
                                    {
                                        if (u > 67)
                                        {
                                        	return 3;
                                        }
                                        else
                                        {
                                            if (u > 65)
                                            {
                                            	return 8;
                                            }
                                            else
                                            {
                                                if (u <= 64)
                                                {
                                                	return 3;
                                                }
                                                else
                                                {
                                                	return 9;
                                                }
                                            }
                                        }
                                    }
                                    else
                                    {
                                        if (u <= 66)
                                        {
                                        	return 9;
                                        }
                                        else
                                        {
                                            if (y > 39)
                                            {
                                            	return 9;
                                            }
                                            else
                                            {
                                                if (y > 38)
                                                {
                                                	return 8;
                                                }
                                                else
                                                {
                                                    if (y <= 37)
                                                    {
                                                    	return 8;
                                                    }
                                                    else
                                                    {
                                                    	return 3;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            else
                            {
                                if (u <= 66)
                                {
                                	return 9;
                                }
                                else
                                {
                                    if (v <= 46)
                                    {
                                        if (y <= 46)
                                        {
                                        	return 3;
                                        }
                                        else
                                        {
                                            if (v > 45)
                                            {
                                            	return 9;
                                            }
                                            else
                                            {
                                                if (y <= 48)
                                                {
                                                	return 3;
                                                }
                                                else
                                                {
                                                    if (y <= 49)
                                                    {
                                                    	return 8;
                                                    }
                                                    else
                                                    {
                                                    	return 9;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    else
                                    {
                                        if (v <= 47)
                                        {
                                            if (y <= 46)
                                            {
                                            	return 8;
                                            }
                                            else
                                            {
                                            	return 9;
                                            }
                                        }
                                        else
                                        {
                                            if (u <= 68)
                                            {
                                            	return 8;
                                            }
                                            else
                                            {
                                                if (y <= 46)
                                                {
                                                	return 9;
                                                }
                                                else
                                                {
                                                	return 8;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                            if (v <= 63)
                            {
                                if (y <= 40)
                                {
                                    if (u <= 66)
                                    {
                                        if (u <= 62)
                                        {
                                            if (v > 61)
                                            {
                                            	return 5;
                                            }
                                            else
                                            {
                                                if (y <= 35)
                                                {
                                                    if (v <= 50)
                                                    {
                                                    	return 7;
                                                    }
                                                    else
                                                    {
                                                        if (u <= 61)
                                                        {
                                                        	return 9;
                                                        }
                                                        else
                                                        {
                                                            if (y <= 34)
                                                            {
                                                            	return 9;
                                                            }
                                                            else
                                                            {
                                                                if (v <= 53)
                                                                {
                                                                	return 9;
                                                                }
                                                                else
                                                                {
                                                                	return 8;
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                                else
                                                {
                                                    if (v > 50)
                                                    {
                                                    	return 9;
                                                    }
                                                    else
                                                    {
                                                        if (v <= 49)
                                                        {
                                                        	return 9;
                                                        }
                                                        else
                                                        {
                                                        	return 8;
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                        else
                                        {
                                            if (v <= 60)
                                            {
                                                if (y <= 32)
                                                {
                                                    if (u <= 64)
                                                    {
                                                    	return 9;
                                                    }
                                                    else
                                                    {
                                                    	return 8;
                                                    }
                                                }
                                                else
                                                {
                                                    if (v > 49)
                                                    {
                                                    	return 9;
                                                    }
                                                    else
                                                    {
                                                        if (u <= 63)
                                                        {
                                                        	return 8;
                                                        }
                                                        else
                                                        {
                                                            if (u <= 64)
                                                            {
                                                            	return 9;
                                                            }
                                                            else
                                                            {
                                                                if (u <= 65)
                                                                {
                                                                	return 8;
                                                                }
                                                                else
                                                                {
                                                                	return 9;
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                            else
                                            {
                                                if (y <= 37)
                                                {
                                                	return 8;
                                                }
                                                else
                                                {
                                                	return 9;
                                                }
                                            }
                                        }
                                    }
                                    else
                                    {
                                        if (v <= 50)
                                        {
                                            if (y <= 33)
                                            {
                                            	return 3;
                                            }
                                            else
                                            {
                                                if (y <= 37)
                                                {
                                                	return 8;
                                                }
                                                else
                                                {
                                                    if (u <= 67)
                                                    {
                                                    	return 9;
                                                    }
                                                    else
                                                    {
                                                        if (u <= 68)
                                                        {
                                                            if (v <= 49)
                                                            {
                                                            	return 8;
                                                            }
                                                            else
                                                            {
                                                            	return 9;
                                                            }
                                                        }
                                                        else
                                                        {
                                                            if (v <= 49)
                                                            {
                                                            	return 9;
                                                            }
                                                            else
                                                            {
                                                            	return 8;
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                        else
                                        {
                                            if (v <= 58)
                                            {
                                                if (u <= 67)
                                                {
                                                	return 9;
                                                }
                                                else
                                                {
                                                    if (v <= 56)
                                                    {
                                                    	return 8;
                                                    }
                                                    else
                                                    {
                                                        if (y > 38)
                                                        {
                                                        	return 9;
                                                        }
                                                        else
                                                        {
                                                            if (v <= 57)
                                                            {
                                                                if (y <= 34)
                                                                {
                                                                	return 8;
                                                                }
                                                                else
                                                                {
                                                                	return 9;
                                                                }
                                                            }
                                                            else
                                                            {
                                                                if (y <= 33)
                                                                {
                                                                	return 9;
                                                                }
                                                                else
                                                                {
                                                                    if (u > 68)
                                                                    {
                                                                    	return 8;
                                                                    }
                                                                    else
                                                                    {
                                                                        if (y <= 36)
                                                                        {
                                                                        	return 8;
                                                                        }
                                                                        else
                                                                        {
                                                                        	return 9;
                                                                        }
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                            else
                                            {
                                                if (y <= 37)
                                                {
                                                	return 8;
                                                }
                                                else
                                                {
                                                	return 9;
                                                }
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    if (u <= 66)
                                    {
                                    	return 9;
                                    }
                                    else
                                    {
                                        if (v > 54)
                                        {
                                        	return 9;
                                        }
                                        else
                                        {
                                            if (u <= 67)
                                            {
                                            	return 9;
                                            }
                                            else
                                            {
                                                if (y <= 44)
                                                {
                                                	return 8;
                                                }
                                                else
                                                {
                                                	return 9;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            else
                            {
                                if (u <= 63)
                                {
                                	return 5;
                                }
                                else
                                {
                                    if (y <= 34)
                                    {
                                    	return 5;
                                    }
                                    else
                                    {
                                        if (v <= 65)
                                        {
                                        	return 9;
                                        }
                                        else
                                        {
                                            if (y <= 40)
                                            {
                                            	return 5;
                                            }
                                            else
                                            {
                                                if (v <= 66)
                                                {
                                                	return 9;
                                                }
                                                else
                                                {
                                                	return 5;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        if (v <= 49)
                        {
                            if (y <= 41)
                            {
                            	return 3;
                            }
                            else
                            {
                                if (u > 72)
                                {
                                	return 3;
                                }
                                else
                                {
                                    if (v <= 46)
                                    {
                                    	return 3;
                                    }
                                    else
                                    {
                                        if (v <= 47)
                                        {
                                            if (u <= 70)
                                            {
                                            	return 8;
                                            }
                                            else
                                            {
                                            	return 3;
                                            }
                                        }
                                        else
                                        {
                                            if (u <= 71)
                                            {
                                                if (y <= 43)
                                                {
                                                	return 8;
                                                }
                                                else
                                                {
                                                	return 9;
                                                }
                                            }
                                            else
                                            {
                                                if (v <= 48)
                                                {
                                                	return 3;
                                                }
                                                else
                                                {
                                                    if (y <= 43)
                                                    {
                                                    	return 3;
                                                    }
                                                    else
                                                    {
                                                    	return 8;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                            if (u <= 72)
                            {
                                if (y <= 41)
                                {
                                    if (v <= 51)
                                    {
                                        if (y <= 33)
                                        {
                                        	return 3;
                                        }
                                        else
                                        {
                                            if (v <= 50)
                                            {
                                                if (u <= 70)
                                                {
                                                	return 8;
                                                }
                                                else
                                                {
                                                	return 3;
                                                }
                                            }
                                            else
                                            {
                                                if (y <= 37)
                                                {
                                                	return 8;
                                                }
                                                else
                                                {
                                                    if (y <= 39)
                                                    {
                                                    	return 3;
                                                    }
                                                    else
                                                    {
                                                    	return 8;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    else
                                    {
                                        if (v <= 56)
                                        {
                                        	return 8;
                                        }
                                        else
                                        {
                                            if (v <= 63)
                                            {
                                                if (y <= 34)
                                                {
                                                    if (v > 57)
                                                    {
                                                    	return 8;
                                                    }
                                                    else
                                                    {
                                                        if (y <= 32)
                                                        {
                                                        	return 8;
                                                        }
                                                        else
                                                        {
                                                        	return 3;
                                                        }
                                                    }
                                                }
                                                else
                                                {
                                                    if (u <= 71)
                                                    {
                                                        if (v > 61)
                                                        {
                                                        	return 9;
                                                        }
                                                        else
                                                        {
                                                            if (y <= 35)
                                                            {
                                                            	return 9;
                                                            }
                                                            else
                                                            {
                                                            	return 8;
                                                            }
                                                        }
                                                    }
                                                    else
                                                    {
                                                        if (v > 60)
                                                        {
                                                        	return 3;
                                                        }
                                                        else
                                                        {
                                                            if (y <= 35)
                                                            {
                                                            	return 8;
                                                            }
                                                            else
                                                            {
                                                                if (y <= 37)
                                                                {
                                                                	return 3;
                                                                }
                                                                else
                                                                {
                                                                	return 8;
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                            else
                                            {
                                                if (y <= 39)
                                                {
                                                	return 5;
                                                }
                                                else
                                                {
                                                	return 3;
                                                }
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    if (v > 63)
                                    {
                                    	return 4;
                                    }
                                    else
                                    {
                                        if (y <= 49)
                                        {
                                            if (u <= 71)
                                            {
                                                if (v > 57)
                                                {
                                                	return 9;
                                                }
                                                else
                                                {
                                                    if (v > 50)
                                                    {
                                                    	return 8;
                                                    }
                                                    else
                                                    {
                                                        if (u > 70)
                                                        {
                                                        	return 3;
                                                        }
                                                        else
                                                        {
                                                            if (y <= 45)
                                                            {
                                                            	return 8;
                                                            }
                                                            else
                                                            {
                                                            	return 9;
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                            else
                                            {
                                                if (y > 43)
                                                {
                                                	return 8;
                                                }
                                                else
                                                {
                                                    if (y <= 42)
                                                    {
                                                        if (v <= 53)
                                                        {
                                                        	return 3;
                                                        }
                                                        else
                                                        {
                                                        	return 8;
                                                        }
                                                    }
                                                    else
                                                    {
                                                        if (v <= 56)
                                                        {
                                                        	return 8;
                                                        }
                                                        else
                                                        {
                                                        	return 3;
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                        else
                                        {
                                            if (u > 70)
                                            {
                                            	return 9;
                                            }
                                            else
                                            {
                                                if (v <= 53)
                                                {
                                                	return 8;
                                                }
                                                else
                                                {
                                                	return 9;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            else
                            {
                                if (v <= 58)
                                {
                                    if (v <= 52)
                                    {
                                        if (y <= 42)
                                        {
                                        	return 3;
                                        }
                                        else
                                        {
                                            if (y <= 44)
                                            {
                                                if (v <= 50)
                                                {
                                                	return 8;
                                                }
                                                else
                                                {
                                                	return 3;
                                                }
                                            }
                                            else
                                            {
                                                if (u > 73)
                                                {
                                                	return 3;
                                                }
                                                else
                                                {
                                                    if (y <= 47)
                                                    {
                                                    	return 9;
                                                    }
                                                    else
                                                    {
                                                    	return 3;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    else
                                    {
                                        if (y <= 35)
                                        {
                                        	return 8;
                                        }
                                        else
                                        {
                                            if (u > 73)
                                            {
                                            	return 3;
                                            }
                                            else
                                            {
                                                if (v <= 55)
                                                {
                                                    if (v <= 54)
                                                    {
                                                        if (v <= 53)
                                                        {
                                                        	return 3;
                                                        }
                                                        else
                                                        {
                                                            if (y > 45)
                                                            {
                                                            	return 8;
                                                            }
                                                            else
                                                            {
                                                                if (y <= 39)
                                                                {
                                                                	return 8;
                                                                }
                                                                else
                                                                {
                                                                	return 3;
                                                                }
                                                            }
                                                        }
                                                    }
                                                    else
                                                    {
                                                        if (y <= 39)
                                                        {
                                                        	return 3;
                                                        }
                                                        else
                                                        {
                                                        	return 8;
                                                        }
                                                    }
                                                }
                                                else
                                                {
                                                    if (v <= 56)
                                                    {
                                                    	return 8;
                                                    }
                                                    else
                                                    {
                                                        if (y <= 43)
                                                        {
                                                        	return 8;
                                                        }
                                                        else
                                                        {
                                                        	return 9;
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    if (v <= 60)
                                    {
                                    	return 6;
                                    }
                                    else
                                    {
                                    	return 4;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
else
{
    if (u <= 84)
    {
        if (y <= 29)
        {
            if (y <= 22)
            {
                if (y <= 20)
                {
                	return 6;
                }
                else
                {
                    if (v <= 52)
                    {
                    	return 6;
                    }
                    else
                    {
                        if (u > 82)
                        {
                        	return 6;
                        }
                        else
                        {
                            if (v > 57)
                            {
                            	return 6;
                            }
                            else
                            {
                                if (y <= 21)
                                {
                                    if (u > 80)
                                    {
                                    	return 6;
                                    }
                                    else
                                    {
                                        if (v > 56)
                                        {
                                        	return 0;
                                        }
                                        else
                                        {
                                            if (u > 79)
                                            {
                                            	return 0;
                                            }
                                            else
                                            {
                                                if (u <= 78)
                                                {
                                                	return 6;
                                                }
                                                else
                                                {
                                                    if (v <= 54)
                                                    {
                                                    	return 6;
                                                    }
                                                    else
                                                    {
                                                    	return 0;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    if (u > 78)
                                    {
                                    	return 0;
                                    }
                                    else
                                    {
                                        if (v <= 55)
                                        {
                                        	return 6;
                                        }
                                        else
                                        {
                                            if (u <= 76)
                                            {
                                            	return 6;
                                            }
                                            else
                                            {
                                            	return 0;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                if (u <= 78)
                {
                    if (y <= 25)
                    {
                        if (u <= 77)
                        {
                        	return 6;
                        }
                        else
                        {
                            if (y > 23)
                            {
                            	return 6;
                            }
                            else
                            {
                                if (v <= 53)
                                {
                                	return 6;
                                }
                                else
                                {
                                    if (v <= 58)
                                    {
                                    	return 0;
                                    }
                                    else
                                    {
                                    	return 6;
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        if (v <= 55)
                        {
                            if (y <= 26)
                            {
                                if (u <= 75)
                                {
                                	return 3;
                                }
                                else
                                {
                                    if (u > 77)
                                    {
                                    	return 6;
                                    }
                                    else
                                    {
                                        if (v <= 52)
                                        {
                                        	return 3;
                                        }
                                        else
                                        {
                                        	return 6;
                                        }
                                    }
                                }
                            }
                            else
                            {
                                if (v <= 51)
                                {
                                	return 3;
                                }
                                else
                                {
                                    if (y <= 28)
                                    {
                                        if (v <= 54)
                                        {
                                        	return 3;
                                        }
                                        else
                                        {
                                            if (u <= 76)
                                            {
                                            	return 8;
                                            }
                                            else
                                            {
                                            	return 0;
                                            }
                                        }
                                    }
                                    else
                                    {
                                        if (u > 75)
                                        {
                                        	return 8;
                                        }
                                        else
                                        {
                                            if (v <= 53)
                                            {
                                            	return 3;
                                            }
                                            else
                                            {
                                            	return 8;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                            if (y <= 28)
                            {
                            	return 6;
                            }
                            else
                            {
                            	return 8;
                            }
                        }
                    }
                }
                else
                {
                    if (v > 58)
                    {
                    	return 6;
                    }
                    else
                    {
                        if (v <= 48)
                        {
                            if (y <= 27)
                            {
                            	return 6;
                            }
                            else
                            {
                                if (u <= 80)
                                {
                                	return 3;
                                }
                                else
                                {
                                    if (v <= 47)
                                    {
                                    	return 6;
                                    }
                                    else
                                    {
                                    	return 0;
                                    }
                                }
                            }
                        }
                        else
                        {
                            if (u <= 80)
                            {
                                if (y > 28)
                                {
                                	return 6;
                                }
                                else
                                {
                                    if (u <= 79)
                                    {
                                        if (v <= 52)
                                        {
                                        	return 6;
                                        }
                                        else
                                        {
                                            if (v <= 55)
                                            {
                                            	return 0;
                                            }
                                            else
                                            {
                                                if (y <= 23)
                                                {
                                                	return 0;
                                                }
                                                else
                                                {
                                                	return 6;
                                                }
                                            }
                                        }
                                    }
                                    else
                                    {
                                        if (v <= 50)
                                        {
                                        	return 6;
                                        }
                                        else
                                        {
                                            if (v <= 56)
                                            {
                                            	return 0;
                                            }
                                            else
                                            {
                                                if (y <= 24)
                                                {
                                                	return 0;
                                                }
                                                else
                                                {
                                                	return 6;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            else
                            {
                                if (v <= 50)
                                {
                                    if (y > 26)
                                    {
                                    	return 0;
                                    }
                                    else
                                    {
                                        if (v <= 49)
                                        {
                                        	return 6;
                                        }
                                        else
                                        {
                                            if (y <= 24)
                                            {
                                            	return 6;
                                            }
                                            else
                                            {
                                                if (u <= 82)
                                                {
                                                	return 6;
                                                }
                                                else
                                                {
                                                	return 0;
                                                }
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    if (v <= 56)
                                    {
                                        if (y > 23)
                                        {
                                        	return 0;
                                        }
                                        else
                                        {
                                            if (v <= 51)
                                            {
                                            	return 6;
                                            }
                                            else
                                            {
                                                if (u <= 83)
                                                {
                                                	return 0;
                                                }
                                                else
                                                {
                                                    if (v <= 54)
                                                    {
                                                    	return 0;
                                                    }
                                                    else
                                                    {
                                                    	return 6;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    else
                                    {
                                        if (y <= 26)
                                        {
                                            if (u <= 83)
                                            {
                                            	return 0;
                                            }
                                            else
                                            {
                                                if (y <= 24)
                                                {
                                                	return 6;
                                                }
                                                else
                                                {
                                                	return 0;
                                                }
                                            }
                                        }
                                        else
                                        {
                                            if (u <= 82)
                                            {
                                            	return 6;
                                            }
                                            else
                                            {
                                            	return 0;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        else
        {
            if (v <= 50)
            {
                if (y <= 72)
                {
                    if (v <= 45)
                    {
                        if (u <= 82)
                        {
                        	return 3;
                        }
                        else
                        {
                            if (v <= 43)
                            {
                            	return 3;
                            }
                            else
                            {
                                if (y <= 40)
                                {
                                	return 0;
                                }
                                else
                                {
                                	return 3;
                                }
                            }
                        }
                    }
                    else
                    {
                        if (u <= 81)
                        {
                            if (y <= 57)
                            {
                            	return 3;
                            }
                            else
                            {
                                if (u > 76)
                                {
                                	return 3;
                                }
                                else
                                {
                                    if (v <= 47)
                                    {
                                        if (u > 75)
                                        {
                                        	return 3;
                                        }
                                        else
                                        {
                                            if (y <= 63)
                                            {
                                            	return 3;
                                            }
                                            else
                                            {
                                                if (y <= 67)
                                                {
                                                	return 9;
                                                }
                                                else
                                                {
                                                	return 3;
                                                }
                                            }
                                        }
                                    }
                                    else
                                    {
                                        if (v > 49)
                                        {
                                        	return 9;
                                        }
                                        else
                                        {
                                            if (y > 66)
                                            {
                                            	return 9;
                                            }
                                            else
                                            {
                                                if (v <= 48)
                                                {
                                                	return 3;
                                                }
                                                else
                                                {
                                                    if (u <= 75)
                                                    {
                                                    	return 9;
                                                    }
                                                    else
                                                    {
                                                    	return 3;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                            if (y > 45)
                            {
                            	return 3;
                            }
                            else
                            {
                                if (y <= 42)
                                {
                                	return 0;
                                }
                                else
                                {
                                    if (v > 48)
                                    {
                                    	return 4;
                                    }
                                    else
                                    {
                                        if (y <= 44)
                                        {
                                        	return 3;
                                        }
                                        else
                                        {
                                        	return 0;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                else
                {
                    if (u <= 82)
                    {
                    	return 9;
                    }
                    else
                    {
                    	return 0;
                    }
                }
            }
            else
            {
                if (u <= 78)
                {
                    if (y <= 56)
                    {
                        if (v <= 53)
                        {
                            if (y <= 40)
                            {
                                if (v <= 52)
                                {
                                	return 3;
                                }
                                else
                                {
                                    if (y > 37)
                                    {
                                    	return 3;
                                    }
                                    else
                                    {
                                        if (u <= 75)
                                        {
                                        	return 8;
                                        }
                                        else
                                        {
                                        	return 3;
                                        }
                                    }
                                }
                            }
                            else
                            {
                                if (u > 75)
                                {
                                	return 3;
                                }
                                else
                                {
                                    if (v <= 52)
                                    {
                                        if (y <= 49)
                                        {
                                        	return 3;
                                        }
                                        else
                                        {
                                        	return 9;
                                        }
                                    }
                                    else
                                    {
                                        if (y <= 48)
                                        {
                                        	return 6;
                                        }
                                        else
                                        {
                                        	return 3;
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                            if (y <= 34)
                            {
                                if (u <= 77)
                                {
                                	return 8;
                                }
                                else
                                {
                                    if (y <= 31)
                                    {
                                    	return 6;
                                    }
                                    else
                                    {
                                    	return 4;
                                    }
                                }
                            }
                            else
                            {
                                if (v <= 55)
                                {
                                	return 3;
                                }
                                else
                                {
                                    if (v <= 63)
                                    {
                                        if (u <= 75)
                                        {
                                            if (y <= 45)
                                            {
                                            	return 3;
                                            }
                                            else
                                            {
                                                if (y <= 53)
                                                {
                                                	return 6;
                                                }
                                                else
                                                {
                                                	return 9;
                                                }
                                            }
                                        }
                                        else
                                        {
                                            if (u <= 76)
                                            {
                                                if (y <= 37)
                                                {
                                                	return 6;
                                                }
                                                else
                                                {
                                                	return 3;
                                                }
                                            }
                                            else
                                            {
                                                if (u <= 77)
                                                {
                                                    if (v > 61)
                                                    {
                                                    	return 6;
                                                    }
                                                    else
                                                    {
                                                        if (v <= 56)
                                                        {
                                                        	return 6;
                                                        }
                                                        else
                                                        {
                                                            if (y <= 47)
                                                            {
                                                            	return 4;
                                                            }
                                                            else
                                                            {
                                                            	return 3;
                                                            }
                                                        }
                                                    }
                                                }
                                                else
                                                {
                                                    if (y > 49)
                                                    {
                                                    	return 4;
                                                    }
                                                    else
                                                    {
                                                        if (v <= 57)
                                                        {
                                                        	return 6;
                                                        }
                                                        else
                                                        {
                                                        	return 3;
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    else
                                    {
                                        if (v <= 66)
                                        {
                                        	return 4;
                                        }
                                        else
                                        {
                                        	return 3;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        if (u <= 77)
                        {
                            if (u <= 76)
                            {
                            	return 9;
                            }
                            else
                            {
                                if (v <= 51)
                                {
                                	return 3;
                                }
                                else
                                {
                                	return 9;
                                }
                            }
                        }
                        else
                        {
                            if (v <= 54)
                            {
                            	return 3;
                            }
                            else
                            {
                                if (y <= 60)
                                {
                                	return 4;
                                }
                                else
                                {
                                	return 9;
                                }
                            }
                        }
                    }
                }
                else
                {
                    if (v <= 53)
                    {
                        if (y <= 36)
                        {
                            if (u <= 80)
                            {
                            	return 6;
                            }
                            else
                            {
                            	return 0;
                            }
                        }
                        else
                        {
                            if (u <= 82)
                            {
                            	return 3;
                            }
                            else
                            {
                                if (y > 52)
                                {
                                	return 3;
                                }
                                else
                                {
                                    if (v > 51)
                                    {
                                    	return 4;
                                    }
                                    else
                                    {
                                        if (u <= 83)
                                        {
                                        	return 6;
                                        }
                                        else
                                        {
                                            if (y <= 42)
                                            {
                                            	return 0;
                                            }
                                            else
                                            {
                                            	return 3;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        if (y > 33)
                        {
                        	return 4;
                        }
                        else
                        {
                            if (u <= 82)
                            {
                                if (v <= 56)
                                {
                                    if (u <= 81)
                                    {
                                    	return 6;
                                    }
                                    else
                                    {
                                        if (y <= 32)
                                        {
                                        	return 0;
                                        }
                                        else
                                        {
                                        	return 4;
                                        }
                                    }
                                }
                                else
                                {
                                    if (v <= 60)
                                    {
                                    	return 6;
                                    }
                                    else
                                    {
                                    	return 4;
                                    }
                                }
                            }
                            else
                            {
                                if (v > 56)
                                {
                                	return 4;
                                }
                                else
                                {
                                    if (v > 55)
                                    {
                                    	return 6;
                                    }
                                    else
                                    {
                                        if (y <= 30)
                                        {
                                        	return 0;
                                        }
                                        else
                                        {
                                        	return 4;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        if (v <= 52)
        {
            if (y <= 32)
            {
                if (y <= 25)
                {
                    if (u > 86)
                    {
                    	return 6;
                    }
                    else
                    {
                        if (y <= 23)
                        {
                        	return 6;
                        }
                        else
                        {
                            if (v <= 51)
                            {
                            	return 6;
                            }
                            else
                            {
                            	return 0;
                            }
                        }
                    }
                }
                else
                {
                    if (u <= 94)
                    {
                        if (y <= 28)
                        {
                            if (u <= 90)
                            {
                                if (y > 27)
                                {
                                	return 0;
                                }
                                else
                                {
                                    if (u <= 88)
                                    {
                                        if (v <= 43)
                                        {
                                        	return 6;
                                        }
                                        else
                                        {
                                        	return 0;
                                        }
                                    }
                                    else
                                    {
                                        if (y <= 26)
                                        {
                                        	return 6;
                                        }
                                        else
                                        {
                                            if (v <= 47)
                                            {
                                            	return 0;
                                            }
                                            else
                                            {
                                                if (u > 89)
                                                {
                                                	return 6;
                                                }
                                                else
                                                {
                                                    if (v <= 49)
                                                    {
                                                    	return 0;
                                                    }
                                                    else
                                                    {
                                                    	return 6;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            else
                            {
                                if (v > 47)
                                {
                                	return 6;
                                }
                                else
                                {
                                    if (u <= 91)
                                    {
                                    	return 0;
                                    }
                                    else
                                    {
                                    	return 6;
                                    }
                                }
                            }
                        }
                        else
                        {
                            if (u <= 91)
                            {
                            	return 0;
                            }
                            else
                            {
                                if (v <= 47)
                                {
                                	return 0;
                                }
                                else
                                {
                                    if (y <= 30)
                                    {
                                        if (u > 93)
                                        {
                                        	return 6;
                                        }
                                        else
                                        {
                                            if (v <= 48)
                                            {
                                            	return 0;
                                            }
                                            else
                                            {
                                            	return 6;
                                            }
                                        }
                                    }
                                    else
                                    {
                                        if (u <= 93)
                                        {
                                        	return 0;
                                        }
                                        else
                                        {
                                            if (y <= 31)
                                            {
                                            	return 6;
                                            }
                                            else
                                            {
                                            	return 0;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        if (v > 45)
                        {
                        	return 6;
                        }
                        else
                        {
                            if (y <= 29)
                            {
                            	return 6;
                            }
                            else
                            {
                            	return 0;
                            }
                        }
                    }
                }
            }
            else
            {
                if (v <= 48)
                {
                    if (u <= 87)
                    {
                        if (v <= 41)
                        {
                        	return 3;
                        }
                        else
                        {
                            if (y <= 45)
                            {
                            	return 0;
                            }
                            else
                            {
                                if (u > 85)
                                {
                                	return 0;
                                }
                                else
                                {
                                    if (y <= 49)
                                    {
                                        if (v <= 44)
                                        {
                                        	return 3;
                                        }
                                        else
                                        {
                                        	return 0;
                                        }
                                    }
                                    else
                                    {
                                        if (y <= 66)
                                        {
                                        	return 3;
                                        }
                                        else
                                        {
                                        	return 0;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        if (v <= 45)
                        {
                        	return 0;
                        }
                        else
                        {
                            if (y <= 44)
                            {
                                if (y > 34)
                                {
                                	return 0;
                                }
                                else
                                {
                                    if (u <= 96)
                                    {
                                    	return 0;
                                    }
                                    else
                                    {
                                    	return 6;
                                    }
                                }
                            }
                            else
                            {
                                if (y <= 48)
                                {
                                	return 0;
                                }
                                else
                                {
                                    if (u <= 102)
                                    {
                                    	return 0;
                                    }
                                    else
                                    {
                                    	return 4;
                                    }
                                }
                            }
                        }
                    }
                }
                else
                {
                    if (y <= 42)
                    {
                        if (u <= 97)
                        {
                        	return 0;
                        }
                        else
                        {
                            if (y <= 36)
                            {
                            	return 6;
                            }
                            else
                            {
                            	return 0;
                            }
                        }
                    }
                    else
                    {
                        if (v <= 50)
                        {
                            if (u <= 86)
                            {
                                if (y > 57)
                                {
                                	return 3;
                                }
                                else
                                {
                                    if (v <= 49)
                                    {
                                    	return 0;
                                    }
                                    else
                                    {
                                        if (y <= 45)
                                        {
                                        	return 0;
                                        }
                                        else
                                        {
                                        	return 4;
                                        }
                                    }
                                }
                            }
                            else
                            {
                                if (y <= 44)
                                {
                                	return 0;
                                }
                                else
                                {
                                    if (v <= 49)
                                    {
                                        if (y <= 46)
                                        {
                                        	return 0;
                                        }
                                        else
                                        {
                                            if (u <= 99)
                                            {
                                                if (u <= 91)
                                                {
                                                	return 0;
                                                }
                                                else
                                                {
                                                    if (y <= 50)
                                                    {
                                                    	return 4;
                                                    }
                                                    else
                                                    {
                                                    	return 0;
                                                    }
                                                }
                                            }
                                            else
                                            {
                                                if (y <= 51)
                                                {
                                                	return 0;
                                                }
                                                else
                                                {
                                                	return 4;
                                                }
                                            }
                                        }
                                    }
                                    else
                                    {
                                        if (y <= 52)
                                        {
                                        	return 4;
                                        }
                                        else
                                        {
                                            if (u <= 101)
                                            {
                                            	return 0;
                                            }
                                            else
                                            {
                                            	return 4;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                            if (v > 51)
                            {
                            	return 4;
                            }
                            else
                            {
                                if (y <= 47)
                                {
                                    if (u > 100)
                                    {
                                    	return 0;
                                    }
                                    else
                                    {
                                        if (u <= 92)
                                        {
                                        	return 0;
                                        }
                                        else
                                        {
                                            if (u <= 98)
                                            {
                                            	return 4;
                                            }
                                            else
                                            {
                                                if (y <= 45)
                                                {
                                                	return 0;
                                                }
                                                else
                                                {
                                                	return 4;
                                                }
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    if (y <= 58)
                                    {
                                    	return 4;
                                    }
                                    else
                                    {
                                    	return 0;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        else
        {
            if (y <= 32)
            {
                if (y <= 28)
                {
                    if (y <= 23)
                    {
                    	return 6;
                    }
                    else
                    {
                        if (u > 88)
                        {
                        	return 6;
                        }
                        else
                        {
                            if (v <= 56)
                            {
                                if (y <= 26)
                                {
                                    if (u > 86)
                                    {
                                    	return 6;
                                    }
                                    else
                                    {
                                        if (u <= 85)
                                        {
                                        	return 0;
                                        }
                                        else
                                        {
                                            if (y <= 24)
                                            {
                                            	return 6;
                                            }
                                            else
                                            {
                                                if (v <= 55)
                                                {
                                                	return 0;
                                                }
                                                else
                                                {
                                                	return 6;
                                                }
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    if (y > 27)
                                    {
                                    	return 0;
                                    }
                                    else
                                    {
                                        if (u <= 86)
                                        {
                                        	return 0;
                                        }
                                        else
                                        {
                                            if (v <= 54)
                                            {
                                            	return 0;
                                            }
                                            else
                                            {
                                            	return 6;
                                            }
                                        }
                                    }
                                }
                            }
                            else
                            {
                                if (u > 85)
                                {
                                	return 6;
                                }
                                else
                                {
                                    if (v > 57)
                                    {
                                    	return 6;
                                    }
                                    else
                                    {
                                        if (y <= 25)
                                        {
                                        	return 6;
                                        }
                                        else
                                        {
                                        	return 0;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                else
                {
                    if (u <= 92)
                    {
                        if (v <= 56)
                        {
                            if (v <= 54)
                            {
                            	return 0;
                            }
                            else
                            {
                                if (y <= 29)
                                {
                                	return 0;
                                }
                                else
                                {
                                    if (u <= 88)
                                    {
                                        if (y > 30)
                                        {
                                        	return 4;
                                        }
                                        else
                                        {
                                            if (v <= 55)
                                            {
                                            	return 0;
                                            }
                                            else
                                            {
                                            	return 4;
                                            }
                                        }
                                    }
                                    else
                                    {
                                        if (y <= 30)
                                        {
                                        	return 0;
                                        }
                                        else
                                        {
                                        	return 6;
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                            if (u <= 90)
                            {
                            	return 4;
                            }
                            else
                            {
                                if (y <= 31)
                                {
                                	return 6;
                                }
                                else
                                {
                                	return 4;
                                }
                            }
                        }
                    }
                    else
                    {
                        if (v <= 58)
                        {
                        	return 6;
                        }
                        else
                        {
                            if (u <= 93)
                            {
                            	return 4;
                            }
                            else
                            {
                            	return 6;
                            }
                        }
                    }
                }
            }
            else
            {
                if (v <= 55)
                {
                    if (y > 40)
                    {
                    	return 4;
                    }
                    else
                    {
                        if (u <= 90)
                        {
                            if (v > 53)
                            {
                            	return 4;
                            }
                            else
                            {
                                if (y <= 37)
                                {
                                    if (u <= 87)
                                    {
                                    	return 0;
                                    }
                                    else
                                    {
                                        if (y <= 33)
                                        {
                                        	return 0;
                                        }
                                        else
                                        {
                                            if (y <= 34)
                                            {
                                            	return 6;
                                            }
                                            else
                                            {
                                                if (u <= 88)
                                                {
                                                	return 0;
                                                }
                                                else
                                                {
                                                	return 4;
                                                }
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    if (u <= 88)
                                    {
                                    	return 4;
                                    }
                                    else
                                    {
                                    	return 0;
                                    }
                                }
                            }
                        }
                        else
                        {
                            if (y <= 36)
                            {
                                if (u > 95)
                                {
                                	return 6;
                                }
                                else
                                {
                                    if (y > 34)
                                    {
                                    	return 0;
                                    }
                                    else
                                    {
                                        if (v <= 53)
                                        {
                                        	return 0;
                                        }
                                        else
                                        {
                                            if (u <= 91)
                                            {
                                            	return 0;
                                            }
                                            else
                                            {
                                            	return 6;
                                            }
                                        }
                                    }
                                }
                            }
                            else
                            {
                                if (v <= 54)
                                {
                                	return 0;
                                }
                                else
                                {
                                    if (y <= 37)
                                    {
                                    	return 0;
                                    }
                                    else
                                    {
                                    	return 4;
                                    }
                                }
                            }
                        }
                    }
                }
                else
                {
                    if (y > 37)
                    {
                    	return 4;
                    }
                    else
                    {
                        if (u <= 95)
                        {
                        	return 4;
                        }
                        else
                        {
                            if (y <= 35)
                            {
                            	return 6;
                            }
                            else
                            {
                                if (y <= 36)
                                {
                                	return 6;
                                }
                                else
                                {
                                	return 4;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

    }
}
