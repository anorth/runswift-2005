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
 * Like Blob.cc except has extra edgePlane
 * Requires more testing
 * Link is buggy - others should be fine.
 **/

#include "Blob.h"
#include "ring.h"
#include <sys/time.h>

static bool bDebugBlob = false;

Blobber::Blobber(GeneralisedColourDetect &gcd) {
    cplane = gcd.cplane;
    nn_multi_colour = gcd.nn_multi_colour;
    /*
    for (int i = 0; i < NUM_USABLE_COLORS; i++) {
        blobs[i].init(i);
    }
    */
    for (int i = 0; i < NUM_USABLE_COLORS; i++) {
        barray[i].init(i); //reset everything
    }

    usingHeap = 0;
    usingAlt = 1;
    multi_colour = 0;
    /*
    for (int k = 0; k < NUM_USABLE_COLORS; k++) {
        nbob[k] = 0;
    }
    */
    top_unused_bn = 0;
    top_reusable_bn = -1;
}

void Blobber::createBlob(int y, RunLengthInfo *segment) {
    int i, line = -1;

    if(y == line)
      cout << "createBlob(";

    if (top_reusable_bn >= 0) { //There are reusable blob numbers
      i = reusable_bn[top_reusable_bn--];
      if(y == line)
	cout << "reused";
    }
    else {
      // Get the next unused blob number
      i = top_unused_bn++;
      if(y == line)
	cout << "unused";
    }

    if (i >= MAX_NUM_BLOBS_ALL_COLORS){
        if (bDebugBlob)
            cout << "Max num blobs exceeded set i = 0" << endl;
        i = 0;
    }

    if(y == line) {
      cout << "," << segment->row << "," << i << "," << (segment->color & 0xff);
      cout << "," << segment->startIndex << "," << segment->endIndex << "," << segment->length;
      }

    if (i < MAX_NUM_BLOBS_ALL_COLORS) {
        segment->rank = 0;
        segment->blob_number = i;
        segment->nextSeg = segment;
        blobinfo[i].Init(segment, y);

	if(y == line)
	  cout << ",saved";
    }
    else {
      if(y == line)
        cout << "Running out of blob number!, i = " << i << " and colour = " << segment->color << " and y = " << y << endl;
    }
    if(y == line)
      cout << ")" << endl;
}

