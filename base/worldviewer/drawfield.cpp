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


#include "drawfield.h"
#include "gltext.h"
#include <cmath>

const float GLObject::DEG2RAD = M_PI/180; 
const float GLObject::RAD2DEG = 180/M_PI; 
GLColour GLObject::default_colour = GLColour::WHITE;

GLObject::GLObject()
{
   colour = default_colour;
}

void GLObject::draw_children()
{
   std::list<GLObject *>::iterator it;

   for (it = children.begin(); it != children.end(); it++)
   {
      (*it)->draw();
   }
}

void GLObject::add_child(GLObject *o)
{
   children.push_back(o);
}

void GLObject::clear_children()
{
   std::list<GLObject *>::iterator it;
   
   for (it = children.begin(); it != children.end(); it++)
   {
      delete (*it);
   }

   children.clear();
}

GLObject::~GLObject()
{
   clear_children();
}

void GLObject::set_colour(GLColour c)
{
   colour = c;
}

void GLObject::set_colour(float r,float b,float g)
{
   colour = GLColour(r, b, g);
}

GLColour *GLObject::get_colour()
{
   return &colour;
}

void GLObject::set_default_colour(GLColour c)
{
   default_colour = c;
}

void GLObject::set_default_colour(float r, float b, float g)
{
   default_colour = GLColour(r, b, g);
}

// *******************************************************************************

Arrow::Arrow(Position pos, float head_width, float head_length, float base_width, float base_length)
{
   this->pos = pos;
   this->head_width = head_width;
   this->head_length = head_length;
   this->base_width = base_width;
   this->base_length = base_length;
}

void Arrow::draw()
{
   float half_base_width = base_width / 2.0f;
   float half_head_width = head_width / 2.0f;

   glPushMatrix();
   
   glTranslatef(pos.x, pos.y - ((base_length + head_length) / 2.0f), 0);
   colour.set();
   
   glBegin(GL_QUADS);
   glVertex2f(-half_base_width, 0);
   glVertex2f(half_base_width, 0);
   glVertex2f(half_base_width, base_length);
   glVertex2f(-half_base_width, base_length);
   glEnd();

   glBegin(GL_TRIANGLES);
   glVertex2f(-half_head_width, base_length);
   glVertex2f(half_head_width, base_length);
   glVertex2f(0, base_length + head_length);
   glEnd();

   glPopMatrix();
}

 
// *******************************************************************************

Circle::Circle(Position pos, float radius, int sections, float width) : GLObject()
{
   this->pos = pos;
   this->radius = radius;
   this->width = width;
   this->sections = sections;
}

void Circle::draw()
{
   colour.set();
   glLineWidth(width);

   glBegin(GL_LINE_LOOP);
   for (int i = 0; i < sections; i++)
   {
      float degInRad = 360 * i * DEG2RAD / sections;
      glVertex2f(pos.x + cos(degInRad)*radius, pos.y + sin(degInRad)*radius);
   }
   glEnd();
}

void Circle::set_radius(float radius)
{
   this->radius = radius;
}


// *******************************************************************************

SolidCircle::SolidCircle(Position pos, float radius, int sections, float width) : 
   Circle(pos, radius, sections, width)
{

}

void SolidCircle::draw()
{
   colour.set();
   
   glBegin(GL_POLYGON);

   for (int i=0; i < sections; i++)
   {
      float degInRad = 360 * i * DEG2RAD / sections;
      glVertex2f(pos.x + cos(degInRad)*radius,pos.y + sin(degInRad)*radius);
   }
 
   glEnd();
}

// *******************************************************************************


CircleStrip::CircleStrip(Position pos, float radius, int sections, float width) :
   Circle(pos, radius, sections, width)
{
   inner_radius = radius - width/2.0f;
   outer_radius = radius + width/2.0f;
}

void CircleStrip::draw()
{
   colour.set();
   
   glBegin(GL_TRIANGLE_STRIP);

   float degInRad, x_angle, y_angle;

   for (int i=0; i < sections + 1; i++)
   {
      degInRad = 360 * i * DEG2RAD / sections;
      x_angle = cos(degInRad);
      y_angle = sin(degInRad);
      
      glVertex2f(pos.x + x_angle * outer_radius, pos.y + y_angle * outer_radius);
      glVertex2f(pos.x + x_angle * inner_radius, pos.y + y_angle * inner_radius);
   }
 
   glEnd();
}
 
// *******************************************************************************

Line::Line(Position start, Position end, float width) : GLObject()
{
   this->start = start;
   this->end = end;
   this->width = width;
}

