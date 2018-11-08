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

#include <stdio.h>
#include <iostream>
#include <math.h>
#include <string>
#include <fstream>
#include <list>
#include "colour_definition.h"

// constant determining the minimum confidence level that has to be reached
// in order for a colour to be classified
#define MIN_CF 0

// constant for determining the confidence level
#define K 0.1

// Techniques we can use.
#define MNN_ALG 0
#define METHOD MNN_ALG

#define THRESHOLD 3*2

#define MAXY 128
#define MAXU 128
#define MAXV 128

#define CUBE_SIZE 128

// Define this constant if you want binary output.
#define BINARY_OUTPUT

using namespace std;

int counter = 0;

typedef struct _pixel
{
    int y,u,v;
    
    unsigned char colour;

} pixel;


#if METHOD == MNN_ALG
// an manhattan no sum nearest neighbour pixel
typedef struct _mnnpixel
{
    // The current manhattan distance to the nearest training pixel.
    int distance;
    
    // The colour of the nearest training pixel.
    unsigned char colour;

} mnnpixel;
#endif


// the mini cube mnnpixel list
mnnpixel *mset;

// the size of the mini cube pixel list
unsigned int mset_size;

// the training set pixel list
pixel *tset;

// the size of the current training set
unsigned int tset_size;

unsigned int min_y, min_u, min_v, max_y, max_u, max_v;


// the manhattan distance between the two pixels
unsigned int distance(pixel *a, pixel *b)
{
    unsigned int manh_dist = abs((int)a->y - (int)b->y) + abs((int)a->u - (int)b->u) + abs((int)a->v - (int)b->v);
    return manh_dist;
}


/**
 * Sets the nearest training set pixel colour to the pixel;
 * @param cur - has to have cube coordinates
 * @param tset_p - the training set pixel
 */
void nearest_neighbour(pixel* cur, pixel* tset_p)
{
    // if the current pixel is not within cube, don't add the confidence
    // level
    if (cur->y < 0 || cur->y >= CUBE_SIZE ||
        cur->u < 0 || cur->u >= CUBE_SIZE ||
        cur->v < 0 || cur->v >= CUBE_SIZE)
        return;


    int index = cur->y*CUBE_SIZE*CUBE_SIZE + cur->u*CUBE_SIZE + cur->v;    
    
    #if METHOD == MNN_ALG
    
    int cf = distance(cur,tset_p);
    
   /* 
    if (mset[index].colour == BLUE_DOG && tset_p->colour == BACKGROUND && mset[index].distance != 0) {
        mset[index].colour = BLUE_DOG_OR_BACKGROUND;
    }
    else if (mset[index].colour == BACKGROUND && tset_p->colour == BLUE_DOG && mset[index].distance != 0) {
        mset[index].colour = BLUE_DOG_OR_BACKGROUND;
    }
    else if (mset[index].colour != BLUE_DOG_OR_BACKGROUND) {
        if (mset[index].distance > cf) {
            mset[index].distance = cf;
            mset[index].colour = tset_p->colour;
        }
    }
*/
        if (mset[index].distance > cf) {
            mset[index].distance = cf;
            mset[index].colour = tset_p->colour;
        }

    #endif
}

