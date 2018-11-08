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
 * Displays the colour cube
 * @author James Sye Jiang Wong
 * @email  jamesjw@cse.unsw.edu.au
 */

#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <list>
#include <algorithm>
#include <cctype>
#include <fstream>
#include "trackball.h"
#include "colour_definition.h"

using namespace std;
#pragma comment( linker, "/entry:\"mainCRTStartup\"" )  // set the entry point to be main()

#define NO_UNCLASSIFIED 1
#define BINARY_INPUT 1
// sets the scale of the colour cube
#define SCALE 2

GLfloat rotAngle = 1;
GLfloat angle = 0;
//char axis= 'y';
char spin = GL_FALSE;
char move = GL_FALSE;
GLint prevX = 0;
GLint prevY = 0; 
GLuint aWidth = 400;
GLuint aHeight = 400;

float curQuat[4];
float lastQuat[4];
float rotMatrix[4][4];

int pixelsNum;

// display list ID
GLuint objectDL;

#define LINE_LENGTH 78

#define MAXIMUM_Y 127
#define MAXIMUM_U 127
#define MAXIMUM_V 127

#define MINIMUM_Y 0
#define MINIMUM_U 0
#define MINIMUM_V 0

GLint max_y = MAXIMUM_Y+1;
GLint max_u = MAXIMUM_U+1;
GLint max_v = MAXIMUM_V+1;
    
GLfloat cmin_y = (0-(float)MAXIMUM_Y/2.0)*SCALE;
GLfloat cmin_u = (0-(float)MAXIMUM_U/2.0)*SCALE;
GLfloat cmin_v = (0-(float)MAXIMUM_V/2.0)*SCALE;
GLfloat cmax_y = (MAXIMUM_Y-(float)MAXIMUM_Y/2.0)*SCALE;
GLfloat cmax_u = (MAXIMUM_U-(float)MAXIMUM_U/2.0)*SCALE;
GLfloat cmax_v = (MAXIMUM_V-(float)MAXIMUM_V/2.0)*SCALE;


GLbyte choose = 'y';

GLfloat eyeZ = 400.0f;


typedef struct _colour {
    GLint y,u,v,c;
} colour;

colour *colours;


void increase()
{
    switch (choose)
    {
        case 'y':
            if (++max_y > MAXIMUM_Y)
                max_y = MAXIMUM_Y;
            break;
        case 'u':
            if (++max_u > MAXIMUM_U)
                max_u = MAXIMUM_U;
            break;
        case 'v':
            if (++max_v > MAXIMUM_V)
                max_v = MAXIMUM_V;
            break;
    }
}

void decrease()
{
    switch (choose)
    {
        case 'y':
            if (--max_y < MINIMUM_Y)
                max_y = MINIMUM_Y;
            break;
        case 'u':
            if (--max_u < MINIMUM_U)
            {
                max_u = MINIMUM_U;
            }
            printf("%d\n",max_u);
            break;
        case 'v':
            if (--max_v < MINIMUM_V)
                max_v = MINIMUM_V;
            break;
    }
}

void keyPressed(unsigned char key, int x, int y)
{
    //printf("Key pressed: %c\n" +(char) key);
    switch (key) {
        case 'q': exit(0); break;
        case '=':
        case '+': --eyeZ; break;
        case '-': ++eyeZ; break;
        case '9': decrease(); glutPostRedisplay(); break;
        case '0': increase(); glutPostRedisplay(); break;
        case 'y': choose = 'y'; break;
        case 'u': choose = 'u'; break;
        case 'v': choose = 'v'; break;
    }
}

void menu(int value)
{
    keyPressed((unsigned char)value, 0, 0);
}

/**
 * 'button' can have any of the three values: GLUT_LEFT_BUTTON,
 * GLUT_MIDDLE_BUTTON, or  GLUT_RIGHT_BUTTON
 *
 * 'state' is either GLUT_UP or GLUT_DOWN
 *
 * 'x' and 'y' indicates the window relative coordinates when the mouse button
 * state changed
 */
void mouseEvent(int button, int state, int x, int y)
{
    // will only call mouseEvent if a mouse button is pressed or released
    switch(state)
    {
        case GLUT_DOWN:
            prevX = x;
            prevY = y;
            spin = GL_FALSE;
            move = GL_TRUE;
            break;
        case GLUT_UP:
            move = GL_FALSE;
            break;            
    }
}