void Line::draw()
{
   glLineWidth(width);
   colour.set();
   glBegin(GL_LINES);
   glVertex2f(start.x, start.y);
   glVertex2f(end.x,end.y);
   glEnd();
}
 
// *******************************************************************************

SolidLine::SolidLine(Position start, Position end, float width) : Line(start, end, width)
{
   float y_offset = end.y - start.y;
   float x_offset = end.x - start.x;
   length = sqrt(x_offset * x_offset + y_offset * y_offset);
   angle = RAD2DEG * atan2(-x_offset, y_offset);

   printf("NEW LINE\n");
   printf("From: (%.2ff, %.2ff) to (%.2ff, %.2ff)\n", start.x, start.y, end.x, end.y);
   printf("Length: %.2ff Offsets: %.2f,%.2f Line angle is %.2f\n", length, x_offset, y_offset, angle);
}

void SolidLine::draw()
{
   glPushMatrix();
   colour.set();

   glTranslatef(start.x, start.y, 0.0);
   glRotatef(angle,0,0,1);
   glBegin(GL_QUADS);
   glVertex2f(0.0, 0.0);
   glVertex2f(width, 0.0);
   glVertex2f(width, length);
   glVertex2f(0.0, length);
   glEnd();
   glPopMatrix();
}

// *******************************************************************************

Rectangle::Rectangle(Position topleft, Position bottomright) : GLObject()
{
   this->topleft = topleft;
   this->bottomright = bottomright;
}

void Rectangle::set_position(Position topleft, Position bottomright)
{
   this->topleft = topleft;
   this->bottomright = bottomright;
}

void Rectangle::draw()
{
   colour.set();
   glBegin(GL_QUADS);
   glVertex3f(topleft.x, topleft.y, 0.0f);
   glVertex3f(topleft.x, bottomright.y, 0.0f);
   glVertex3f(bottomright.x, bottomright.y, 0.0f);
   glVertex3f(bottomright.x, topleft.y, 0.0f);
   glEnd();

   //printf("Drawing rectangle: %f, %f -> %f, %f\n", topleft.x, topleft.y, bottomright.x, bottomright.y);
}

// *******************************************************************************

GLField::GLField()
{
   // Warning: should add in order you want to draw for now
   
   Rectangle *r = new Rectangle(Position(0.0f, FULL_LENGTH), Position(FULL_WIDTH, 0.0f));
   r->set_colour(GLColour::GREEN);
   add_child(r);

   float goal_x_offset = (FULL_WIDTH - GOAL_WIDTH) / 2.0;
   float goal_y_offset = GOAL_DEPTH;

   r = new Rectangle(Position(goal_x_offset, goal_y_offset), Position(goal_x_offset + GOAL_WIDTH, 0.0));
   r->set_colour(GLColour::SKYBLUE);
   add_child(r);

   r = new Rectangle(Position(goal_x_offset, FULL_LENGTH), Position(goal_x_offset + GOAL_WIDTH, FULL_LENGTH - GOAL_DEPTH));
   r->set_colour(GLColour::YELLOW);
   add_child(r);
   
   float half_width = FULL_WIDTH / 2.0;
   float half_length = FULL_LENGTH / 2.0;

   float length_margin = (FULL_LENGTH - FIELD_LENGTH) / 2.0f;
   float width_margin = (FULL_WIDTH - FIELD_WIDTH) / 2.0f;

   GLObject::set_default_colour(GLColour::WHITE);
   
   CircleStrip *c = new CircleStrip(Position(half_width, half_length), 18, 20, 2.5);
   add_child(c);

   Position bottomleft(width_margin, length_margin);
   Position bottomright(FULL_WIDTH - width_margin, length_margin);
   Position topright(FULL_WIDTH - width_margin, FULL_LENGTH - length_margin);
   Position topleft(width_margin, FULL_LENGTH - length_margin);
   
   Line *l = new Line(bottomleft, bottomright, 2.5);
   add_child(l);
   
   l = new Line(bottomright, topright, 2.5);
   add_child(l);

   l = new Line(topright, topleft, 2.5);
   add_child(l);
   
   l = new Line(topleft, bottomleft, 2.5);
   add_child(l);
   
   l = new Line(Position(width_margin, FULL_LENGTH / 2.0), Position(FULL_WIDTH - width_margin, FULL_LENGTH / 2.0), 2.5);
   add_child(l);

   float goalbox_x_offset = (FULL_WIDTH - GOALBOX_WIDTH) / 2;
   float goalbox_y_offset = length_margin + GOALBOX_LENGTH;

   l = new Line(Position(goalbox_x_offset, goalbox_y_offset), Position(goalbox_x_offset, length_margin), 2.5);
   add_child(l);

   l = new Line(Position(goalbox_x_offset, goalbox_y_offset), Position(goalbox_x_offset + GOALBOX_WIDTH, goalbox_y_offset), 2.5);
   add_child(l);
   
   l = new Line(Position(goalbox_x_offset + GOALBOX_WIDTH, length_margin), Position(goalbox_x_offset + GOALBOX_WIDTH, goalbox_y_offset), 2.5);
   add_child(l);

   l = new Line(Position(goalbox_x_offset, FULL_LENGTH - goalbox_y_offset), Position(goalbox_x_offset, FULL_LENGTH - length_margin), 2.5);
   add_child(l);

   l = new Line(Position(goalbox_x_offset, FULL_LENGTH - goalbox_y_offset), Position(goalbox_x_offset + GOALBOX_WIDTH, FULL_LENGTH - goalbox_y_offset), 2.5);
   add_child(l);
   
   l = new Line(Position(goalbox_x_offset + GOALBOX_WIDTH, FULL_LENGTH - length_margin), Position(goalbox_x_offset + GOALBOX_WIDTH, FULL_LENGTH - goalbox_y_offset), 2.5);
   add_child(l);

   beacons.clear();

   float beacon_offset = width_margin / 2.0f;
   
   GLBeacon *b = new GLBeacon(Position(beacon_offset, half_length - 135), GLColour::SKYBLUE, GLColour::PINK);
   beacons.push_back(b);

   b = new GLBeacon(Position(FULL_WIDTH - beacon_offset, half_length - 135), GLColour::PINK, GLColour::SKYBLUE);
   beacons.push_back(b);
   
   b = new GLBeacon(Position(beacon_offset, half_length + 135), GLColour::YELLOW, GLColour::PINK);
   beacons.push_back(b);
   
   b = new GLBeacon(Position(FULL_WIDTH - beacon_offset, half_length + 135), GLColour::PINK, GLColour::YELLOW);
   beacons.push_back(b);
}