void mirror_pixel(pixel* cur, pixel* tset_p)
{
    pixel tmp;
    int diff_y = abs(cur->y - tset_p->y);
    int diff_u = abs(cur->u - tset_p->u);
    int diff_v = abs(cur->v - tset_p->v);
    nearest_neighbour(cur,tset_p);
    
    // (y,u,v)
    tmp.y = cur->y;
    tmp.u = cur->u;
    tmp.v = cur->v;
    
    if (cur->u!=tset_p->u)
    {
        // (y,-u,v)
        tmp.y = cur->y;
        tmp.u = tset_p->u - diff_u;
        tmp.v = cur->v;
        nearest_neighbour(&tmp,tset_p);
        
        if (cur->v!=tset_p->v)
        {
            // (y,-u,-v)
            tmp.y = cur->y;
            tmp.u = tset_p->u - diff_u;
            tmp.v = tset_p->v - diff_v;
            nearest_neighbour(&tmp,tset_p);
        }
    }
    
    if (cur->v!=tset_p->v)
    {
        // (y,u,-v)
        tmp.y = cur->y;
        tmp.u = cur->u;
        tmp.v = tset_p->v - diff_v;
        nearest_neighbour(&tmp,tset_p);
    }
    
    if (cur->y!=tset_p->y)
    {
        // (-y,u,v)
        tmp.y = tset_p->y - diff_y;
        tmp.u = cur->u;
        tmp.v = cur->v;
        nearest_neighbour(&tmp,tset_p);

        if (cur->u!=tset_p->u)
        {
            // (-y,-u,v)
            tmp.y = tset_p->y - diff_y;
            tmp.u = tset_p->u - diff_u;
            tmp.v = cur->v;
            nearest_neighbour(&tmp,tset_p);

            if (cur->v!=tset_p->v)
            {
                // (-y,-u,-v)
                tmp.y = tset_p->y - diff_y;
                tmp.u = tset_p->u - diff_u;
                tmp.v = tset_p->v - diff_v;
                nearest_neighbour(&tmp,tset_p);
            }
        }
        
        if (cur->v!=tset_p->v)
        {
            // (-y,u,-v)
            tmp.y = tset_p->y - diff_y;
            tmp.u = cur->u;
            tmp.v = tset_p->v - diff_v;
            nearest_neighbour(&tmp,tset_p);
        }
    }
}

/**
 * Used in Manhattan Nearest Neighbour Algorithm. Fills the neighbouring
 * pixels with the confidence level.
 * @param tset_p - the current training set pixel
 */
void explore_neighbours(pixel* tset_p)
{
    pixel tmp;
    int limit;
    int y,u,v;
    y = tset_p->y;
    u = tset_p->u;
    v = tset_p->v;
    tmp.y = tset_p->y;
    tmp.u = tset_p->u;
    tmp.v = tset_p->v;
    
    nearest_neighbour(tset_p, tset_p);

    if (tset_p->colour == BACKGROUND)
    {
        limit = 0;
    }
    else if (tset_p->colour == GREEN_FIELD)
    {
        limit = THRESHOLD-4;
    }
    else if (tset_p->colour == GREEN)
    {
        limit = THRESHOLD - 4;
    }
    else if (tset_p->colour == BLUE)
    {
        limit = THRESHOLD - 2;
    }
    else if (tset_p->colour == BLUE_DOG)
    {
        limit = THRESHOLD + 2;
    }
    else
    {
        limit = THRESHOLD;
    }
    
    for (int i=1; i<=limit; ++i)
    {
        for (int j=0; j<=i; ++j)
        {
            for (int k=0; k<=i-j; ++k)
            {
                
                tmp.y = y + (i-j-k);
                tmp.u = u + k;
                tmp.v = v + j;
                mirror_pixel(&tmp,tset_p);
            }
        }
    }
}


/**
 * Retrieve the most dominant colour of each of the pixels in the mini-cube.
 */
void classify_pixels()
{
    int index;
    unsigned char colour;
    
    #if METHOD == MNN_ALG
    // Replace all the unclassified pixels with background colour.
    for (int i=0; i<CUBE_SIZE; ++i)
    {
        for (int j=0; j<CUBE_SIZE; ++j)
        {
            for (int k=0; k<CUBE_SIZE; ++k)
            {
                index = i*CUBE_SIZE*CUBE_SIZE + j*CUBE_SIZE + k;
                if (mset[index].colour == UNCLASSIFIED) {
                    mset[index].colour = BACKGROUND;
                }
                
                // Replace 10 with background colour for colourcube purposes
                if (mset[index].colour == 10) {
                    mset[index].colour = UNCLASSIFIED;
                }
            }
        }
    }
    #endif
    
    for (int i=0; i<CUBE_SIZE; ++i)
    {
        for (int j=0; j<CUBE_SIZE; ++j)
        {
            for (int k=0; k<CUBE_SIZE; ++k)
            {
                index = i*CUBE_SIZE*CUBE_SIZE + j*CUBE_SIZE + k;
                
                #if METHOD == MNN_ALG
                colour = mset[index].colour;
                #endif
                
                #ifdef BINARY_OUTPUT
                printf("%c", colour);
                #endif
                
                #ifndef BINARY_OUTPUT
                printf("%d %d %d %d\n",min_y+i, min_u+j, min_v+k, colour);
                #endif
            }
        }
    }
}