void idle(void)
{
    if (spin)
    {
        add_quats(lastQuat, curQuat, curQuat);
    } else
    {
        angle += rotAngle;
    }
    glutPostRedisplay();
}

void vis(int visible)
{
    if (visible == GLUT_VISIBLE) {
        if (spin)
            glutIdleFunc(idle);
    } else {
        if (spin)
            glutIdleFunc(NULL);
    }
}

void mouseMotion(int x, int y)
{
    if (move)
    {
        trackball(lastQuat,
          (2.0 * prevX - aWidth) / aWidth,
          (aHeight - 2.0 * prevY) / aHeight,
          (2.0 * x - aWidth) / aWidth,
          (aHeight - 2.0 * y) / aHeight
          );

        prevX = x;
        prevY = y;
        spin = GL_TRUE;
        glutIdleFunc(idle);
    }
    add_quats(lastQuat, curQuat, curQuat);    
    
}

void reshape(int width, int height)
{
    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);
    aWidth = width;
    aHeight = height;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void init(void)
{
    glEnable(GL_DEPTH_TEST);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    trackball(curQuat,0.0f,0.0f,0.0f,0.0f);
}

void drawCube(void)
{
/*
    glBegin(GL_LINE_LOOP);
    glVertex3f((float)-max_y/2.0,(float)-max_u/2.0,(float)-max_v/2.0);
    glVertex3f((float)max_y/2.0,(float)-max_u/2.0,(float)-max_v/2.0);
    glVertex3f((float)max_y/2.0,(float)max_u/2.0,(float)-max_v/2.0);
    glVertex3f((float)-max_y/2.0,(float)max_u/2.0,(float)-max_v/2.0);
    glVertex3f((float)-max_y/2.0,(float)-max_u/2.0,(float)-max_v/2.0);
    
    glVertex3f((float)-max_y/2.0,(float)-max_u/2.0,(float)max_v/2.0);
    glVertex3f((float)max_y/2.0,(float)-max_u/2.0,(float)max_v/2.0);
    glVertex3f((float)max_y/2.0,(float)-max_u/2.0,(float)-max_v/2.0);
    
    glVertex3f((float)max_y/2.0,(float)max_u/2.0,(float)-max_v/2.0);
    glVertex3f((float)max_y/2.0,(float)max_u/2.0,(float)max_v/2.0);
    
    glVertex3f((float)max_y/2.0,(float)-max_u/2.0,(float)max_v/2.0);
    glVertex3f((float)-max_y/2.0,(float)-max_u/2.0,(float)max_v/2.0);
    glVertex3f((float)-max_y/2.0,(float)max_u/2.0,(float)max_v/2.0);
    glVertex3f((float)max_y/2.0,(float)max_u/2.0,(float)max_v/2.0);
    
    glVertex3f((float)max_y/2.0,(float)max_u/2.0,(float)-max_v/2.0);
    glVertex3f((float)-max_y/2.0,(float)max_u/2.0,(float)-max_v/2.0);
    glVertex3f((float)-max_y/2.0,(float)max_u/2.0,(float)max_v/2.0);
    
    glVertex3f((float)-max_y/2.0,(float)-max_u/2.0,(float)max_v/2.0);
    
    glVertex3f((float)-max_y/2.0,(float)max_u/2.0,(float)max_v/2.0);
    glVertex3f((float)max_y/2.0,(float)max_u/2.0,(float)max_v/2.0);
    glVertex3f((float)max_y/2.0,(float)-max_u/2.0,(float)max_v/2.0);
    glVertex3f((float)-max_y/2.0,(float)-max_u/2.0,(float)max_v/2.0);
    glEnd();
*/

    glBegin(GL_LINE_LOOP);
    glVertex3f(cmin_y,cmin_u,cmin_v);
    glVertex3f(cmax_y,cmin_u,cmin_v);
    glVertex3f(cmax_y,cmax_u,cmin_v);
    glVertex3f(cmin_y,cmax_u,cmin_v);
    glVertex3f(cmin_y,cmin_u,cmin_v);
    
    glVertex3f(cmin_y,cmin_u,cmax_v);
    glVertex3f(cmax_y,cmin_u,cmax_v);
    glVertex3f(cmax_y,cmin_u,cmin_v);
    
    glVertex3f(cmax_y,cmax_u,cmin_v);
    glVertex3f(cmax_y,cmax_u,cmax_v);
    
    glVertex3f(cmax_y,cmin_u,cmax_v);
    glVertex3f(cmin_y,cmin_u,cmax_v);
    glVertex3f(cmin_y,cmax_u,cmax_v);
    glVertex3f(cmax_y,cmax_u,cmax_v);
    
    glVertex3f(cmax_y,cmax_u,cmin_v);
    glVertex3f(cmin_y,cmax_u,cmin_v);
    glVertex3f(cmin_y,cmax_u,cmax_v);
    
    glVertex3f(cmin_y,cmin_u,cmax_v);
    
    glVertex3f(cmin_y,cmax_u,cmax_v);
    glVertex3f(cmax_y,cmax_u,cmax_v);
    glVertex3f(cmax_y,cmin_u,cmax_v);
    glVertex3f(cmin_y,cmin_u,cmax_v);
    glEnd();
}

