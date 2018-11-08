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

/**
 * Title: readJoystick
 * Description: reads joystick input, scales it down to acceptable RoboCommander inputs
 *              and sends it via client
 * Copyright: Copyright (c) 2003
 * Company:
 * @author UNSW 2003 Robocup (Terry Tam)
 * @version 1.0
 */

#ifdef JOYSTICK_CONTROL_D

// joystick includes
#include "stdio.h"
#include "fcntl.h"
#include "unistd.h"
#include "sys/ioctl.h"
#include "linux/joystick.h"

// general includes
#include "stdlib.h"
#include "pthread.h"
#include "sys/timeb.h"


#include <iostream>

// joystick defines
#define JOY_DEV "/dev/js0"
#define JOYSTICK_KEY "j"
#define NUM_BUTTONS 15
#define NUM_SLIDERS 3

// debug defines
//#define STAND_ALONE_APP_D
//#define AXIS_DEBUG_D
//#define BUTTON_DEBUG_D
//#define SEND_DEBUG_D
//#define INFLEX_DEBUG_D
//#define OUTPUT_MSG_D

// function prototypes
void processAxes(int *);
void processButtons(char *);
void sendAxisData(int, int, int);
void sendButtonData(int);
int hasRemainderOne(int, int);
void reduceDataFrequency();
void reduceAxisSpeeds();
void scale_x_axis(int);
void scale_y_axis(int);
void scale_z_axis(int);
int multipleButtons(char *);
void logAxes(char *);
void logButtons(int);

#ifndef STAND_ALONE_APP_D
extern void sendRobotCommand(char [], char []);
#endif

// global variables
int this_x = 0, this_y = 0, this_z = 0; // current axis data
int old_x = 0, old_y = 0, old_z = 0;  // axis data last sent
int this_button = 0; // current button data
int old_button = -1; // button data last sent
clock_t start_time;
clock_t lastButtonPress_t;
struct timeb tp;
FILE *f;

int xlastRead1 = 0;
int xlastRead2 = 0;
int xlastRead3 = 0;
int ylastRead1 = 0;
int ylastRead2 = 0;
int ylastRead3 = 0;
int zlastRead1 = 0;
int zlastRead2 = 0;
int zlastRead3 = 0;

