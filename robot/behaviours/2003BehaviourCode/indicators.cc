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
 * Last modification background information
 * $Id: indicators.cc 2234 2004-01-20 23:22:27Z paulooi $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 * 
 * Indicators tester
 *
 **/ 

#include "indicators.h"
#include <string>
#include <list>

namespace Indicators {
	static const char separator = ':';
}

void Indicators::initIndicatorsTester() {
}

void Indicators::parseInput(string input) {
	int index;

	if ((index = input.find(separator)) != -1) {
		string value = input.substr(index + 1, input.size() - index - 1);
		input.erase(index, input.size() - index);

		int intValue = atoi(value.c_str());

		if (input == "mouth")
			mouth = intValue;
		else if (input == "tail_h")
			tail_h = intValue;
		else if (input == "tail_v")
			tail_v = intValue;
		else if (input == "ear_l")
			ear_l = intValue;
		else if (input == "ear_r")
			ear_r = intValue;
		else if (input == "mode_led1")
			mode_led1 = intValue;
		else if (input == "mode_led2")
			mode_led2 = intValue;
		else if (input == "mode_led3")
			mode_led3 = intValue;
		else if (input == "mode_led4")
			mode_led4 = intValue;
		else if (input == "mode_led5")
			mode_led5 = intValue;
		else if (input == "mode_led6")
			mode_led6 = intValue;
		else if (input == "mode_led7")
			mode_led7 = intValue;
		else if (input == "mode_led8")
			mode_led8 = intValue;
		else if (input == "mode_led9")
			mode_led9 = intValue;
		else if (input == "mode_led10")
			mode_led10 = intValue;
		else if (input == "mode_led11")
			mode_led11 = intValue;
		else if (input == "mode_led12")
			mode_led12 = intValue;
		else if (input == "mode_led13")
			mode_led13 = intValue;
		else if (input == "mode_led14")
			mode_led14 = intValue;
		else if (input == "led1")
			led1 = intValue;
		else if (input == "led2")
			led2 = intValue;
		else if (input == "led3")
			led3 = intValue;
		else if (input == "led4")
			led4 = intValue;
		else if (input == "led5")
			led5 = intValue;
		else if (input == "led6")
			led6 = intValue;
		else if (input == "led7")
			led7 = intValue;
		else if (input == "led8")
			led8 = intValue;
		else if (input == "led9")
			led9 = intValue;
		else if (input == "led10")
			led10 = intValue;
		else if (input == "led11")
			led11 = intValue;
		else if (input == "led12")
			led12 = intValue;
		else if (input == "led13")
			led13 = intValue;
		else if (input == "led14")
			led14 = intValue;
		else if (input == "head_color")
			head_color = intValue;
		else if (input == "head_white")
			head_white = intValue;
		else if (input == "mode_red")
			mode_red = intValue;
		else if (input == "mode_green")
			mode_green = intValue;
		else if (input == "mode_blue")
			mode_blue = intValue;
		else if (input == "wireless")
			wireless = intValue;
		else if (input == "back_front_color")
			back_front_color = intValue;
		else if (input == "back_front_white")
			back_front_white = intValue;
		else if (input == "back_middle_color")
			back_middle_color = intValue;
		else if (input == "back_middle_white")
			back_middle_white = intValue;
		else if (input == "back_rear_color")
			back_rear_color = intValue;
		else if (input == "back_rear_white")
			back_rear_white = intValue;

		cout << "Value for indicator " << input << " changed to " << intValue
			<< "." << endl;
	}
}

void Indicators::doIndicatorsTester() {
	/*
	   led1 = led2 = led3 = led4 = led5 = led6 = led7 = IND_LED3_INTENSITY_MAX;
	   led8 = led9 = led10 = led11 = led12 = led13 = led14 = IND_LED3_INTENSITY_MAX;
	   head_color = IND_LED2_ON;
	   head_white = IND_LED2_OFF;
	   mode_red   = IND_LED2_ON;
	   mode_green = IND_LED2_OFF;
	   mode_blue  = IND_LED2_ON;
	   back_front_color = IND_LED3_INTENSITY_MAX;
	   back_front_white = IND_LED3_INTENSITY_MIN;
	   back_middle_color = IND_LED3_INTENSITY_MIN; 
	   back_middle_white = IND_LED3_INTENSITY_MAX;
	   back_rear_color = IND_LED3_INTENSITY_MAX;
	   back_rear_white = IND_LED3_INTENSITY_MIN;
	   return;
	   */

	headtype = ABS_H;
	panx = tilty = cranez = 0;

	cout << "------------------------" << endl;
	cout << "Current indicator values" << endl;
	cout << "------------------------" << endl; 

	cout << "mouth: " << mouth << endl;

	cout << "tail_h: " << tail_h << "\t" << "tail_v: " << tail_v << endl;
	cout << "ear_l: " << ear_l << "\t" << "ear_r: " << ear_r << endl;

	cout << "mode_led1:  " << mode_led1 << "\t" << "mode_led2:  " << mode_led2
		<< "\t" << "mode_led3:  " << mode_led3 << "\t" << "mode_led4:  "
		<< mode_led4 << endl;
	cout << "mode_led5:  " << mode_led5 << "\t" << "mode_led6:  " << mode_led6
		<< "\t" << "mode_led7:  " << mode_led7 << "\t" << "mode_led8:  "
		<< mode_led8 << endl;
	cout << "mode_led9:  " << mode_led9 << "\t" << "mode_led10: " << mode_led10
		<< "\t" << "mode_led11: " << mode_led11 << "\t" << "mode_led12: "
		<< mode_led12 << endl;
	cout << "mode_led13: " << mode_led13 << "\t" << "mode_led14: " << mode_led14
		<< endl; 

	cout << "led1:  " << led1 << "\t" << "led2:  " << led2 << "\t" << "led3:  "
		<< led3 << "\t" << "led4:  " << led4 << endl;
	cout << "led5:  " << led5 << "\t" << "led6:  " << led6 << "\t" << "led7:  "
		<< led7 << "\t" << "led8:  " << led8 << endl;
	cout << "led9:  " << led9 << "\t" << "led10: " << led10 << "\t" << "led11: "
		<< led11 << "\t" << "led12: " << led12 << endl;
	cout << "led13: " << led13 << "\t" << "led14: " << led14 << endl; 

	cout << "head_color: " << head_color << "\t" << "head_white: " << head_white
		<< endl;

	cout << "mode_red:   " << mode_red << endl;
	cout << "mode_green: " << mode_green << endl;
	cout << "mode_blue:  " << mode_blue << endl;

	cout << "wireless: " << wireless << endl;

	cout << "back_front_color: " << back_front_color << "\t"
		<< "back_front_white: " << back_front_white << endl;
	cout << "back_middle_color: " << back_middle_color << "\t"
		<< "back_middle_white: " << back_middle_white << endl;
	cout << "back_rear_color: " << back_rear_color << "\t"
		<< "back_rear_white: " << back_rear_white << endl;

	string input;

	while (input != "done") {
		cout << "[paulooi@Dogmatix: ~]$ ";
		cin >> input;
		parseInput(input);
	}
}