// no optimisation. display normally.
void normalDisplay(void)
{
    GLfloat zero[] = {0.0f, 0.0f, 0.0f, 1.0f};
    GLfloat objectMat[] = {0.4f, 0.4f, 0.4f, 1.0f};
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_COLOR_MATERIAL);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, objectMat);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, zero);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0);

    glPointSize(1.0f);
    glBegin(GL_POINTS);
    for (int i=0; i<pixelsNum; ++i)
    {
        switch(colours[i].c)
        {
            case BALL:
                glColor3ub(237,94,28);
                break;
            case BLUE:
                glColor3ub(33,215,239);
                break;
            case GREEN:
                glColor3ub(31,158,42);
                break;
            case YELLOW:
                glColor3ub(249,233,9);
                break;
            case PINK:
                glColor3ub(255,127,235);
                break;
            case BLUE_DOG:
                glColor3ub(28,17,155);
                break;
            case RED_DOG:
                glColor3ub(255,0,0);
                break;
            case GREEN_FIELD:
                glColor3ub(45,191,38);
                break;
            case UNCLASSIFIED:
                glColor3ub(255,255,255);
                break;
						case FIELDWHITE:
								glColor3ub(128,128,128);
								break;
						
/*
            case MAYBE_ORANGE:
                glColor3ub(237,94,28);
                break;
            case MAYBE_BLUE:
                glColor3ub(33,215,239);
                break;
            case MAYBE_GREEN:
                glColor3ub(31,158,42);
                break;
            case MAYBE_PINK:
                glColor3ub(255,127,235);
                break;
            case MAYBE_RED_DOG:
                glColor3ub(255,0,0);
                break;
            case MAYBE_BLUE_DOG:
                glColor3ub(28,17,155);
                break;
            case MAYBE_DOG:
                glColor3ub(128,128,128);
                break;
            case YELLOW_OR_ORANGE:
                glColor3ub(249,233,9);
                break;
*/
            default:
                continue;
        }
        
        for (int yf=0; yf<SCALE; ++yf)
            for (int uf=0; uf<SCALE; ++uf)
                for (int vf=0; vf<SCALE; ++vf)
                {
                    glVertex3f((float)((colours[i].y-MAXIMUM_Y/2.0)*SCALE+yf),(float)((colours[i].u-MAXIMUM_U/2.0)*SCALE+uf),(float)((colours[i].v-MAXIMUM_V/2.0)*SCALE+vf));
                }
    }

    /*for (int y=0; y<max_y;++y)
    {
        for (int u=0; u<max_u; ++u)
        {
            for (int v=0; v<max_v; ++v)
            {
                float r = (y + (1.370705f * (v-128)));
                float g = (y - (0.698001f * (v-128)) - (0.337633f * (u-128)));
                float b = (y + (1.732446f * (u-128)));
                if (r > 255) r = 255;
                if (g > 255) g = 255;
                if (b > 255) b = 255;
                if (r < 0) r = 0;
                if (g < 0) g = 0;
                if (b < 0) b = 0;
                //cout << "Red is: " << r/255.0 << endl;
                glColor3f(r/255.0,g/255.0,b/255.0);

                //glVertex3f(y/(float)max_y-0.5,u/(float)max_u-0.5,v/(float)max_v-0.5);
                glVertex3f((float)y-MAXIMUM_Y/2.0,(float)u-MAXIMUM_U/2.0,(float)v-MAXIMUM_V/2.0);
            }
        }
    }*/

    glEnd();
    
    glColor3ub(200,200,200);
    //glutWireCube(max_y);
    drawCube();

    glPopMatrix();
    glFlush();
    glutSwapBuffers();
}


