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
 * Last modification background information
 * $Id: stlCommon.h 4653 2005-01-18 06:27:35Z weiming $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 *
 * contains basic utilities and functions that involve the standard template
 * library
 * 
 **/

#ifndef _STLCommon_h_DEFINED
#define _STLCommon_h_DEFINED

#include <list>
#include <iterator>
#include <stdexcept>

using std::list;
using std::domain_error;



//====================================================================
//A* search template function
//====================================================================
// note - heuristic is assumed to include cost up to current point
// heuristic must be admissible (ie - underestimate) for optimal solution
template <class T>
T aStarSearch(const T &initial,
		bool (*goalTest) (const T &),
		double (*heuristic) (const T &),
		void (*expandNode) (const T&, list<T> &)) {
	list<T> tree(1, initial);
	while (true) {
		if (tree.empty())
			throw domain_error("impossible A* search");
		typename list<T>::iterator iter = tree.begin();
		if (goalTest(*iter))
			return *iter;
		list<T> nextNodes;
		expandNode(*iter, nextNodes);
		tree.pop_front();
		typename list<T>::iterator newNode = nextNodes.begin();
		while (newNode != nextNodes.end()) {
			iter = tree.begin();
			double insHeuristic = heuristic(*iter);
			while (iter != tree.end()) {
				double curHeuristic = heuristic(*iter);
				if (insHeuristic < curHeuristic) {
					tree.insert(iter, *newNode);
					break;
				}
				iter++;
			}
			if (iter == tree.end())
				tree.push_back(*newNode);
			newNode++;
		}
	}
}


#endif // _STLCommon_h_DEFINED