/**
 * Classifies the colour of each of the pixels in the mini-cube.
 * Used in the Manhattan Nearest Neighbour Algorithm.
 */
void classify_cube(string fileName)
{
    string line;
    char cline[78];
    pixel tmp;
    
    ifstream file(fileName.c_str());

    int mset_min_u = min_u - THRESHOLD;
    int mset_max_u = max_u + THRESHOLD;
    
    if (mset_min_u < 0 ) {
        mset_min_u = 0;
    }
    if (mset_max_u > MAXU - 1) {
        mset_max_u = MAXU - 1;
    }

    int mset_min_y = min_y - THRESHOLD;
    int mset_max_y = max_y + THRESHOLD;
    
    if (mset_min_y < 0 ) {
        mset_min_y = 0;
    }
    if (mset_max_y > MAXY - 1) {
        mset_max_y = MAXY - 1;
    }

    int mset_min_v = min_v - THRESHOLD;
    int mset_max_v = max_v + THRESHOLD;
    
    if (mset_min_v < 0 ) {
        mset_min_v = 0;
    }
    if (mset_max_v > MAXV - 1) {
        mset_max_v = MAXV - 1;
    }
    
    // Ignore the first line.
//    getline(file,line);
    
    // Read each line in the file.
    int y, u, v, colour;
    
    while (getline(file, line))
    {
        strcpy(cline, line.c_str());

        // Tokenise the line to extract the Y, U , V coordinates and the
        // associated colour.
        sscanf(cline, "%d %d %d %d\n", &y, &u, &v, &colour);
        
        // Process the training set pixel if it is within region of influence.
        if (y >= mset_min_y && y <= mset_max_y && 
            u >= mset_min_u && u <= mset_max_u && 
            v >= mset_min_v && v <= mset_max_v)
        {
            // we actually pass the coordinate of the pixel in the cube's
            // coordinate space
            tmp.y = y - mset_min_y;
            tmp.u = u - mset_min_u;
            tmp.v = v - mset_min_v;
            tmp.colour = (unsigned char)colour;
            explore_neighbours(&tmp);
            if (counter%10000==0)
                cerr << "up to: " << counter << endl;
            ++counter;
        }
    }
    
    file.close();
    /***********************************************************
     * Need to go through the whole cube and choose the colour *
     * with the greatest weighting                             *
     ***********************************************************/
    classify_pixels();
}


/*
 * Use the manhattan without sum nearest neighbour algorithm.
 * Does not use mini-cubes assumes the total cube is small enough
 * for brute calculation.
 */
#if METHOD == MNN_ALG
void mnn_algorithm(int argc, char** argv)
{
    min_y = 0;
    min_u = 0;
    min_v = 0;
    max_y = min_y + CUBE_SIZE - 1;
    max_u = min_u + CUBE_SIZE - 1;
    max_v = min_v + CUBE_SIZE - 1;
    mset_size = MAXY*MAXU*MAXV;
    mset = (mnnpixel *)calloc(mset_size,sizeof(mnnpixel));

    // Initialise all the cubes in the pixel to a distance larger than the threshold.
    // And initialise all pixels to the background colour.
    for (int i = 0; i < mset_size; i++) {
        mset[i].colour = UNCLASSIFIED;
        mset[i].distance = THRESHOLD+1;
    }

    if (argc<2)
        return;

    classify_cube(argv[1]);    
}
#endif

int main(int argc, char** argv)
{
    #ifndef BINARY_OUTPUT
    // needed for colourcube REMOVE LATER
    printf("%d %d %d\n", MAXY, MAXU, MAXV);
    #endif
    #if METHOD == MNN_ALG
    mnn_algorithm(argc,argv);
    #endif
}