GLField::~GLField()
{
   std::list<GLBeacon *>::iterator it;
   
   for (it = beacons.begin(); it != beacons.end(); it++)
   {
      delete (*it);
   }

   beacons.clear();
}

void GLField::draw()
{
   glTranslatef(-(float)FULL_WIDTH / 2.0, -(float)FULL_LENGTH / 2.0, 0.0f);
   draw_children();
   draw_beacons();
}

void GLField::draw_beacons()
{
   std::list<GLBeacon *>::iterator it;

   for (it = beacons.begin(); it != beacons.end(); it++)
   {
      (*it)->draw();
   }
}

void GLField::set_beacon_angle(float angle)
{
   std::list<GLBeacon *>::iterator it;

   for (it = beacons.begin(); it != beacons.end(); it++)
   {
      GLBeacon *b = *it;
      b->set_angle(angle);
   }
}

// *******************************************************************************

Position::Position()
{
   x = y = -1.0f;
}

Position::Position(float x, float y)
{
   this->x = x;
   this->y = y;
}

// *******************************************************************************

GLBeacon::GLBeacon(Position pos, GLColour middle, GLColour top) : GLObject()
{
   this->pos = pos;
   this->middle = middle;
   this->top = top;
   this->angle = 0;

   //printf("Adding becaon!\n");
}

void GLBeacon::draw()
{
   float length = 20;
   float width = 10;
   float section = length / 3.0f;

   glPushMatrix();
   
   glTranslatef(pos.x, pos.y,0.0f);
   glRotatef(angle, 0.0, 0.0, 1.0f);
   glTranslatef(-width / 2.0f, -length / 2.0f, 0.0f);
   
   glBegin(GL_QUADS);
   colour.set();
   glVertex2f(0, 0);
   glVertex2f(width, 0);
   glVertex2f(width, section);
   glVertex2f(0, section);
   middle.set();
   glVertex2f(0, section);
   glVertex2f(width, section);
   glVertex2f(width, section * 2);
   glVertex2f(0, section * 2);
   top.set();
   glVertex2f(0, section * 2);
   glVertex2f(width, section * 2);
   glVertex2f(width, section * 3);
   glVertex2f(0, section * 3);
   glEnd();

   glPopMatrix();
}

void GLBeacon::set_angle(float angle)
{
   this->angle = angle;
}

GLBeacon::~GLBeacon()
{
   //printf("WHAT???\n");
}

