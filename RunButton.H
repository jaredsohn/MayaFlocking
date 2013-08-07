#ifndef RUNBUTTON_H
#define RUNBUTTON_H

// $Header: /u/s/o/sohn/public/CVS/cs838proj2/flocking/RunButton.H,v 1.1 2002/03/24 01:02:52 sohn Exp $
// written October, 1999 by Michael L. Gleicher
//
// CS638 Example code - this provides a utility routine to help with animation
//
// This file implements a "play" button. The idea is that you creat a window that
// does your drawing. When redrawing, this window looks at a slider to see what
// time it is, so it knows what part of the animation to draw.
//
// The slider must call the window's redraw function whenever the time changes.
//
// The play button, when pressed, advances the slider forward a timestep continually.
// In order to do this, it installs itself in the FlTk "Idle" loop so it gets called
// periodically.
//
// See the Simple4.cpp program for an example of how to use this


#include <Fl/Fl_Value_Slider.H>
#include <Fl/Fl_Light_Button.H>

// the time slider
// the only thing it does is create itself. all of its work happens
// in a callback it installs
class RunSlider  : public Fl_Value_Slider {
public:		
	RunSlider(Fl_Widget* draw,		// what needs to be told to redraw
				  int length,			// how many frames is the animation
				  int x, int y, int w,int h);			// standard slider parameters	

  };

class FPSSlider : public Fl_Value_Slider {
public:		
	FPSSlider(Fl_Widget* draw, int length, int x, int y, int w,int h);
};

// the run button - advances the slider periodically
// note: this is hard coded to 30 frames per second
class RunButton : public Fl_Light_Button {
public:	
	RunButton(Fl_Slider* time, Fl_Slider* framesPerSec, int x, int y, int w, int h);
	~RunButton();					// remove the idle function

	Fl_Slider* slider;				// need to remember this
	Fl_Slider* framesPerSecSlider;
	Fl_Slider* velSlider;
	long lastRedraw;				// time of last redraw		
};

#endif