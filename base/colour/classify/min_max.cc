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

#include <string>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include "colour_definition.h"

#define MAX_VALUE 127
#define MAX_Y MAX_VALUE
#define MAX_U MAX_VALUE
#define MAX_V MAX_VALUE

#define LINE_LENGTH 80

using namespace std;

int min_y, min_u, min_v;
int max_y, max_u, max_v;

bool wanted_colour(unsigned char c)
{
    if (c<BACKGROUND)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void binary_min_max(char* fileName)
{
    ifstream file(fileName,ios::binary);
    unsigned char c;
    for (int y=0; y<=MAX_Y; ++y)
    {
        for (int u=0; u<=MAX_U; ++u)
        {
            for (int v=0; v<=MAX_V; ++v)
            {
                if (!file.read((unsigned char *)(&c), sizeof(c)))
                {
                    printf("(%d, %d, %d)  (%d, %d, %d)\n", min_y, min_u, min_v, max_y, max_u, max_v);
                    return;
                }
                if (wanted_colour(c))
                {
                    if (y < min_y)
                        min_y = y;
                    if (u < min_u)
                        min_u = u;
                    if (v < min_v)
                        min_v = v;

                    if (y > max_y)
                        max_y = y;
                    if (u > max_u)
                        max_u = u;
                    if (v > max_v)
                        max_v = v;
                }
            }
        }
    }
    printf("(%d, %d, %d)  (%d, %d, %d)\n", min_y, min_u, min_v, max_y, max_u, max_v);
    file.close();
}

void ascii_min_max(char* fileName)
{
    FILE *file;
    char line[80];
    file = fopen(fileName,"r");
    int y,u,v,c;
    
    if (fgets(line, LINE_LENGTH, file) == NULL)
    {
        fprintf(stderr,"No data in file\n");
        return;
    }
    
    while (fgets(line, LINE_LENGTH, file) != NULL)
    {
        sscanf(line,"%d %d %d %d", &y, &u, &v, &c);
        
        if (wanted_colour((char)c))
        {
            if (y < min_y)
                min_y = y;
            if (u < min_u)
                min_u = u;
            if (v < min_v)
                min_v = v;

            if (y > max_y)
                max_y = y;
            if (u > max_u)
                max_u = u;
            if (v > max_v)
                max_v = v;
        }
    }
    printf("(%d, %d, %d)  (%d, %d, %d)\n", min_y, min_u, min_v, max_y, max_u, max_v);
    fclose(file);
}

void print_usage()
{
    printf("Usage: ./min_max <a|b> <filename>\n");
    printf("Use option \"a\" for ascii mode or \"b\" for binary mode\n");
}

int main(int argc, char** argv)
{
    min_y = min_u = min_v = MAX_VALUE;
    max_y = max_u = max_v = 0;
    if (argc == 3)
    {
        if (strcmp(argv[1],"a")==0)
            ascii_min_max(argv[2]);
        else if (strcmp(argv[1],"b")==0)
            binary_min_max(argv[2]);
        else
            print_usage();
    }
    else
    {
        print_usage();
    }
}