// main function
#ifdef STAND_ALONE_APP_D
int main() {
#else
void* readJoystick(void *) {
#endif
    int joy_fd, *axis=NULL, num_of_axis=0, num_of_buttons=0;
    char *button=NULL, name_of_joystick[80];
    struct js_event js;

    if((joy_fd = open(JOY_DEV, O_RDONLY)) == -1) {
        #ifdef OUTPUT_MSG_D
	    printf("readJoystick.c: Couldn't open joystick\n");
        #endif
        #ifdef STAND_ALONE_APP_D
        return -1;
        #else
        pthread_exit(0);
        #endif
    }


    // read joystick info
    ioctl(joy_fd, JSIOCGAXES, &num_of_axis);
    ioctl(joy_fd, JSIOCGBUTTONS, &num_of_buttons);
    ioctl(joy_fd, JSIOCGNAME(80), &name_of_joystick);

    // store axis and button data
    axis = (int *) calloc(num_of_axis, sizeof(int));
    button = (char *) calloc(num_of_buttons, sizeof(char));

    #ifdef OUTPUT_MSG_D
        printf("Joystick detected: %s\n\t%d axis\n\t%d buttons\n\n"
          , name_of_joystick
          , num_of_axis
          , num_of_buttons);
    #endif

    fcntl(joy_fd, F_SETFL, O_NONBLOCK);  // use non-blocking mode

    while(1) { // infinite loop
 
	// read the joystick state
	read(joy_fd, &js, sizeof(struct js_event));

	// see what to do with the event
	switch (js.type & ~JS_EVENT_INIT) {
	    case JS_EVENT_AXIS:
        	axis[js.number] = js.value;
        	break;
	    case JS_EVENT_BUTTON:
        	button[js.number] = js.value;
        	break;
	}

    processButtons(button);
    processAxes(axis);

	// if this data is same as the old, keep reading
	if ((old_x == this_x) && (old_y == this_y) && (old_z == this_z)) { 
	    continue;
	}

	// keep a copy of this data
	old_x = this_x;
	old_y = this_y;
	old_z = this_z;
	old_button = this_button;

	#ifdef SEND_DEBUG_D    
	printf("Axes X: %6d Y: %6d Z: %6d\n", this_x, this_y, this_z);
	#endif

	#ifndef STAND_ALONE_APP_D
	sendAxisData(this_x, this_y, this_z);        
	#endif
    }
    fclose(f);
    close(joy_fd);
    return 0;
}

// sends joystick axis data via Client.c's sendRobotCommand function
void sendAxisData(int x, int y, int z) {
    char name[1];  // e.g. 'j'
    char value[9]; // e.g. '-8 -8 -21'
    sprintf(name, JOYSTICK_KEY);
	sprintf(value, "%d %d %d", x, y, z);

    #ifndef STAND_ALONE_APP_D
    logAxes(value);
    sendRobotCommand(name, value);
    #endif
}

// prints out joystick data to be logged
void logAxes(char *data) {
    ftime(&tp);
    if (tp.millitm < 10) {
  	    printf("#%ld00%d:%s\n",tp.time,tp.millitm, data);
    } else if (tp.millitm < 100) {
	    printf("#%ld0%d:%s\n",tp.time,tp.millitm, data);
    } else {
	    printf("#%ld%d:%s\n",tp.time,tp.millitm, data);
    }
    std::cout.flush();
}

// checks the button array to see if more than 1 button is being pressed
int multipleButtons(char *button) {
    int b = 0;
    int numPressed = 0;
    // check each of the buttons except for the sliders
    for (b=0; b < NUM_BUTTONS - NUM_SLIDERS; b++) {
        if (button[b]) {
	        numPressed++;
            if (numPressed > 1) {
		        return 1;
	        }
   	    }
    }
    return 0;
}

// reads which button is pressed, and sends a command
void processButtons(char *button) {

    // set delay in CLOCKS_PER_SEC between successive button presses
    // higher delay used to avoid congesting network with commands due
    // to multiple button bashing
    double num = 0.3;
    int b = 0;

    // ignore button presses if too soon after the last button press
    if ((clock() - lastButtonPress_t) < (num * CLOCKS_PER_SEC)) {
        return;
    }

    // if a button has been pressed before, check to see if it has been released
    // if it has, then reset the old_button variable, so that if the same button
    // is pressed again, the command will be resent
    if (old_button != -1) {
        if (!button[old_button]) {
	        old_button = -1;
   	    }
    }

    // ignore buttons if more than 1 button pressed simultaneously
    if (multipleButtons(button)) {
        return;
    }

    // check all buttons (except sliders) to see if they have been pressed
    for (b=0; b < NUM_BUTTONS - NUM_SLIDERS; b++) {
        if (button[b]) {
            this_button = b;

            // if button command same as the last, don't send
	        if (old_button == this_button) {
	            return;
	        }

  	        #ifdef BUTTON_DEBUG_D 
            printf("Button %d pressed, ", this_button);
            #endif

            #ifndef STAND_ALONE_APP_D
            sendButtonData(this_button);        
	        #endif

            // keep track of the time the last button was pressed
            lastButtonPress_t = clock();

            break; // used so that only the first detected button press is executed
	               // i.e. simultaneous button presses are ignored      	    
        }
    }
}

// filters inputs by sampling 3 consecutive inputs, and only leaving inputs which
// are inflexion points:
// e.g        _                __     __
//     \_/ , / \ , __/ , \__ ,   \ , /
//      1     2     3     4     5     6
int checkInflexions(int lastRead3, int lastRead2, int lastRead1, int type) {
//    printf ("===== %d %d %d ======\n", lastRead3, lastRead2, lastRead1);

    if (lastRead3 < lastRead2 && lastRead2 > lastRead1) {
        #ifdef INFLEX_DEBUG_D
            printf("^ inflexion found (%d %d %d) = %d\n", lastRead3, lastRead2, lastRead1, lastRead2);
        #endif
        return lastRead2;
    }
    if (lastRead3 > lastRead2 && lastRead2 < lastRead1) {
        #ifdef INFLEX_DEBUG_D
            printf("v inflexion found (%d %d %d) = %d\n", lastRead3, lastRead2, lastRead1, lastRead2);
        #endif
        return lastRead2;
    }
    if (lastRead3 == lastRead2 && lastRead2 < lastRead1) {
        #ifdef INFLEX_DEBUG_D
            printf("_] inflexion found (%d %d %d) = %d\n", lastRead3, lastRead2, lastRead1, lastRead2);
        #endif
        return lastRead2;
    }
    if (lastRead3 > lastRead2 && lastRead2 == lastRead1) {
        #ifdef INFLEX_DEBUG_D
            printf("[_ inflexion found (%d %d %d) = %d\n", lastRead3, lastRead2, lastRead1, lastRead2);
        #endif
        return lastRead2;
    }
    if (lastRead3 == lastRead2 && lastRead2 > lastRead1) {
        #ifdef INFLEX_DEBUG_D
            printf("-] inflexion found (%d %d %d) = %d\n", lastRead3, lastRead2, lastRead1, lastRead2);
        #endif
        return lastRead2;
    }
    if (lastRead3 < lastRead2 && lastRead2 == lastRead1) {
        #ifdef INFLEX_DEBUG_D
            printf("[- inflexion found (%d %d %d) = %d\n", lastRead3, lastRead2, lastRead1, lastRead2);
        #endif
        return lastRead2;
    }
    // x-axis
    if (type == 1) {
        return old_x;
    // y-axis
    } else if (type == 2) {
        return old_y;
    // z-axis
    } else if (type == 3) {
        return old_z;
    } else {
        return 9999999;
    }
}

// linearly scales joystick axis data down to acceptable RoboCommander inputs
void processAxes(int *axis) {
    int x = axis[0];
    int y = axis[1];
    int z = axis[2];
	
    #ifdef AXIS_DEBUG_D    
    printf("<<< X: %6d Y: %6d Z: %6d\n", x, y, z);
    #endif

    // scale down joystick data to acceptable RoboCommander inputs
    scale_x_axis(x);
    scale_y_axis(y);    
    scale_z_axis(z);

    this_x = checkInflexions(xlastRead3, xlastRead2, xlastRead1, 1);
    this_y = checkInflexions(ylastRead3, ylastRead2, ylastRead1, 2);
    this_z = checkInflexions(zlastRead3, zlastRead2, zlastRead1, 3);

    #ifdef AXIS_DEBUG_D    
    printf(">>> X: %6d Y: %6d Z: %6d\n", this_x, this_y, this_z);
    printf("------------------------\n");
    #endif

    // reduce the amount of data being sent
//    reduceDataFrequency();

    // slow down speeds when more than 2 or more axes are being moved
    reduceAxisSpeeds();
}

// reduces axis speeds when 2 or more axes are moving too fast
void reduceAxisSpeeds() {
    // if all 3 axes are moving, disable one of the axes
    if (this_x !=0 && this_y != 0 && this_z != 0) {
        // if x-axis is the last to move, disable it
	    if (old_y != 0 && old_z != 0 && old_x == 0) {
	        this_x = 0;

        // if y-axis is the last to move, disable it
	    } else if (old_x != 0 && old_z != 0 && old_y == 0) {
	        this_y = 0;

        // if z-axis is the last to move, disable it
	    } else if (old_x != 0 && old_y != 0 && old_z == 0) {
	        this_z = 0;

        // default action is to disable the x-axis
        } else {
            this_x = 0;
        }
    }

    // if both y and z axes are moving, reduce speed
    if (this_y != 0 && this_z != 0) {
        if (this_y > 0) {
	        if (this_z > 16) {
	            this_z = 16;
	        } else if (this_z < -16) {
	            this_z = -16;
	        }
	    } else if (this_y < 0) {
	        if (this_z > 11) {
	            this_z = 11;
	        } else if (this_z < -11) {
	            this_z = -11;
	        }
	    }

    // if both x and z axes are moving, reduce speed    
    } else if (this_x != 0 && this_z != 0) {
        if (this_x > 0) {
	        if (this_z > 11) {
	            this_z = 11;
	        }
	    } else if (this_x < 0) {
	        if (this_z < -11) {
	            this_z = -11;
	        }
	    }
    
    // if both x and y axes are moving, reduce speed    
    } else if (this_x != 0 && this_y != 0) {
        if (this_x > 0) {
	        if (this_y > 4) {
	            this_y = 4;
	        } else if (this_y < 0) {
	            this_y = 0;
	        }
	    } else if (this_x < 0) {
	        if (this_y > 2) {
	            this_y = 2;
	        } else if (this_y < -2) {
	            this_y = -2;
	        }
	    }
    }    
}

// reduces joystick sending rate by only sending half of the joystick data
// x-axis data is sent if it is in the set {-8,-6,-4,-2,0,2,4,6,8}
// y-axis data is sent if it is in the set {-6,-4,-2,0,2,4,6,8}
// z-axis data is sent if it is in the set {-21,-16,-11,-6,-1,0,1,6,11,16,21}
void reduceDataFrequency() {
    if (hasRemainderOne(this_x, 2)) {
        this_x = old_x;
    }
    if (hasRemainderOne(this_y, 2)) {
        this_y = old_y;
    }
    if (this_z!=0 && !hasRemainderOne(this_z, 5)) {
        this_z = old_z;
    }
}

// checks to see whether a given number has a remainder of one, when
// divided by another number
int hasRemainderOne(int n, int modNum) {
    if (n >= 0) {
        return (n%modNum==1);
    } else {
        int m = -n;
 	    return (m%modNum==1);
    }
}

// x-axis: scaled from [-512,511] to [-8,8]    
void scale_x_axis(int x) {
    this_x = 0;
    if (x > 100) {
        if (x < 65) {
	        this_x = -1;
	    } else if (x < 129) {
	        this_x = -2;
	    } else if (x < 193) {
	        this_x = -3;	
	    } else if (x < 257) {
	        this_x = -4;	
	    } else if (x < 321) {
	        this_x = -5;	
	    } else if (x < 385) {
	        this_x = -6;	
	    } else if (x < 449) {
	        this_x = -7;	
	    } else {
	        this_x = -8;	
	    } 
    } else if (x < -100) {
        if (x > -65) {
	        this_x = 1;
	    } else if (x > -129) {
	        this_x = 2;
	    } else if (x > -193) {
	        this_x = 3;	
	    } else if (x > -257) {
	        this_x = 4;	
	    } else if (x > -321) {
	        this_x = 5;	
	    } else if (x > -385) {
	        this_x = 6;	
	    } else if (x > -449) {
	        this_x = 7;	
	    } else {
	        this_x = 8;	
	    } 
    }
    xlastRead3 = xlastRead2;
    xlastRead2 = xlastRead1;
    xlastRead1 = this_x;
}

// y-axis: scaled from [-512,511] to [-6,8]    
void scale_y_axis(int y) {
    this_y = 0;
    if (y > 100) {
        if (y < 86) {
	        this_y = -1;
	    } else if (y < 171) {
	        this_y = -2;
	    } else if (y < 256) {
	        this_y = -3;	
	    } else if (y < 341) {
	        this_y = -4;	
	    } else if (y < 426) {
	        this_y = -5;	
            } else {
	        this_y = -6; 
	    } 
    } else if (y < -100) {
        if (y > -65) {
	        this_y = 1;
	    } else if (y > -129) {
	        this_y = 2;
	    } else if (y > -193) {
	        this_y = 3;	
	    } else if (y > -257) {
	        this_y = 4;	
	    } else if (y > -321) {
	        this_y = 5;	
	    } else if (y > -385) {
	        this_y = 6;	
	    } else if (y > -449) {
	        this_y = 7;	
	    } else {
	        this_y = 8;	
	    } 
    }
    ylastRead3 = ylastRead2;
    ylastRead2 = ylastRead1;
    ylastRead1 = this_y;
}

// z-axis: scaled from [-512,511] to [-21,21]
void scale_z_axis(int z) {
    this_z = 0;
    if (z > 100) {
        this_z = -((z+1) / 34);
    } else if (z < -100) {
        this_z = -(z / 34);
    }
    zlastRead3 = zlastRead2;
    zlastRead2 = zlastRead1;
    zlastRead1 = this_z;
}

// sends joystick button data via Client.c's sendRobotCommand function
void sendButtonData(int button_number) {
    char name[1];
    char value[3];

    sprintf(name, " ");
    sprintf(value, " ");

    switch (button_number) {
	    // 90 turn kick left
	    case 0:
	        sprintf(name, "K");
	        sprintf(value, "8");
            break;
	    // 180 turn kick left
	    case 3:
	        sprintf(name, "K");
	        sprintf(value, "10");
            break;

	    // 90 turn kick right
	    case 2:
	        sprintf(name, "K");
	        sprintf(value, "9");
            break;
	    // 180 turn kick right			
	    case 5:
	        sprintf(name, "K");
	        sprintf(value, "11");
            break;

	    // lightning kick
	    case 1:
	        sprintf(name, "K");
	        sprintf(value, "3");
            break;
	    // block
	    // chest push
	    case 4:
	        sprintf(name, "K");
//	        sprintf(value, "6");
	        sprintf(value, "1");
            break;

        // buttons do nothing
	    case 6:
	    case 7:
            return;

	    //every other button just does abort kick
	    case 8:
	    case 9:
	    case 10:
	        sprintf(name, "K");
	        sprintf(value, "14");
            break;
    }

    #ifndef STAND_ALONE_APP_D
    logButtons(button_number);
    sendRobotCommand(name, value);
    #endif
}

// prints out joystick data to be logged
void logButtons(int button) {
    ftime(&tp);
    if (tp.millitm < 10) {
  	    printf("#%ld00%d:&%d\n",tp.time,tp.millitm, button+1);
    } else if (tp.millitm < 100) {
	    printf("#%ld0%d:&%d\n",tp.time,tp.millitm, button+1);
    } else {
	    printf("#%ld%d:&%d\n",tp.time,tp.millitm, button+1);
    }
    std::cout.flush();
}

#endif // JOYSTICK_CONTROL_D
