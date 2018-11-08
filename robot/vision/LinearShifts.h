/* Copyright 2003 The University of New South Wales (UNSW) and National  
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
 * $Id: VisualCortex.h 1953 2003-08-21 03:51:39Z eileenm $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 *
 * Object identification from blobs
 * 
 **/

#include "CorrectedImage.h"

using namespace std;

struct DogMacLookupType {
    unsigned long mac;
    DogColourDistortion colMunge;
};

// table that came with us from sydney 11/7/05
// // Lookup table for dog-specific vision calibration values
// // to generate these see base/colour/linearShifts/getDogShifts.java
// DogMacLookupType DogMacLookup[] = {
//     // Default Entry (Base Dog = 0xC6245478)
//     {0, {1.0, 0.0, 1.0, 0.0, 1.0, 0.0}} ,    
//     {0xc6245466, {1.0067, -2.8731, 0.9786, 0.2309, 1.0027, -2.4957}},
//     {0xc6245457, {1.022, -3.5762, 1.0004, -1.1732, 1.0188, -6.9349}},
//     {0xc624544b, {0.9746, -4.9311, 0.9549, 5.0885, 0.9961, -5.7398}},
//     {0xc6245458, {1.0017, -1.7996, 1.0174, -0.6574, 1.0057, -4.226}},
//     {0xc6245473, {0.9965, -3.5624, 0.9788, 4.683, 0.9777, 0.7503}},
//     {0xc6244f15, {1.0293, -0.6547, 1.0525, -5.7608, 1.0217, -4.5393}},
//     {0xc6244d35, {0.9315, -0.3235, 0.898, 10.0886, 0.9687, 5.5699}},
//     {0xc62450cd, {0.9965, -3.4048, 0.9798, 1.729, 0.9887, -2.1301}},
//     {0xc62450d7, {0.9589, -2.6635, 0.9732, 4.261, 0.9773, -0.9882}},
//     {0xc6567d24, {0.9464, -1.4007, 0.9261, 7.3307, 0.9584, 4.8831}},
//     {0xc6245481, {0.9658, 0.194, 0.9577, 3.4535, 0.9886, -0.1586}},
//     {0xc6740fac, {0.9713, -4.992, 0.9706, 3.795, 1.0014, -2.8015}},
//     {0xc66b055c, {0.9985, -1.2418, 0.984, 3.0973, 0.9643, 4.6924}},
//     {0xc66b0561, {0.965, -1.8434, 0.9664, 4.8717, 0.9928, 0.4975}},
//     {0xc66aff42, {0.9509, -2.0989, 0.9715, 5.2192, 0.9769, 1.8232}},
//     {0xc66aff3a, {0.9799, -4.6747, 0.942, 7.0132, 0.9942, -1.3763}}
//     
// };

// new table hacked by base/colour/linearShifts/reconfigureShifts
DogMacLookupType DogMacLookup[] = {
    // Default Entry (Base Dog = 0xc66aff3a)
    {0, {1.0, 0.0, 1.0, 0.0, 1.0, 0.0}} ,
    {0xc6245466, {1.02735, 1.83855, 1.03885, -7.19989, 1.00855, -1.12593}},
    {0xc6245457, {1.04296, 1.12103, 1.062, -8.69045, 1.02474, -5.59103}},
    {0xc624544b, {0.994591, -0.261659, 1.01369, -2.04321, 1.00191, -4.38896}},
    {0xc6245458, {1.02225, 2.93407, 1.08004, -8.14289, 1.01157, -2.86632}},
    {0xc6245473, {1.01694, 1.13512, 1.03907, -2.47367, 0.983404, 2.13901}},
    {0xc6244f15, {1.05041, 4.10246, 1.1173, -13.5605, 1.02766, -3.18145}},
    {0xc6244d35, {0.950607, 4.44045, 0.953291, 3.26476, 0.974351, 6.98672}},
    {0xc62450cd, {1.01694, 1.29595, 1.04013, -5.60955, 0.994468, -0.758198}},
    {0xc62450d7, {0.978569, 2.05245, 1.03312, -2.92166, 0.983001, 0.390364}},
    {0xc6567d24, {0.965813, 3.34116, 0.983121, 0.337049, 0.963991, 6.29592}},
    {0xc6245481, {0.985611, 4.96857, 1.01667, -3.77887, 0.994367, 1.2248}},
    {0xc6740fac, {0.991224, -0.323809, 1.03036, -3.41635, 1.00724, -1.43351}},
    {0xc66b055c, {1.01898, 3.50332, 1.04459, -4.15701, 0.969926, 6.1041}},
    {0xc66b0561, {0.984794, 2.88938, 1.0259, -2.27335, 0.998592, 1.88473}},
    {0xc66aff42, {0.970405, 2.62864, 1.03132, -1.90446, 0.982599, 3.21817}},
    {0xc6245478, {1.02051, 4.77059, 1.06157, -7.44501, 1.00583, 1.38433}}
};

// zero table for finding new linear shifts
//DogMacLookupType DogMacLookup[] = {
//    {0, {1.0, 0.0, 1.0, 0.0, 1.0, 0.0}}
//};

