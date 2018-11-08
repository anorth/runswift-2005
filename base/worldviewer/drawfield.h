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


#ifndef FIELD_H
#define FIELD_H

#include <list>
#include <GL/gl.h>
#include <stdarg.h>
#include <math.h>

class Position
{
   public:
      Position();
      Position(float x, float y);
      float x, y;
};

class GLColour
{
   public:
      GLColour();
      GLColour(float r, float b, float g, float a = -1.0f);
      void set();
      void set_alpha(float a);
   protected:
      float r,b,g;
      float alpha;
      //float r,b,g;
   public:
      static GLColour RED, DARKRED, GREEN, PINK, SKYBLUE, BLUE, DARKBLUE, YELLOW, WHITE, ORANGE, PURPLE, GREYWHITE;
};

class GLObject
{
   public:
      GLObject();
      virtual ~GLObject();
      virtual void draw() = 0;
      void draw_children();
      virtual void set_colour(GLColour c);
      virtual void set_colour(float r, float b, float g);
      GLColour *get_colour();
      
      static void set_default_colour(float r, float b, float g);
      static void set_default_colour(GLColour c);
   protected:
      void add_child(GLObject *);
      void clear_children();
      GLColour colour;
      static GLColour default_colour;
      static const float DEG2RAD;
      static const float RAD2DEG;
   private:
      std::list<GLObject*> children;

};

class GLBeacon: public GLObject
{
   public:
      GLBeacon(Position pos, GLColour middle, GLColour top);
      ~GLBeacon();
      void draw();
      void set_angle(float angle);
   protected:
      Position pos;
      GLColour middle, top;
      float angle;
};

class GLField: public GLObject
{
   public:
      GLField();
      ~GLField();
      void draw();
      void set_beacon_angle(float angle);
      enum 
      {
         FULL_WIDTH = 400,
         FULL_LENGTH = 600,
         FIELD_WIDTH = 360,
         FIELD_LENGTH = 540,
         GOAL_WIDTH = 80,
         GOAL_DEPTH = 30,
         GOALBOX_WIDTH = 130,
         GOALBOX_LENGTH = 65,
      };
   protected:
      void draw_beacons();
      std::list<GLBeacon *> beacons;
};

class Line : public GLObject
{
   public:
      Line(Position start, Position end, float width = 5.0f);
      void draw();
   protected:
      Position start, end;
      float width;
};

class SolidLine : public Line
{
   public:
      SolidLine(Position start, Position end, float width = 5.0f);
      void draw();
   protected:
      float length, angle;
};

class Circle: public GLObject
{
   public:
      Circle(Position pos, float radius, int sections = 10, float width=1.0f);
      void draw();
      void set_radius(float radius);

   protected:
      Position pos;
      float width, radius;
      int sections;
};

class SolidCircle: public Circle
{
   public:
      SolidCircle(Position pos, float radius, int sections = 10, float width = 5.0f);
      void draw();
};

class CircleStrip: public Circle
{
   public:
      CircleStrip(Position pos, float radius, int sections = 10, float width = 5.0f);
      void draw();
   protected:
      float inner_radius, outer_radius;
};
              
class Rectangle : public GLObject
{
   public:
      Rectangle(Position topleft, Position bottomright);
      void set_position(Position topleft, Position bottomright);
      void draw();
   protected:
      Position topleft, bottomright;
};

class Arrow : public GLObject
{
   public:
      Arrow(Position pos, float head_width, float head_length, float base_width, float base_length);
      void draw();
   protected:
      Position pos;
      float head_width, head_length, base_width, base_length;
};

class GLObstacle : public Rectangle
{
    public: 
        GLObstacle(); 
        ~GLObstacle(); 
        void flip(); 
        void set_position(Position pos);    
        
    private: 
        Position pos;
};

class GLBall : public GLObject
{
   public:
      GLBall();
      ~GLBall();
      void flip();
      void set_position(Position pos);
      void set_variance(float variance);
      void set_show_variance(bool show_var);
      void set_var_colour(GLColour c);
      void set_ball_colour(GLColour c);
      void set_core_colour(GLColour c);
      void set_text(const char *fmt, ...);
      static void set_show_text(bool show_text);
      void draw();
   protected:
      char text[1024];
      Position pos;
      float variance;
      Circle *var_circle;
      SolidCircle *ball_core;
      SolidCircle *ball_circle;
      bool show_variance;
      static bool show_text;
};

class GLRobot : public GLObject
{
   public:
      GLRobot();
      ~GLRobot();
      void flip();
      void set_position(Position pos);
      void set_heading(float heading);
      void set_variance(float variance);
      void set_show_variance(bool show_var);
      void set_colour(GLColour c);
      void set_arrow_colour(GLColour c);
      void set_text(const char *fmt, ...);
      static void set_show_text(bool show_text);
      void draw();
   protected:
      char text[1024];
      Position pos;
      float heading, variance;
      Circle *var_circle;
      SolidCircle *main_circle;
      Arrow *main_arrow;
      bool show_variance;
      static bool show_text;
};
#endif
