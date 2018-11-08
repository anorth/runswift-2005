/*
   Copyright 2005 The University of New South Wales (UNSW) and National  
   ICT Australia (NICTA).

   This file is part of the 2005 team rUNSWift RoboCup entry.  You may  
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
   should be included in that publication.

   This rUNSWift source is distributed in the hope that it will be useful,  
   but WITHOUT ANY WARRANTY; without even the implied warranty of  
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  
   General Public License for more details.

   You should have received a copy of the GNU General Public License along  
   with this source code; if not, write to the Free Software Foundation,  
   Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 */


#include "gltext.h"
#include <stdio.h>

GLText *GLText::instance = NULL;

GLText::GLText(const char *filename)
{
   is_ok = false;
   
   FILE *f;
   f = fopen(filename, "rb");
   if (f == NULL)
      return;

   char *data;
   data = (char *)malloc(65536);
   memset(data, 0, 65536);

   fread(data, 1, 65536, f);
   fclose(f);

   glGenTextures(1, &texture_num);
   
   
   glBindTexture(GL_TEXTURE_2D, texture_num);
   glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_INTENSITY4, 256, 256, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, data);
   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);	// Linear Filtering
   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);	// Linear Filtering
   printf("Loaded textures\n");

   free(data);


   // 256 chars
   int num_chars = 256;
   float x, y;
   int i;

   base_list = glGenLists(num_chars);

   for (i = 0; i < num_chars; i++)
   {
      x = ((float)(i % 16)) / 16.0f;
      y = ((float)(i / 16)) / 16.0f;

      glNewList(base_list + i, GL_COMPILE);
      glBegin(GL_QUADS);
      glTexCoord2f(x, 1-y-0.0625f);
      glVertex2i(0,0);
      glTexCoord2f(x + 0.0625f, 1-y-0.0625f);
      glVertex2i(16,0);
      glTexCoord2f(x + 0.0625f, 1-y);
      glVertex2i(16,16);
      glTexCoord2f(x, 1-y);
      glVertex2i(0,16);
      glEnd();
      glTranslatef(10.0f,0,0);
      glEndList();
   }

   printf("Generated gl font list\n");

   is_ok = true;
}

GLText::~GLText()
{
}

GLText *GLText::get_instance()
{
   return instance;
}

void GLText::kill()
{
   if (instance != NULL)
      delete instance;
}

void GLText::print(float x, float y, const char *fmt,...)
{
   char text[1024];
   va_list ap;

   if (fmt == NULL)
      return;

   va_start(ap, fmt);
   vsnprintf(text, 1024, fmt, ap);
   va_end(ap);

   int width = strlen(text) * 10 + 6;

   glPushMatrix();
   
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, texture_num);
   //glDisable(GL_DEPTH_TEST);
   glEnable(GL_BLEND);								// For transparent background
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   glTranslatef(x - ((float)width) / 2.0f, y, 0);
   //glListBase(base_list - ' ' + 128);
   glListBase(base_list - ' ');
   colour.set();

   glCallLists(strlen(text), GL_BYTE, text);
   /*glBegin(GL_POLYGON);
   glTexCoord2f(0,0); glColor3f(1,0,0); glVertex2i(-128, 0);
   glTexCoord2f(1,0); glColor3f(0,1,0); glVertex2i(128, 0);
   glTexCoord2f(1,1); glColor3f(0,0,1); glVertex2i(128,256);
   glTexCoord2f(0,1); glColor3f(1,1,1); glVertex2i(-128,256);
   glEnd();*/

   glDisable(GL_BLEND);
   glDisable(GL_TEXTURE_2D);
   //glEnable(GL_DEPTH_TEST);

   glPopMatrix();
}

int GLText::start(const char *filename)
{
   if (instance != NULL)
      return 1;

   instance = new GLText(filename);
   
}

void GLText::set_colour(GLColour c)
{
   colour = c;
}


