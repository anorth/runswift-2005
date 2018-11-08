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

/*
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 * 
 **/

#include "Blob.h"
#include "ring.h"
#include <sys/time.h>

#ifndef OFFLINE
#include <MCOOP.h>
#endif

#define JOIN4DIRECTION

static bool bDebugBlob = false;

Blobber::Blobber(GeneralisedColourDetect &gcd) {
    cplane = gcd.cplane;
    nn_multi_colour = gcd.nn_multi_colour;

    for (int i = 0; i < NUM_USABLE_COLORS; i++) {
        blobs[i].init(i);
    }

    for (int i = 0; i < NUM_USABLE_COLORS; i++) {
        barray[i].init(i); //reset everything
    }

    usingHeap = 0;
    usingAlt = 1;
    multi_colour = 0;

    for (int k = 0; k < NUM_USABLE_COLORS; k++) {
        nbob[k] = 0;
    }
}


void Blobber::createBlob(RunLengthInfo *segment, int y) {
    int i;

    if (top_bn[segment->color] >= 0) { //There are reusable blob numbers
        i = available_bn[segment->color][top_bn[segment->color]--];
    }
    else {
        // Get the next unused blob number
        i = nbob[segment->color]++;
    }

    if (i >= MAX_NUM_BLOBS){
        if (bDebugBlob)
            cout << "Max num blobs exceeded set i = 0" << endl;
        i = 0;
    }

    if (i < MAX_NUM_BLOBS) {
        segment->rank = 0;
        segment->blob_number = i;
        segment->nextSeg = segment;
        blobinfo[segment->color][i].Start(segment->rawColor, segment->startIndex, segment->endIndex, y, segment->length, i);
        blobinfo[segment->color][i].xsum = segment->xsum;
        blobinfo[segment->color][i].ysum = segment->ysum;
        if (bDebugBlob)
            if (segment->color == cROBOT_RED)
                cout << "New Red blob number = " << segment->blob_number << "  row = " << y << endl;

    }
    else {
        cout << "Running out of blob number!, i = " << i << " and colour = " << segment->color << " and y = " << y << endl;
    }
}

// Find root of set
RunLengthInfo * Find_Set(RunLengthInfo *cs) {
    if (cs->nextSeg == 0) {
        cout << "Error, the parent pointer should pointing to something!" << endl;
        return 0;
    }

    RunLengthInfo *root = cs;
    while (root != root->nextSeg){
        root = root->nextSeg;
    }

    while (cs != root){
        RunLengthInfo *temp = cs->nextSeg;
        cs ->nextSeg = root;
        cs = temp;
    }

    return root;
}

inline void Link(RunLengthInfo *x, RunLengthInfo *y) {
    if (x->rank == -1 || x->rank == -1) {
        cout << "Error, both x and y should have rank at least 0!" << endl;
        return;
    }

    if (x->rank > y->rank) {
        y->nextSeg = x;
    }
    else {
        x->nextSeg = y;

        if (x->rank == y->rank)
            y->rank ++;
    }
}

/*
   inline void Merge(RunLengthInfo *x, RunLengthInfo *y) {
   Link(Find_Set(x), Find_Set(y));
   }
 */