// Find root of set
RunLengthInfo * Find_Set(RunLengthInfo *cs) {

    if (cs->nextSeg == NULL) {
        cout << "Error, the parent pointer should pointing to something!" << endl;
        return NULL;
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

//This function is buggy
void Blobber::Link(int y, RunLengthInfo *cs, RunLengthInfo *ps) {

  RunLengthInfo *smallTree, *bigTree, *smallTreeRoot, *bigTreeRoot;
  HalfBeacon *smallTreeBlob, *bigTreeBlob;

  if (cs->rank == -1 || ps->rank == -1) { //Should never happen
    cout << "Error, both x and y should have rank at least 0!" << endl;
    return;
  }

  if(cs->blob_number == -1 || cs->rank <= ps->rank) {
    smallTree = cs; bigTree = ps;
  }
  else {
    smallTree = ps; bigTree = cs;
  }

  smallTreeRoot = Find_Set(smallTree);
  bigTreeRoot = Find_Set(bigTree);
  bigTreeBlob = &blobinfo[bigTreeRoot->blob_number];

  if(smallTreeRoot->blob_number != -1 && 
     smallTreeRoot->blob_number != bigTreeRoot->blob_number) {

    //cout << "Link deallocating bn: " << smallTree->blob_number << endl;
    smallTreeBlob = &blobinfo[smallTree->blob_number];

    reusable_bn[++top_reusable_bn] = smallTree->blob_number;
    bigTreeBlob->Update(smallTreeBlob);
    smallTreeBlob->ResetBlob();

  }
  else {
    bigTreeBlob->Update(smallTree, y);
  }

  smallTree->blob_number = bigTree->blob_number;
  smallTree->nextSeg = bigTree;

  if(smallTree->rank == bigTree->rank)
    bigTree->rank++;
}

void Blobber::testSaveBlob(int y, RunLengthInfo *ps)
{
  RunLengthInfo* psRoot = Find_Set(ps);
  HalfBeacon& pblob = blobinfo[psRoot->blob_number];
  int line = -1;

  if(y == line) {
    cout << "testSaveBlob(";
    cout << ps->row;
    cout << "," << ps->blob_number << "," << (ps->color & 0xff);
    cout << "," << ps->startIndex << "," << ps->endIndex;
    cout << "," << ps->length;
    cout << "," << pblob.large << "," << pblob.getYMax();
    cout << "," << (pblob.getYMax() < y && ps->endIndex == pblob.large);
  }

  if(pblob.getYMax() < y && ps->endIndex == pblob.large) {
    /****do any final find color****/
    if(!barray[psRoot->color].add(pblob)) {
      //if (bDebugBlob)
      if(y == line)
	cout << "barray rejected blob : " << psRoot->blob_number << " color : " << psRoot->color << endl;
      // Deallocate the blob number regardless pblob has been successfully added to the barray or not.				
      reusable_bn[++top_reusable_bn] = psRoot->blob_number;
      pblob.ResetBlob();
    }
    else {

      if(y == line) {
      cout << "testSaveBlob(";
      cout << ps->row;
      cout << "," << ps->blob_number << "," << (ps->color & 0xff);
      cout << ",saved";
      cout << "[" << pblob.getXMin() << "," << pblob.getXMax() << ",";
      cout << pblob.getYMin() << "," << pblob.getYMax() << "]";
      }
    }
  }
  if(y == line)
    cout << ")" << endl;
}

void Blobber::joinSegments(int y, RunLengthInfo *cs, RunLengthInfo *ps)
{
  RunLengthInfo *csRoot, *psRoot;
  HalfBeacon* pblob;
  int line = -1;
  /*
  if(cs->row = 15) {
    cout << "\tjoinSegments(" << y << "," << (cs->color & 0xff);
    cout << "," << cs->startIndex << "," << cs->endIndex << endl;
  }
  */
  if(cs->nextSeg == NULL) //should've been done in init?
    cs->nextSeg = cs;

  csRoot = Find_Set(cs);
  psRoot = Find_Set(ps);
  pblob = &blobinfo[psRoot->blob_number];

  if(y==line) {
    cout << "\tjoinSegments(";
    cout << y << "," << (cs->color & 0xff) << "," << (ps->color & 0xff);
    cout << "," << cs->startIndex << "," << cs->endIndex;
    cout << "," << ps->startIndex << "," << ps->endIndex;
    cout << ";" << pblob->blob_number;
    cout << ";" << (csRoot->color & 0xff) << "," << (psRoot->color & 0xff);
    cout << "," << csRoot->startIndex << "," << csRoot->endIndex;
    cout << "," << psRoot->startIndex << "," << psRoot->endIndex;
    cout << ")" << endl;
  }

  if(cs->shouldJoin(ps)) {
    if(y==line)
      cout << "join";
    Link(y, csRoot, psRoot);
  }
  else { //pass pblob to barray
    if(y==line)
      cout << "not join";
    if(cs->endIndex > pblob->large) {
      //cout << "," << y << "," << pblob->getYMax();
      //cout << "," << cs->endIndex << "," << pblob->large;
      //cout << ",pass" << endl;
      testSaveBlob(y,ps);
    }
  }
  //cout << ")" << endl;
}

void Blobber::test() {}

void Blobber::formDisjoint(SegmentedPlane& segmentPlane, bool ignoreBkg)
{
  int* segmentCount = segmentPlane.segmentCount;
  RunLengthInfo *ps, *cs, *nextps, *psRoot, *csRoot;
  HalfBeacon* pblob;
  int line = -1;

  //second row
  for(int y=1, x=0; x < segmentCount[y]; x++) {
    cs = &segmentPlane.segment[y][x];
    if(ignoreBkg && cs->color >= NUM_USABLE_COLORS)
      continue;
    createBlob(y, cs);
  }

  //rest of rows
  for(int py=1, y=py+1, px=0; y < USABLE_HEIGHT; y++, py++) {
    px = 0;
    for(int x=0; x < segmentCount[y]; x++) {

      cs = &segmentPlane.segment[y][x];
      if(ignoreBkg && cs->color >= NUM_USABLE_COLORS)
	continue;

      if(y == line) {
	cout << "ya: " << y << " x: " << x << " px: " << px << endl;
	cout << "top_unused_bn: " << top_unused_bn << endl;
      }

      for(; px < segmentCount[py]; px++) {

	if(y == line)
	  cout << "yb: " << y << " x: " << x << " px: " << px << endl;

	ps = &segmentPlane.segment[py][px];
	if(ignoreBkg && ps->color >= NUM_USABLE_COLORS)
	  continue;

	//cout << "join1" << endl;
	joinSegments(y, cs, ps);

	if(ps->endIndex >= cs->endIndex) {

	  if(ps->endIndex == cs->endIndex && px+1 < segmentCount[py]) {

	    nextps = &segmentPlane.segment[py][px+1];
	    if(ignoreBkg && nextps->color >= NUM_USABLE_COLORS)
	      continue;

	    //cout << "join2" << endl;
	    joinSegments(y, cs, nextps);
	  }
	  else {
	    if(y == line)
	      cout << " increment";
	    //px++;
	  }

	  if(y == line)
	    cout << " break";
	  break;
	}
      }

      if(y == line)
	cout << "yc: " << y << " x: " << x << " px: " << px << endl;

      if(cs->blob_number == -1)
	createBlob(y, cs);
    }

    for(; px < segmentCount[py]; px++) {

      ps = &segmentPlane.segment[py][px];
      if(ignoreBkg && ps->color >= NUM_USABLE_COLORS)
	continue;
      //cout << "test: " << py << "," << px << "," << (ps->color & 0xff) << " ";
      testSaveBlob(y, ps);
    }
    if(y == line)
      cout << endl;
  }

  //pass all blobs in the last row to barray
  for(int y=USABLE_HEIGHT-1, x=0; x < segmentCount[y]; x++) {

    ps = &segmentPlane.segment[y][x];

    if(ignoreBkg && ps->color >= NUM_USABLE_COLORS)
      continue;

    testSaveBlob(y+1, ps);
  }
  /*
  for(int i=0; i<top_unused_bn; i++) {
    HalfBeacon &pblob = blobinfo[i];
    cout << "blobs: " << i << "," << (pblob.color & 0xff);
    cout << " [" << pblob.getXMin() << "," << pblob.getXMax() << ",";
    cout << pblob.getYMin() << "," << pblob.getYMax() << "," << pblob.area << "]" << endl;
  }

  cout << "reusable_bn:";
  for(int i=0; i<top_reusable_bn; i++) {
    cout << " " << i;
  }
  cout << endl;

  for(int i=0; i<NUM_USABLE_COLORS; i++) {
    for(int j=0; j<barray[i].size; j++) {
      HalfBeacon *pblob = barray[i].array[j];
      cout << "barray: " << i << "," << j;
      cout << " [" << pblob->getXMin() << "," << pblob->getXMax() << ",";
      cout << pblob->getYMin() << "," << pblob->getYMax() << "," << pblob->area << "]" << endl;
    }
  }
  */
}

void Blobber::formBlobs() {

    formDisjoint(splane, true);

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

//not used yet
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
	  updateSegmentLength(y, i-1);
	  //saveEdgeSegment(y, estart, i-1, sumY, sumU, sumV, true);
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
	  updateSegmentLength(y, i-1);
	  //saveEdgeSegment(y, estart, i-1, sumY, sumU, sumV, true);
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
	updateSegmentLength(y, i-1);
	//saveEdgeSegment(y, start, i-1, sumY, sumU, sumV, true);
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
	updateSegmentLength(y, i-1);
	//saveEdgeSegment(y, start, i-1, sumY, sumU, sumV, true);
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