// *******************************************************************************

bool GLRobot::show_text = true;

GLRobot::GLRobot() : GLObject()
{
   pos = Position(0,0);
   heading = 0;

   float radius = 10;
   var_circle = new Circle(pos, 10, 20);
   main_circle = new SolidCircle(pos, radius, 20);
   //main_arrow = new Arrow(pos, 8, 4, 4, 4);
   main_arrow = new Arrow(pos, 14, 10, 4, 6);
   main_arrow->set_colour(GLColour::WHITE);
}

GLRobot::~GLRobot()
{
   if (var_circle != NULL)
      delete var_circle;

   if (main_circle != NULL)
      delete main_circle;

   if (main_arrow != NULL)
      delete main_arrow;
}

void GLRobot::flip()
{
   pos.x = GLField::FULL_WIDTH - pos.x;
   pos.y = GLField::FULL_LENGTH - pos.y;
   heading = heading + 180;
}

void GLRobot::set_position(Position pos)
{
   this->pos.x = pos.x+20;
   this->pos.y = pos.y+30;
}

void GLRobot::set_heading(float heading)
{
   this->heading = heading;
}

void GLRobot::set_variance(float variance)
{
   this->variance = variance;
   var_circle->set_radius(sqrt(variance));
}

void GLRobot::set_colour(GLColour c)
{
   colour = c;
   var_circle->set_colour(c);
   var_circle->get_colour()->set_alpha(0.5f);
   main_circle->set_colour(c);
}

void GLRobot::set_arrow_colour(GLColour c)
{
   main_arrow->set_colour(c);
}

void GLRobot::set_text(const char *fmt, ...)
{
   va_list ap;
   va_start(ap, fmt);
   vsnprintf(text, 1024, fmt, ap);
   va_end(ap);
}

void GLRobot::set_show_text(bool new_show_text)
{
   show_text = new_show_text;
}

void GLRobot::set_show_variance(bool show_var)
{
   show_variance = show_var;
}

void GLRobot::draw()
{
   glPushMatrix();
   colour.set();
   glTranslatef(pos.x, pos.y,0.0f);
   glRotatef(heading-90,0,0,1);

   if (show_variance)
   {
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glEnable(GL_BLEND);
      var_circle->draw();
      glDisable(GL_BLEND);
   }
   main_circle->draw();
   main_arrow->draw();

   if (show_text)
   {
      // Draw text
      float modelview[16];
      float up[2];
      int i,j;

      glGetFloatv(GL_MODELVIEW_MATRIX, modelview);

      up[0] = modelview[1];
      up[1] = modelview[5];

      float uplength =  sqrtf(up[0] * up[0] + up[1] * up[1]);
      up[0] /= uplength;
      up[1] /= uplength;

      glTranslatef(up[0]*15, up[1]*15, 0);
      glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
      
      // undo all rotations...beware all scaling is lost as well 
      for( i=0; i<3; i++ ) 
      {
         for( j=0; j<3; j++ ) 
         {
            if ( i==j )
               modelview[i*4+j] = 1.0;
            else
               modelview[i*4+j] = 0.0;
         }
      }

      glLoadMatrixf(modelview);
      
      GLText *gltext = GLText::get_instance();
      gltext->set_colour(colour);
      gltext->print(0,0, text);
   }
   
   glPopMatrix();
   //printf("Drawing robot at: %.2f, %.2f, heading %.2f\n", pos.x, pos.y, heading);
}

// *******************************************************************************

GLObstacle::GLObstacle() : Rectangle(Position(0,0),Position(0,0))
{
    pos = Position(0,0);
}

GLObstacle::~GLObstacle() 
{
}

void GLObstacle::flip() 
{
    pos.x = GLField::FULL_WIDTH - pos.x; 
    pos.y = GLField::FULL_LENGTH - pos.y;    
    Rectangle::set_position(Position(pos.x-5-20,pos.y+5-30),
                            Position(pos.x+5-20,pos.y-5-30));
}

void GLObstacle::set_position(Position pos) 
{
    this->pos = pos;
    Rectangle::set_position(Position(pos.x-5+20,pos.y+5+30),
                            Position(pos.x+5+20,pos.y-5+30));
}


// *******************************************************************************

bool GLBall::show_text = true;

GLBall::GLBall() : GLObject()
{
   pos = Position(0,0);

   float radius = 6;
   var_circle = new Circle(pos, 10, 20);
   ball_circle = new SolidCircle(pos, radius, 20);
   ball_core = new SolidCircle(pos, (radius/2), 20);
   set_colour(GLColour::ORANGE);
}