void Blobber::jointSegments(int row,RunLengthInfo *cs,RunLengthInfo *ps){
    /*
       if ((ps->color & COLOR_MASK) >= NUM_USABLE_COLORS)
       continue;
     */
    // If the current segment has no root, then create a root.
    if (cs->nextSeg == 0) { //no next segment
        cs->nextSeg = cs;
    }

    // Get the root of the current segment and previous segment.
    RunLengthInfo *csRoot = Find_Set(cs);
    RunLengthInfo *psRoot = Find_Set(ps);

    /*
       if (y == 14 && cs->color == 4) {
       test();	
       cout << "(" << cs->startIndex << "," << cs->endIndex << ")" << endl; 
       }
     */

    // If the root have same color (we join same color segments only). 
    // If the roots are the identical and they have at least one pixel overlap.
#ifdef JOIN4DIRECTION
    if (csRoot->color == psRoot->color && csRoot != psRoot && ps->overlaps(cs)) {
#else
    if (csRoot->color == psRoot->color && csRoot != psRoot && ps->overlaps8(cs)) {
#endif
        // If the current segment is not joined with any other segment before.
        if (csRoot->blob_number == -1) {

            // Copy the blob number from the previous segment.
            csRoot->blob_number = psRoot->blob_number;

            // Merge both segments in a single disjoint set.
            Link(csRoot, psRoot);

            // Also update the sum of x, y and pixles.
            blobinfo[psRoot->color][psRoot->blob_number].Update(cs->rawColor, cs->startIndex, cs->endIndex, row, cs->length, cs->xsum, cs->ysum);

            // definitely affect the small and large in this case.

            blobinfo[psRoot->color][psRoot->blob_number].large = cs->endIndex;
        }
        else {

            // Get the blobinfo from previous and current segment.
            BlobInfo *pblob = &blobinfo[psRoot->color][psRoot->blob_number];
            BlobInfo *cblob = &blobinfo[csRoot->color][csRoot->blob_number];

            // Merge both disjoint sets.
            Link(csRoot, psRoot);

            // Blob number deallocation,
            available_bn[csRoot->color][++top_bn[csRoot->color]] = csRoot->blob_number;;

            // Find the root of the new disjoint set.
            RunLengthInfo *root = Find_Set(ps);

            // This new root is assigned the blob number from the previous segment.
            root->blob_number = psRoot->blob_number;

            // We deallocate the blob number from the current segment.
            pblob->Update(cblob);
            //cblob->ResetBlob();
            cblob->area = 0;
        }
    }
    else {
        // ps is not merged with cs. If the blobinfo[ps] have a maximum y value not equal to the current row value.
        // And if there is no chance that ps will merge with any other segment after cs. If these conditions satsify,
        // get rid of ps immediately.
        BlobInfo *pblob = &blobinfo[psRoot->color][psRoot->blob_number];

        // first work for no dummy, second for the dummy.
#ifdef JOIN4DIRECTION
        if (pblob->getYMax() < row && cs->endIndex >= pblob->large && ps->endIndex == pblob->large ) 
#else
        //changed >= to > for diagonal matching
        if (pblob->getYMax() < row && cs->endIndex > pblob->large && ps->endIndex == pblob->large ) 
#endif
        {

            HalfBeacon &blob = blobinfo[psRoot->color][psRoot->blob_number];

            if (!barray[psRoot->color].add(blob)) {
                if (bDebugBlob)
                    cout << "barray rejected blob : " << psRoot->blob_number << " color : " << psRoot->color << endl;
                // Deallocate the blob number regardless pblob has been successfully added to the barray or not.				
                available_bn[psRoot->color][++top_bn[psRoot->color]] = psRoot->blob_number;
                pblob->ResetBlob();

            }
        }		
    }

}

void Blobber::formDisjoint() {

#ifdef OFFLINE
    timeval start;
    gettimeofday(&start, NULL);
    double t1 = start.tv_sec + (start.tv_usec);
#endif

    // Get the array of segment counts, one entry for each row
    int *segmentCount = splane.segmentCount;

    // Create blobs for segments in the first row only.
    for (int x = 0; x < segmentCount[0]; x++) {
        RunLengthInfo *cs = &splane.segment[0][x];
        /*
        // don't blob field green or higher colours
        if ((cs->color & COLOR_MASK) >= NUM_USABLE_COLORS)
        continue;
         */
        createBlob(cs, 0);	
    }

    int prev_y = 0;

    // Process the segments from row 2 to the end of the CPlane. 
    for (int y = 1; y < USABLE_HEIGHT; y++, prev_y++) {

        int w = 0; //index of segments in previous row

        // Process all segments in the current row.
        for (int x = 0; x <= segmentCount[y]; x++) {
            // Note - this runs one segment over the end of the initialized data
            // the final run will be initialised below - assumes we have extra space in the array

            RunLengthInfo *cs = &splane.segment[y][x];
            /*
            // don't blob field green or higher colours
            if ((cs->color & COLOR_MASK) >= NUM_USABLE_COLORS)
            continue;
             */  	  
            if (x == segmentCount[y]) {
                cs->Set(y, 1000, 1000, 10);
            }

            RunLengthInfo *ps = NULL;

            // Compare all the segments from the previous row.
            for (; w < segmentCount[prev_y]; w++) {

                // A pointer to a segment from the previous row.
                ps = &splane.segment[prev_y][w];
                jointSegments(y,cs,ps);
                // Skip to the next row segment if previous segment is larger than current segment to compare.
                if (ps->endIndex >= cs->endIndex)
                    break;

            }

            //If not joined to any segment then create single blob
            if (cs->blob_number == -1 && x != segmentCount[y]) {
                createBlob(cs, y);
            }
            // Joint the tricky diagonal case
            //need to  joint (cs with nextPS ) and (nextCS with PS)

            if (ps != 0 && ps->endIndex == cs->endIndex){ 
#ifdef JOIN4DIRECTION
                w++;
#else                
                if (w + 1 < segmentCount[prev_y]){
                    RunLengthInfo *nextPS = &splane.segment[prev_y][w+1];
                    jointSegments(y,cs,nextPS);
                }
#endif  //JOIN4DIRECTION
            }
        }
    }

    for (int xx = 0; xx < segmentCount[USABLE_HEIGHT - 1]; xx++) {
        RunLengthInfo *ps = &splane.segment[USABLE_HEIGHT - 1][xx];
        /*
        // don't blob field green or higher colours
        if ((ps->color & COLOR_MASK) >= NUM_USABLE_COLORS)
        continue;
         */
        RunLengthInfo *psRoot = Find_Set(ps);

        BlobInfo *pblob = &blobinfo[psRoot->color][psRoot->blob_number];

        if (ps->endIndex == pblob->large) {

            HalfBeacon &blob = blobinfo[psRoot->color][psRoot->blob_number];

            if (!barray[psRoot->color].add(blob)) {

                if (bDebugBlob)
                    cout << "barray rejected blob : " << psRoot->blob_number << " color : " << psRoot->color << endl;
                // Deallocate the blob number regardless pblob has been successfully added to the barray or not.				
                available_bn[psRoot->color][++top_bn[psRoot->color]] = psRoot->blob_number;
                pblob->ResetBlob();
            }
        }
    }

#ifdef OFFLINE
    timeval end;
    gettimeofday(&end, NULL);
    double t2 = end.tv_sec + (end.tv_usec);
    //cout << "Time took by disjoint: " << t2 - t1 << endl;
#endif

}

// This function allows the programmer to test and compare disjoint sets and old blob algorithm.
void Blobber::test() {

#ifndef OFFLINE
	// System time for the old blob algorithm.
	SystemTime oldStartTime;
	GetSystemTime(&oldStartTime);
#endif

#ifdef OFFLINE
	// System time for the old blob algorithm.
    timeval oldStartTime;
    gettimeofday(&oldStartTime, NULL);
    double oldStartTimeSecond = oldStartTime.tv_sec + (oldStartTime.tv_usec);
#endif

	// Firstly form the blobs with the old blob algorithm.
    formOriginalBlobs();

#ifndef OFFLINE
	SystemTime oldEndTime;
	GetSystemTime(&oldEndTime);
	SystemTime oldUseTime = oldEndTime - oldStartTime;
#endif
	
#ifdef OFFLINE
    timeval oldEndTime;
    gettimeofday(&oldEndTime, NULL);
    double oldEndTimeSecond = oldEndTime.tv_sec + (oldEndTime.tv_usec);
	double oldUseTime       = oldEndTimeSecond - oldStartTimeSecond;
	cout << "Original takes: " << oldUseTime << endl;
#endif
	
	// Reset everything for the disjoint sets blob algorithm.
    int barraySizeO[NUM_USABLE_COLORS];
    int barraySizeD[NUM_USABLE_COLORS];

    for (int i = 0; i < NUM_USABLE_COLORS; i++)
        barraySizeO[i] = barray[i].size;

	// Reset top_bn and nbob.
    reset();
	
    int *segmentCount = splane.segmentCount;

    // Reset the segmentPlane.
    for (int y = 0; y < USABLE_HEIGHT; y++) {
        for (int x = 0; x < segmentCount[y]; x++) {
            RunLengthInfo *run = &splane.segment[y][x];
            run->nextSeg = 0;
            run->blob_number = -1;
            run->rank = 0;
        }
    } 

    barray->clear();

#ifndef OFFLINE
	// System time for the new blob algorithm.
	SystemTime newStartTime;
	GetSystemTime(&newStartTime);
#endif

#ifdef OFFLINE
	// System time for the new blob algorithm.
    timeval newStartTime;
    gettimeofday(&newStartTime, NULL);
    double newStartTimeSecond = newStartTime.tv_sec + (newStartTime.tv_usec);
#endif

	// Form the blobs with the disjoint blob algorithm.	
    formDisjoint();

#ifndef OFFLINE	
	SystemTime newEndTime;
	GetSystemTime(&newEndTime);
	SystemTime newUseTime = newEndTime - newStartTime;

	// Format: old time <tab> new time
	cout << oldUseTime.useconds << "\t" << newUseTime.useconds << endl;
#endif

#ifdef OFFLINE
    timeval newEndTime;
    gettimeofday(&newEndTime, NULL);
    double newEndTimeSecond = newEndTime.tv_sec + (newEndTime.tv_usec);
	double newUseTime       = newEndTimeSecond - newStartTimeSecond;
	cout << "Disjoint takes: " << newUseTime << endl;
#endif

	/*	
    	for (int i = 0; i < NUM_USABLE_COLORS; i++)
    	    barraySizeD[i] = barray[i].size;

	    for (int i = 0; i < NUM_USABLE_COLORS; i++) {
        	if (barraySizeO[i] != barraySizeD[i])
        	    cout << "Color i: " << i << " - Something wrong!!!. Original = " << barraySizeO[i] << "  Disjoint = " << barraySizeD[i] << endl;
    	}
	
	*/
	
}


void Blobber::formBlobs() {
	// Compare disjoint sets with the old blob algorithm.
	// test();

	// Disjoint sets blob algorithm.
    	formDisjoint();
}


void Blobber::updateArray(int y) {
    for (int j = 0; j < NUM_USABLE_COLORS; j++) {
        for (vector<int>::reverse_iterator i = openBlobs[j].rbegin();
                i != openBlobs[j].rend();
                i++) {
            /*
               if (y == 14 && j == 4 && *i == 52)
               test();
             */

            HalfBeacon &blob = blobinfo[j][*i];

            // blob is marked for deletion
            if (blob.area == 0) {
                available_bn[j][++top_bn[j]] = *i;
                *i = openBlobs[j][(int)openBlobs[j].size() - 1];
                openBlobs[j].pop_back();
            }
            else if (blob.getYMax() != y) {
                /* Blob hasn't been altered so we store it if it meets all thresholds.  
                 * always erase it from the openBlobs list because 
                 * it can no longer connect to anything.
                 **/
                // ricky: only store core blobs, no point storing maybe blobs that 
                // cant connect to anything
                if (blob.color < cFIELD_GREEN) {
                    if (!barray[j].add(blob)) {
                        available_bn[j][++top_bn[j]] = *i;
                        blob.area = 0;
                    }
                }
                *i = openBlobs[j][(int)openBlobs[j].size() - 1];
                openBlobs[j].pop_back();
            }
        }
    }
}


void Blobber::reloadNNMC(GeneralisedColourDetect &gcd){
    gcd.reloadNNMC();
    cplane = gcd.cplane;
    nn_multi_colour = gcd.nn_multi_colour;
}


void Blobber::createOriginalBlob(RunLengthInfo *segment, int y) {
    int i;
    if (nbob[segment->color] >= MAX_NUM_BLOBS && top_bn[segment->color] >= 0) {
        i = available_bn[segment->color][top_bn[segment->color]--];
    }
    else {
        i = nbob[segment->color]++;
    }

    if (i < MAX_NUM_BLOBS) {
        segment->blob_number = i;
        blobinfo[segment->color][i].Start(segment->rawColor,
                segment->startIndex,
                segment->endIndex,
                y,
                segment->length,
                i);
        blobinfo[segment->color][i].xsum = segment->xsum;
        blobinfo[segment->color][i].ysum = segment->ysum;
        openBlobs[segment->color].push_back(i);
    }
}


void Blobber::updateBlobNums(RunLengthInfo segment[],
        int min,
        int max,
        int oldnum,
        int newnum,
        int colorMatch,
        int newcolor) {
    for (int i = min; i <= max; i++) {
        if (segment[i].blob_number == oldnum && segment[i].color == colorMatch) {
            segment[i].blob_number = newnum;
            segment[i].color = newcolor;
        }
    }
}









void Blobber::formOriginalBlobs() {

    /* Array of number of segments in each row. */
    int *segmentCount = splane.segmentCount;

    //runLengthEncode ();

    /* Go through segments in first row. */
    for (int x = 0; x < segmentCount[0]; x++) {
        RunLengthInfo *cs = &splane.segment[0][x];

        /* If this segment is not field green (see enum Colors in
         * GeneralisedColorDetect.h). */ 
        if (cs->color < 7)
            createOriginalBlob(cs, 0);
    }

    // Merge blobs every 2 lines
    int prev_y = 0;
    for (int y = 1; y < USABLE_HEIGHT; y++, prev_y++) {
        /*
#ifdef OFFLINE
cout << "at height " << y << endl;
#endif                        
         */
        int w = 0;
        for (int x = 0; x < segmentCount[y]; x++) {
            RunLengthInfo *cs = &splane.segment[y][x];
            RunLengthInfo *ps = 0;
            for (; w < segmentCount[prev_y]; w++) {
                ps = &splane.segment[prev_y][w];



                /*
                   if (y == 14 && cs->color == 4) {
                   test();	
                   cout << "*" << cs->startIndex << "," << cs->endIndex << ")" << endl; 
                   }*/




                if (cs->color == ps->color
                        && ps->overlaps(cs)
                        && ps->blob_number != cs->blob_number) {
                    if (cs->blob_number == -1) {
                        cs->blob_number = ps->blob_number;

                        // also update the sum of x, y and pixles
                        blobinfo[ps->color][ps->blob_number].Update(cs->rawColor,
                                cs->startIndex,
                                cs->endIndex,
                                y,
                                cs->length,
                                cs->xsum,
                                cs->ysum);
                        /*
#ifdef OFFLINE
cout << "new segment joined with segment " << w << " number " << cs->blob_number << " (" << blobinfo[cs->color][cs->blob_number].getXMin();
cout << "," << blobinfo[cs->color][cs->blob_number].getYMin() << ") (";
cout << blobinfo[cs->color][cs->blob_number].getXMax() << ",";
cout << blobinfo[cs->color][cs->blob_number].getYMax() << ")" << endl;
#endif                        
                         */
                    }
                    else {
                        BlobInfo *pblob = &blobinfo[ps->color][ps->blob_number];
                        BlobInfo *cblob = &blobinfo[cs->color][cs->blob_number];
                        int oldnum = cs->blob_number;
                        pblob->Update(cblob);
                        /*
#ifdef OFFLINE
cout << ps->blob_number << " took over " << cs->blob_number << ": (" << pblob->getXMin() << "," << pblob->getYMin();
cout << ") (" << pblob->getXMax() << "," << pblob->getYMax() << ")" << endl;
                        //cout << "update current height - 1 from " << w << " to " << segmentCount[prev_y]-1 << endl;
#endif                        
                         */
                        updateBlobNums(splane.segment[prev_y],
                                w,
                                segmentCount[prev_y] - 1,
                                cs->blob_number,
                                ps->blob_number,
                                ps->color,
                                ps->color);
                        /*
#ifdef OFFLINE
                        //cout << "update current height from 0 to " << x << endl;
#endif                        
                         */
                        updateBlobNums(splane.segment[y],
                                0,
                                x,
                                cs->blob_number,
                                ps->blob_number,
                                ps->color,
                                ps->color);
                        cblob->area = 0;
                    }
                }

                if (ps->endIndex >= cs->endIndex)
                    break;
            }

            // apply these actions no matter the exit condition above
            if (cs->blob_number == -1) {
                createOriginalBlob(cs, y);
                /*
#ifdef OFFLINE
cout << "new " << cs->blob_number << " created " << cs->blob_number << " (";
cout << blobinfo[cs->color][cs->blob_number].getXMin();
cout << "," << blobinfo[cs->color][cs->blob_number].getYMin() << ") (";
cout << blobinfo[cs->color][cs->blob_number].getXMax() << ",";
cout << blobinfo[cs->color][cs->blob_number].getYMax() << ")" << endl;
#endif                        
                 */
            }
            if (ps != 0 && ps->endIndex == cs->endIndex)
                w++;
        }
        updateArray(y);
    }
    updateArray(USABLE_HEIGHT);
}














/* Try to find the first 6 largest blobs of the specified color. Save the
 * pointers to these blobs in hbArray. */
/* input:   colorIndex
 * output:  hbArray 
 * return:  number of halfbeacons found. */
int Blobber::findHalfBeacons(int colorIndex, HalfBeacon **hbArray, int hbArraySize) {
    int i, cur;
    int biggestArea, bigIndex;

    BlobArray binfo;
    binfo = barray[colorIndex];

    /* Examine all the blobs of our interested colour. */
    for (i = 0; i < binfo.getCount() && i < hbArraySize; i++) {
        biggestArea     = -1;
        bigIndex    = -1;

        for (cur = 0; cur < binfo.numCandidates(); cur++) {

            if (!binfo[cur].blobProcessed && binfo[cur].area > biggestArea) {
                biggestArea = binfo[cur].area;
                bigIndex    = cur;
            }
        }

        // should never get here
        if (bigIndex == -1) {
            return 0;
        }

        // now bigIndex is set to the index of the next biggest blob
        hbArray[i] = &binfo[bigIndex];

        // now set the size of the current biggest blob to 0, so we can get
        // the next biggest
        binfo[bigIndex].formationComplete();

        // hbArray[i]->Print();
    }

    // Set the variable so the caller knows how many halfbeacons of this 
    // type we found. 
    // Never return value > hbArraySize so that calling function will not
    // access memory outside of hbArray.

    //return binfo.getCount();
    return MIN(binfo.getCount(), hbArraySize);
}

void Blobber::orderMin(int color, int lo0, int hi0) {
    int lo = lo0;
    int hi = hi0;
    int mid;

    if (hi0 > lo0) {
        //mid = blobinfo[color][( lo0 + hi0 ) / 2].getXMin()*cos_roll + 
        //        blobinfo[color][( lo0 + hi0 ) / 2].getYMin()*sin_roll;
        //mid = blobinfo[color][( lo0 + hi0 ) / 2].boundary[SOUTH].midPoint()*
        //        cos_roll + blobinfo[color][( lo0 + hi0 ) / 2].getYMid()*
        //        sin_roll;
        //mid = blobinfo[color][( lo0 + hi0 ) / 2].getXMin();
        mid = barray[color][(lo0 + hi0) / 2].xRmin;

        while (lo <= hi) {
            /* find the first element that is greater than or equal to
             * the partition element starting from the left Index.
             */
            while ((lo < hi0) && (barray[color][lo].xRmin < mid))
                ++lo;

            /* find an element that is smaller than or equal to
             * the partition element starting from the right Index.
             */
            while ((hi > lo0) && (barray[color][hi].xRmin > mid))
                --hi;

            /* if the indexes have not crossed, swap
             */
            if (lo <= hi) {
                HalfBeacon info = barray[color][lo];
                barray[color][lo] = barray[color][hi];
                barray[color][hi] = info;
                ++lo;
                --hi;
            }
        }

        /* If the right index has not reached the left side of array
         * must now sort the left partition.
         */
        if (lo0 < hi)
            orderMin(color, lo0, hi);

        /* If the left index has not reached the right side of array
         * must now sort the right partition.
         */
        if (lo < hi0)
            orderMin(color, lo, hi0);
    }
}

/* Perform rotation on blobs about the center of the image
 * Only xRmin/max, yRmin/max, cRx/y are changed.
 */
void Blobber::rotate(int color, double sin_eroll, double cos_eroll) {
    int max = barray[color].numCandidates();
    int xOrigin = WIDTH / 2, yOrigin = HEIGHT / 2;

    for (int i = 0; i < max; i++) {
        int nX, nY, eX, eY, sX, sY, wX, wY;
        int rnX, rnY, reX, reY, rsX, rsY, rwX, rwY;

        nX = (xOrigin - barray[color][i].boundary[NORTH].midPoint());
        nY = (yOrigin - barray[color][i].getYMin());

        rnX = (int) (cos_eroll * nX - sin_eroll * nY);
        rnY = (int) (sin_eroll * nX + cos_eroll * nY);

        sX = (xOrigin - barray[color][i].boundary[SOUTH].midPoint());
        sY = (yOrigin - barray[color][i].getYMax());

        rsX = (int) (cos_eroll * sX - sin_eroll * sY);
        rsY = (int) (sin_eroll * sX + cos_eroll * sY);

        eX = (xOrigin - barray[color][i].getXMax());
        eY = (yOrigin - barray[color][i].boundary[EAST].midPoint());

        reX = (int) (cos_eroll * eX - sin_eroll * eY);
        reY = (int) (sin_eroll * eX + cos_eroll * eY);

        wX = (xOrigin - barray[color][i].getXMin());
        wY = (yOrigin - barray[color][i].boundary[WEST].midPoint());

        rwX = (int) (cos_eroll * wX - sin_eroll * wY);
        rwY = (int) (sin_eroll * wX + cos_eroll * wY);

        barray[color][i].xRmin = xOrigin - MAX(MAX(MAX(rnX, rsX), reX), rwX);
        barray[color][i].xRmax = xOrigin - MIN(MIN(MIN(rnX, rsX), reX), rwX);

        barray[color][i].yRmin = yOrigin - MAX(MAX(MAX(rnY, rsY), reY), rwY);
        barray[color][i].yRmax = yOrigin - MIN(MIN(MIN(rnY, rsY), reY), rwY);

        barray[color][i].cRx = (barray[color][i].xRmin + barray[color][i].xRmax)
            / 2;
        barray[color][i].cRy = (barray[color][i].yRmin + barray[color][i].yRmax)
            / 2;
    }
}

void Blobber::saveColorSegment(int column, int start, int end, uchar color)
{
  if((color & COLOR_MASK) < NUM_USABLE_COLORS) {
    int *segmentCount = splane.segmentCount;
    splane.segment[column][segmentCount[column]].Set(column, start, end, color);
    segmentCount[column]++;
  }
}

void Blobber::saveEdgeSegment(int column, int start, int end, 
			      ColorList cl, bool isEdge)
{
    int *segmentCount = esplane.segmentCount;
    esplane.segment[column][segmentCount[column]].Set(column, start, end, cl, isEdge);
    segmentCount[column]++;
}

void Blobber::saveEdgeSegment(int column, int start, int end, 
			      long sumY, long sumU, long sumV, bool isEdge)
{
  //cout << "saveEdgeSegment(" << column << "," << start << "," << end << ",";
  //cout << sumY << "," << sumU << "," << sumV << "," << isEdge << ")" << endl;
    int *segmentCount = esplane.segmentCount;
    int length = end - start + 1;
    uchar cp = nn_multi_colour[(sumY / (length*FACTOR)) * MAXU * MAXV + (sumU / (length*FACTOR)) * MAXV + (sumV / (length*FACTOR))];
    esplane.segment[column][segmentCount[column]].Set(column, start, end, sumY, sumU, sumV, cp, isEdge);
    segmentCount[column]++;
}

void Blobber::updateSegmentLength(int column, int end)
{
  int *segmentCount = esplane.segmentCount;
  esplane.segment[column][segmentCount[column]-1].UpdateLength(column, end);
}

int Blobber::getCrossVal(uchar* p, long row)
{
    uchar* lastRow = p - row;

    return (abs(*(lastRow-1) - *p) + 
            abs(*lastRow - *(p-1)));
}
/*
uchar getColor(int array[])
{
    int max=array[0], i;
    uchar maxc=0, c;

    for(c=1; c<=MAX_COLOR; c++)
        if(max < array[c]) {
            max = array[c];
            maxc = c;
        }
    return maxc;
}

void Blobber::parseSegment(
        int column, int start, int end, uchar* yp, uchar* up, uchar* cp, 
        int lhsColorArray[],
        long rhsSumY, long rhsSumYSqr, long rhsSumU, long rhsSumUSqr
        ) {

    long lhsSumY, lhsSumYSqr, lhsSumU, lhsSumUSqr;
    int lhsLen = 0, rhsLen = end - start + 1;

    int minPos = start;
    double minRSS = VERY_LARGE_DOUBLE; //should be largest possible double value
    long minLhsSumY=0, minLhsSumYSqr=0, minLhsSumU=0, minLhsSumUSqr=0;
    long minRhsSumY=0, minRhsSumYSqr=0, minRhsSumU=0, minRhsSumUSqr=0;
    double minLHSVarY=0, minLHSVarU=0, minRHSVarY=0, minRHSVarU=0;

    //double lhsRSSY=0, lhsRSSU=0, rhsRSSY = 0, rhsRSSU = 0, rss=0;

    int rhsColorArray[MAX_COLOR+1];

    for(int i = 0; i <= MAX_COLOR; i++)
        rhsColorArray[i] = 0;

    lhsColorArray[*cp]--; rhsColorArray[*cp]++;

    lhsSumY = *yp; lhsSumYSqr = *yp * *yp;
    lhsSumU = *up; lhsSumUSqr = *up * *up;

    rhsSumY += *yp; rhsSumYSqr += *yp * *yp;
    rhsSumU += *up; rhsSumUSqr += *up * *up;

    lhsLen++; rhsLen--;

    yp++; up++; cp++;

    for(int i = start+1; i < end; i++) {

        lhsSumY += *yp; lhsSumYSqr += *yp * *yp;
        lhsSumU += *up; lhsSumUSqr += *up * *up;

        rhsSumY -= *yp; rhsSumYSqr -= *yp * *yp;
        rhsSumU -= *up; rhsSumUSqr -= *up * *up;

        lhsLen++; rhsLen--;

        double lhsRSSY = lhsSumYSqr - (double) lhsSumY*lhsSumY/lhsLen;
        double lhsRSSU = lhsSumUSqr - (double) lhsSumU*lhsSumU/lhsLen;

        double rhsRSSY = rhsSumYSqr - (double) rhsSumY*rhsSumY/rhsLen;
        double rhsRSSU = rhsSumUSqr - (double) rhsSumU*rhsSumU/rhsLen;

        double rss = lhsRSSY + lhsRSSU + rhsRSSY + rhsRSSU;

        if(rss < minRSS) {
            minPos = i;
            minRSS = rss;

            minLhsSumY = lhsSumY; minLhsSumYSqr = lhsSumYSqr;
            minLhsSumU = lhsSumU; minLhsSumUSqr = lhsSumUSqr;

            minRhsSumY = rhsSumY; minRhsSumYSqr = rhsSumYSqr;
            minRhsSumU = rhsSumU; minRhsSumUSqr = rhsSumUSqr;

            minLHSVarY = lhsRSSY/lhsLen; minLHSVarU = lhsRSSU/lhsLen;
            minRHSVarY = rhsRSSY/rhsLen; minRHSVarU = rhsRSSU/rhsLen;
        }

        yp++; up++; cp++;
    }

    int n = end - start;
    if(n >= 10 && (minLHSVarY > 15 || minLHSVarU > 15))
        parseSegment(column, start, minPos, yp-n, up-n, cp-n, lhsColorArray, 
                minLhsSumY, minLhsSumYSqr, minLhsSumU, minLhsSumUSqr);
    else
        saveEdgeSegment(column, start, minPos, 14);

    n = end - (minPos + 1);
    if(n >= 10 && (minRHSVarY > 15 || minRHSVarU > 15))
        parseSegment(column, minPos+1, end, yp-n, up-n, cp-n, rhsColorArray, 
                minRhsSumY, minRhsSumYSqr, minRhsSumU, minRhsSumUSqr);
    else
        saveEdgeSegment(column, minPos+1, end, 15);
}

void Blobber::findEdge(uchar *yp, uchar *up, uchar *cp, long skip) {

    bool prevEdge, curEdge;
    int start;
    int yval, uval, threshold = 15;
    int colorArray[MAX_COLOR+1];
    long sumY, sumYSqr, sumU, sumUSqr;

    eplane.segmentCount[0] = 0;
    yp += skip; up += skip; cp += WIDTH;

    sumY = *yp; sumYSqr = *yp * *yp;
    sumU = *up; sumUSqr = *up * *up;

    for(int y=1; y < HEIGHT; y++) {

        start = 0;
        eplane.segmentCount[y] = 0;
        for(int i=0; i<=MAX_COLOR; i++)
            colorArray[i] = 0;

        //first column
        prevEdge = true;
        colorArray[*cp]++;

        sumY = *yp; sumYSqr = *yp * *yp;
        sumU = *up; sumUSqr = *up * *up;

        yp++; up++; cp++;

        for(int x=1; x < WIDTH; x++) {

            yval = getCrossVal(yp, skip+WIDTH);
            uval = getCrossVal(up, skip+WIDTH);
            curEdge = (yval >= threshold || uval >= threshold);

            if((!prevEdge && curEdge) || x == WIDTH - 1) {

                if(x == WIDTH - 1) x++;

                int n = x - start;
                double varY = (double) sumYSqr/n - (double) sumY*sumY/(n*n);
                double varU = (double) sumUSqr/n - (double) sumU*sumU/(n*n);

                if(n > 10 && (varY > 20 || varU > 20))
                    parseSegment(y, start, x-1, yp-n+1, up-n+1, cp-n+1, colorArray, 
                            sumY, sumYSqr, sumU, sumUSqr);
                else
                    saveEdgeSegment(y, start, x-1, getColor(colorArray));

                start = x;
                for(int i=0; i<=MAX_COLOR; i++)
                    colorArray[i] = 0;
            }

            prevEdge = curEdge;
            colorArray[*cp]++;

            sumY += *yp; sumYSqr += *yp * *yp;
            sumU += *up; sumUSqr += *up * *up;

            yp++; up++; cp++;
        }
        yp+=skip; up+=skip;
    }
}
*/
void Blobber::findVerticalEdge(uchar *yplane, uchar *uplane, long skip)
{
  //cout << "findVerticalEdge(";
  //cout << (*yplane & 0xff) << "," << (*uplane  & 0xff) << ")" << endl;;

  uchar* yp = yplane; uchar* up = uplane;
  bool prevEdge, curEdge;
  int start, len;
  int yval, uval, threshold = 20;
  long sumY, sumYSqr, sumU, sumUSqr;

  //skip first column
  for(int x=1; x < WIDTH; x++) {
    //skip first row
    prevEdge = true;
    start = 0;
    sumY = *yp; sumYSqr = *yp * *yp;
    sumU = *up; sumUSqr = *up * *up;
    yp = yplane + skip + x; up = uplane + skip + x;

    for(int y=1; y<HEIGHT; y++) {
      //cout << "(" << y << "," << x;
      //cout << "," << (*yp & 0xff) << "," << (*up & 0xff);

      yval = getCrossVal(yp, skip);
      uval = getCrossVal(up, skip);

      //cout << "," << yval << "," << uval;

      curEdge = (yval >= threshold || uval >= threshold);
      edgePlane[y][x] = curEdge ? 1 : 0;

      //cout << "," << curEdge;
      if(!prevEdge && (curEdge || y == HEIGHT-1)) {
	//cout << ", !prev && curEdge";
	if(y == HEIGHT-1) y++;

	len = y - start;

	if(len <= MIN_OVERLAP_PIXELS) {
	  //cout << ", len <= overlap";
	  //set last segment to edges
	  for(int i = start; i < y; i++)
	    edgePlane[i][x] = 1;
	}
	else if(len > LONG_SEGMENT_LENGTH) {
	  //cout << ", len > long segment";
	  double varY = sumYSqr/len - sumY*sumY/(len*len);
	  double varU = sumUSqr/len - sumU*sumU/(len*len);
	  //cout << "," << varY << "," << varU;
	  if(varY > 50 || varU > 50) {
	    //cout << endl;
	    parseVerticalSegment(start, y, x, yp-(len-1)*skip, up-(len-1)*skip, skip);
	  }
	}
	start = y;
	sumY = 0; sumYSqr = 0;
	sumU = 0; sumUSqr = 0;
      }
      else if(prevEdge && !curEdge) {
	//cout << ", prevEdge && !curEdge";
	if(y==HEIGHT-1)
	  edgePlane[y][x] = 1;
	start = y;
	sumY = 0; sumYSqr = 0;
	sumU = 0; sumUSqr = 0;
      }
      prevEdge = curEdge;
      sumY += *yp; sumYSqr += *yp * *yp;
      sumU += *up; sumUSqr += *up * *up;
      yp += skip; up += skip;
      //cout << ")" << endl;
    }
  }
}

void Blobber::parseVerticalSegment(int start, int end, int x, uchar* yp, uchar* up, long skip)
{
  //cout << "parseVerticalSegment(";
  //cout << start << "," << end << "," << x << ",";
  //cout << (*yp & 0xff) << "," << (*up & 0xff) << "," << skip << ")" << endl;

  int threshold = 15, yval, uval, len;
  bool prevEdge = true, curEdge;

  for(int y=start; y <= end; y++) {
    //cout << "\t(" << y;

    yval = getCrossVal(yp, skip);
    //cout << "," << yval;
    uval = getCrossVal(yp, skip);
    //cout << "," << uval;

    curEdge = (yval >= threshold || uval >= threshold);
    edgePlane[y][x] = curEdge ? 3 : 0;
    //cout << "," << curEdge;

    if(!prevEdge && curEdge) {
      //cout << ", !prevEdge && curEdge";
      len = y - start;

      if(len <= MIN_OVERLAP_PIXELS) {
	//cout << ",len <= overlap";
	//set last segment to edges
	for(int i = start; i < y; i++)
	  edgePlane[i][x] = 3;
      }
      start = y;
    }
    else if(prevEdge && !curEdge) {
      //cout << ",prevEdge && !curEdge";
      start = y;
    }
    prevEdge = curEdge;
    yp += skip; up += skip;
    //cout << ")" << endl;
  }
}

void Blobber::findHorizontalEdge(uchar* yp, uchar *up, uchar* vp, long skip)
{
  //cout << "findHorizontalEdge(";
  //cout << (*yp & 0xff) << "," << (*up & 0xff)  << "," << (*vp & 0xff) << ")" << endl;

  uchar *cp = cplane;
  int cstart, estart, i, len;
  int* segmentCount = splane.segmentCount;
  int* eSegmentCount = esplane.segmentCount;
  long sumY, sumU, sumV, sumYSqr, sumUSqr;

  //skip first row

  segmentCount[0] = 0;
  eSegmentCount[0] = 0;

  yp += skip+WIDTH; up += skip+WIDTH; vp += skip+WIDTH; cp += WIDTH;

  for(int y = 1; y<HEIGHT; y++) {
    cstart = 0; estart = 0;

    segmentCount[y] = 0;
    eSegmentCount[y] = 0;

    //skip first column
    edgePlane[y][0] = 2;
    sumY = *yp; sumYSqr = *yp * *yp;
    sumU = *up; sumUSqr = *up * *up;
    sumV = *vp;

    *cp++ = nn_multi_colour[(*yp++ / FACTOR) * MAXU * MAXV + (*up++ / FACTOR) * MAXV + (*vp++ / FACTOR)];

    for(int x = 1; x < WIDTH; x++) {
      //cout << "(" << y << "," << x;

#ifndef OFFLINE // OffVision's classification already got ring removed.            
      *yp = sliceY[*yp][(xLUT[y][abs(x-cyx)])];
      *up = sliceU[*up][(xLUT[y][abs(x-cux)])];
      *vp = sliceV[*vp][(xLUT[y][abs(x-cvx)])];
#endif // OFFLINE

      *cp = nn_multi_colour[(*yp / FACTOR) * MAXU * MAXV + (*up / FACTOR) * MAXV + (*vp / FACTOR)];

      if (*cp != *(cp - 1) || x == WIDTH - 1) {

	if((i=x) == WIDTH - 1) i++;

	saveColorSegment(y, cstart, i - 1, *(cp - 1));
	cstart = x;  // reset start for segment just started.
      }

      if(!edgePlane[y][x-1] && (edgePlane[y][x] || x==WIDTH-1)) {
	//cout << ",!prevEdge && curEdge" << endl;
	if((i=x) == WIDTH - 1) i++;
	len = i - estart;
	if(len <= MIN_OVERLAP_PIXELS) {
	  //set last segment to edges
	  for(int j=estart; j<x; j++)
	    edgePlane[y][i] = 2;
	  //append last segment to segment before it (should be another edge)
	  //updateSegmentLength(y, i-1);
	  saveEdgeSegment(y, estart, i-1, sumY, sumU, sumV, true);
	}
	else if (len > LONG_SEGMENT_LENGTH) {
	  //cout << ",long" << endl;
	  double varY = sumYSqr/len - sumY*sumY/(len*len);
	  double varU = sumUSqr/len - sumU*sumU/(len*len);
	  //cout << "," << varY << "," << varU << endl;
	  if(varY > 50 || varU > 50)
	    parseHorizontalSegment(y, estart, i-1, yp-(len-1), up-(len-1), vp-(len-1), skip+WIDTH);
	  else
	    saveEdgeSegment(y, estart, i-1, sumY, sumU, sumV, false);
	}
	else {
	  //cout << ",save1" << endl;
	  saveEdgeSegment(y, estart, i-1, sumY, sumU, sumV, false);
	}

	estart = x;
	sumY = 0; sumYSqr = 0;
	sumU = 0; sumUSqr = 0;
	sumV = 0;
      }
      else if(edgePlane[y][x-1] && (!edgePlane[y][x] || x==WIDTH-1)) {
	//cout << ",prevEdge && !curEdge" << endl;
	if((i=x) == WIDTH-1) i++;
	//if segment before last was edge as well then join
	if(eSegmentCount[y] > 0 && esplane.segment[y][eSegmentCount[y]-1].isEdge) {
	  //cout << ",update" << endl;
	  //updateSegmentLength(y, i-1);
	  saveEdgeSegment(y, estart, i-1, sumY, sumU, sumV, true);
	}
	else {
	  //cout << ",save2" << endl;
	  saveEdgeSegment(y, estart, i-1, sumY, sumU, sumV, true);
	}
	estart = x;
 	sumY = 0; sumYSqr = 0;
	sumU = 0; sumUSqr = 0;
	sumV = 0;
     }
      sumY += *yp; sumYSqr += *yp * *yp;
      sumU += *up; sumUSqr += *up * *up;
      sumV += *vp;

      yp++; up++; vp++; cp++;
      //cout << ")" << endl;
    }
    yp+=skip; up+=skip; vp+=skip;
  }
}

void Blobber::parseHorizontalSegment(int y, int start, int end, 
				     uchar* yp, uchar* up, uchar* vp, long skip)
{
  //cout << "parseHorizontalSegment(";
  //cout << y << ","<< start << "," << end << ",";
  //cout << (*yp & 0xff) << "," << (*up & 0xff) << "," << (*vp & 0xff) << ")" << endl;

  int* eSegmentCount = esplane.segmentCount;
  bool prevEdge = false, curEdge;
  int yval, uval, threshold = 15, i, len;
  long sumY=*yp, sumU=*up, sumV=*vp;
  yp++; up++; vp++;
  for(int x=start+1; x<=end; x++) {
    //cout << "\t(" << x << endl;
    yval = getCrossVal(yp, skip);
    uval = getCrossVal(yp, skip);

    curEdge = (yval >= threshold || uval >= threshold);
    edgePlane[y][x] = curEdge ? 4 : 0;

    if(!prevEdge && (curEdge || x==end)) {
      //cout << ",!prevEdge && curEdge" << endl;
      if((i=x) == end) i++;
      len = i - start;
      if(len <= MIN_OVERLAP_PIXELS) {
	//cout << ",overlap" << endl;
	//set last segment to edges
	for(int j=start; j<x; j++)
	  edgePlane[y][i] = 4;
	//append last segment to segment before it (should be another edge)
	//updateSegmentLength(y, i-1);
	saveEdgeSegment(y, start, i-1, sumY, sumU, sumV, true);
      }
      else {
	//cout << ",save" << endl;
	saveEdgeSegment(y, start, i-1, sumY, sumU, sumV, false);
      }

      start = x;
      sumY = 0; sumU = 0; sumV = 0;
    }
    else if(prevEdge && (!curEdge || x==end)) {
      //cout << ",prevEdge && !curEdge" << endl;
      if((i=x) == end) i++;
      //if segment before last was edge as well then join
      if(eSegmentCount[y] > 0 && esplane.segment[y][eSegmentCount[y]-1].isEdge) {
	//cout << ",update" << endl;
	//updateSegmentLength(y, i-1);
	saveEdgeSegment(y, start, i-1, sumY, sumU, sumV, true);
      }
      else {
	//cout << ",save2" << endl;
	saveEdgeSegment(y, start, i-1, sumY, sumU, sumV, true);
      }
      start = x;
      sumY = 0; sumU = 0; sumV = 0;
    }

    sumY += *yp; sumU += *up; sumV += *vp;
    yp++; up++; vp++;
    //cout << ")" << endl;
  }
}

uchar Blobber::dcdY[256];
uchar Blobber::dcdU[256];
uchar Blobber::dcdV[256];

void Blobber::initDCD(const DogColourDistortion &dcd) {
	for (int i=0; i<256; i++) {
		double yd = i * dcd.My + dcd.Cy;
		double ud = i * dcd.Mu + dcd.Cu;
		double vd = i * dcd.Mv + dcd.Cv;

		if (yd < 0) {
			yd = 0;
		} else if (yd > 255) {
			yd = 255;
		}
			
		if (ud < 0) {
			ud = 0;
		} else if (ud > 255) {
			ud = 255;
		}

		if (vd < 0) {
			vd = 0;
		} else if (vd > 255) {
			vd = 255;
		}
		
		dcdY[i] = (uchar)(yd + 0.5);
		dcdU[i] = (uchar)(ud + 0.5);
		dcdV[i] = (uchar)(vd + 0.5);
	}
}

/* Given YUV planes, produce classified CPlane and also store some segment
 * information in splane. 
 */
void Blobber::replaceCplane(uchar *yp, uchar *up, uchar *vp, long skip) {
 
  //findVerticalEdge(yp, up, skip+WIDTH);
  //findHorizontalEdge(yp, up, vp, skip);

    uchar* cp = cplane;

    int x, y;
    int start;

    for (y = 0; y < HEIGHT; y++) {

        start = 0;
        splane.segmentCount[y] = 0;

        for (x = 0; x < WIDTH; x++) {

#ifndef OFFLINE // OffVision's classification already got ring removed.            

            //Remove chromatic distortions (ring) near edge of image 
            *yp = sliceY[*yp][(xLUT[y][abs(x-cyx)])];
            *up = sliceU[*up][(xLUT[y][abs(x-cux)])];
            *vp = sliceV[*vp][(xLUT[y][abs(x-cvx)])];

#endif // OFFLINE

            *cp = nn_multi_colour[(*yp++ / FACTOR) * MAXU * MAXV + (*up++ / FACTOR) * MAXV + (*vp++ / FACTOR)];

            if(x >= 1) {

                // New segment if: 
                // current colour doesn't equal previous colour or
                // is the last pixel of the row.

                //save every segment and not just the usable colours, 
                //let the blobber ignore background colours if it wants to

                if (*cp != *(cp - 1) || x == WIDTH - 1) {

                    if(x == WIDTH - 1) x++;

                    saveColorSegment(y, start, x - 1, *(cp - 1));
                    start = x;  // reset start for segment just started.
                }
            }
            cp++;
        } 
        yp += skip; up += skip; vp += skip;
    }
}

#ifndef OFFLINE
// kim: this is a portable way of getting vision frames using OPENR API, 
// however, offvision cannot use it, so it's no good, remained as a reference

void Blobber::replaceCplane(OFbkImageInfo *info, byte *data) {
    unsigned char c, cc;
    uchar *cp = cplane;
    int *segmentCount = splane.segmentCount;
    int start, end;

    OFbkImage yImage(info, data, ofbkimageBAND_Y);
    OFbkImage crImage(info, data, ofbkimageBAND_Cr);
    OFbkImage cbImage(info, data, ofbkimageBAND_Cb);

    for (int y = 0; y < HEIGHT; ++y) {
        segmentCount[y] = 0;
        start = end = 0;
        for (int x = 0; x < WIDTH; ++x) {
            uchar yvalue = yImage.Pixel(x, y);
            uchar uvalue = crImage.Pixel(x, y);
            uchar vvalue = cbImage.Pixel(x, y);

            c = nn_multi_colour[(yvalue / FACTOR) * MAXU * MAXV + (uvalue / FACTOR) * MAXV + (vvalue / FACTOR)];
            *cp = c;
            start = x;
            for (end = start + 1; end < WIDTH; end++) {
                yvalue = yImage.Pixel(end, y);
                uvalue = crImage.Pixel(end, y);
                vvalue = cbImage.Pixel(end, y);
                cc = nn_multi_colour[(yvalue / FACTOR) * MAXU * MAXV + (uvalue / FACTOR) * MAXV + (vvalue / FACTOR)];
                *(++cp) = cc;
                if (cc != c)
                    break;
            }

            splane.segment[y][segmentCount[y]].Set(start, end - 1, y, c);
            segmentCount[y] += 1;

            x = end - 1;
        }
        (++cp);
    }
}

#endif // OFFLINE