void display(void)
{
    GLfloat lightdiff[] = {1.0f,1.0f,1.0f,1.0f};
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, 1, 1, 1000);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    
    gluLookAt(0.0f, 0.0f, eyeZ, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    glShadeModel(GL_FLAT);
    glPushMatrix();
    glTranslatef(0.0f,0.0f,-5.0f);

    build_rotmatrix(rotMatrix,curQuat);
    glMultMatrixf(&rotMatrix[0][0]);
    normalDisplay();
}


int isnotspace(int c)
{
    return !isspace(c);
}

void parseFile(string fileName)
{
    int y,u,v,c;
    ifstream file(fileName.c_str());
    string line;
    char temp_line[LINE_LENGTH];
    if(!getline(file,line))
    {
        exit(1);
    }
    
    strcpy(temp_line,line.c_str());
    sscanf(temp_line, "%i %i %i", &max_y,&max_u,&max_v);
    
    colours = new colour[max_y*max_u*max_v];
    
    while (getline(file,line))
    {
        strcpy(temp_line,line.c_str());
        sscanf(temp_line, "%i %i %i %i", &y,&u,&v,&c);
        #if NO_UNCLASSIFIED
        if (c>=UNCLASSIFIED) {
            continue;
        }
        #endif
        colours[pixelsNum].y = y;
        colours[pixelsNum].u = u;
        colours[pixelsNum].v = v;
        colours[pixelsNum].c = c;
        ++pixelsNum;
    }
    file.close();
    //for (int i=0;i<pixelsNum;++i)
    //{
    //    printf("%d %d %d %d\n",colours[i].y,colours[i].u,colours[i].v,colours[i].c);
    //}
}

void parseBinaryFile(string fileName)
{
    unsigned char c;
    ifstream file(fileName.c_str(), ios::binary);
    colours = new colour[max_y*max_u*max_v];
    for (int y=0; y<max_y; ++y)
    {
        for (int u=0; u<max_u; ++u)
        {
            for (int v=0; v<max_v; ++v)
            {
                if (file.read((char *)(&c), sizeof(c)))
                {
                #if NO_UNCLASSIFIED
                if (c==UNCLASSIFIED) {
                    continue;
                }
                #endif

                    colours[pixelsNum].y = y;
                    colours[pixelsNum].u = u;
                    colours[pixelsNum].v = v;
                    colours[pixelsNum].c = c;
                    ++pixelsNum;
                }
                else
                {
                    file.close();
                    return;
                }
            }
        }
    }
}

int main(int argc, char** argv)
{
    if (argc>1)
        #if BINARY_INPUT
            parseBinaryFile(argv[1]);
        #else
            parseFile(argv[1]);
        #endif
    // used to take in any command line arguments intended for glut
    glutInit(&argc, argv);
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize (aWidth, aHeight);
    glutInitWindowPosition (100, 100);
    glutCreateWindow ("colourcube");
    glutCreateMenu(menu);
    glutAddMenuEntry("[=]  Zoom in",'=');
    glutAddMenuEntry("[+]  Zoom in",'+');
    glutAddMenuEntry("[-]  Zoom out",'-');
    glutAddMenuEntry("", 0);
    glutAddMenuEntry("[q]  Quit", 'q');
    glutAttachMenu(GLUT_RIGHT_BUTTON);
    init ();
    glutDisplayFunc(display); 
    glutReshapeFunc(reshape);
    glutVisibilityFunc(vis);
    glutKeyboardFunc(keyPressed);
    glutMouseFunc(mouseEvent);
    glutMotionFunc(mouseMotion);
    glutIdleFunc(idle);
    glutMainLoop();
    
    return 0;
}