GLBall::~GLBall()
{
   if (var_circle != NULL)
      delete var_circle;

   if (ball_circle != NULL)
      delete ball_circle;
   
   if (ball_core != NULL)
      delete ball_core;   
}

void GLBall::flip()
{
   pos.x = GLField::FULL_WIDTH - pos.x;
   pos.y = GLField::FULL_LENGTH - pos.y;
}

void GLBall::set_position(Position pos)
{
   this->pos.x = pos.x+20;
   this->pos.y = pos.y+30;
}

void GLBall::set_variance(float variance)
{
   this->variance = variance;
   var_circle->set_radius(sqrt(variance));
}

void GLBall::set_var_colour(GLColour c)
{
   colour = c;
   var_circle->set_colour(c);
   var_circle->get_colour()->set_alpha(0.5f);
}

void GLBall::set_ball_colour(GLColour c)
{
   ball_circle->set_colour(c);
   ball_core->set_colour(c);
}

void GLBall::set_core_colour(GLColour c)
{
   ball_core->set_colour(c);
}

void GLBall::set_text(const char *fmt, ...)
{
   va_list ap;
   va_start(ap, fmt);
   vsnprintf(text, 1024, fmt, ap);
   va_end(ap);
}

void GLBall::set_show_text(bool new_show_text)
{
   show_text = new_show_text;
}

void GLBall::set_show_variance(bool show_var)
{
   show_variance = show_var;
}

void GLBall::draw()
{
   glPushMatrix();
   colour.set();
   glTranslatef(pos.x, pos.y, 0.0f);

   
   ball_circle->draw();
   ball_core->draw();
   if (show_variance)
   {
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glEnable(GL_BLEND);
      var_circle->draw();
      glDisable(GL_BLEND);
   }

   if (show_text)
   {
      // Draw text
      float modelview[16];
      float up[2];
      int i,j;

      glGetFloatv(GL_MODELVIEW_MATRIX, modelview);

      up[0] = modelview[1];
      up[1] = modelview[5];

      float uplength =  sqrtf(up[0] * up[0] + up[1] * up[1]);
      up[0] /= uplength;
      up[1] /= uplength;

      glTranslatef(up[0]*15, up[1]*15, 0);
      glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
      
      // undo all rotations...beware all scaling is lost as well 
      for( i=0; i<3; i++ ) 
      {
         for( j=0; j<3; j++ ) 
         {
            if ( i==j )
               modelview[i*4+j] = 1.0;
            else
               modelview[i*4+j] = 0.0;
         }
      }

      glLoadMatrixf(modelview);
      
      GLText *gltext = GLText::get_instance();
      gltext->set_colour(colour);
      gltext->print(0,0, text);
   }
   
   glPopMatrix();
   //printf("Drawing robot at: %.2f, %.2f, heading %.2f\n", pos.x, pos.y, heading);
}

// *******************************************************************************
 
GLColour::GLColour()
{
   r = b = g = 1.0f;
   alpha = -1.0f;
}

GLColour::GLColour(float r, float b, float g, float a)
{
   this->r = r;
   this->b = b;
   this->g = g;
   this->alpha = a;
}

void GLColour::set()
{
   if (alpha < 0)
      glColor3f(r,b,g);
   else
      glColor4f(r,b,g,alpha);
}

void GLColour::set_alpha(float a)
{
   this->alpha = a;
}

// *******************************************************************************

GLColour GLColour::RED = GLColour(1.0f, 0.0f, 0.0f);
GLColour GLColour::DARKRED = GLColour(0.7f, 0.0f, 0.0f);
GLColour GLColour::GREEN = GLColour(0.149f, 0.784f, 0.235f);
GLColour GLColour::PINK = GLColour(1.0f, 0.482f, 0.863f);
GLColour GLColour::SKYBLUE = GLColour(0.259f, 0.694f, 1.0f);
GLColour GLColour::BLUE = GLColour(0.0f, 0.0f, 1.0f);
GLColour GLColour::DARKBLUE = GLColour(0.0f, 0.0f, 0.7f);
GLColour GLColour::YELLOW = GLColour(0.890f, 0.957f, 0.082f);
GLColour GLColour::WHITE = GLColour(1.0f, 1.0f, 1.0f);
GLColour GLColour::ORANGE = GLColour(1.00f, 0.639f, 0.165f);
GLColour GLColour::PURPLE = GLColour(1.00f, 0, 1.00f);
GLColour GLColour::GREYWHITE = GLColour(0.8f, 0.8f, 0.8f);
