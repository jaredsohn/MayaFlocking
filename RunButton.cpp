// $Header: /u/s/o/sohn/public/CVS/cs838proj2/flocking/RunButton.cpp,v 1.1 2002/03/24 01:02:54 sohn Exp $
//
// CS638 Example code - this provides a utility routine to help with animation
// written October, 1999 by Michael L. Gleicher
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

#include "RunButton.H"
#include <Fl/Fl.h>
#include <time.h>
#include "stdio.h"

// the guts of the slider - 
void runSliderCB(Fl_Widget*, Fl_Widget* o)
{
	o->damage(1);
}

// the slider constructor
RunSlider::RunSlider(Fl_Widget* dr, int length,
					 int x, int y, int w, int h) : Fl_Value_Slider(x,y,w,h)
{

	// set up slider parameters
	value(1);
	range(1,length);
	step(1);
	type(5);				// horizontal, "nice"
	when(FL_WHEN_CHANGED);	// update whenever it changes

	callback((Fl_Callback*) runSliderCB, dr);
}

FPSSlider::FPSSlider(Fl_Widget* dr, int length, int x, int y, int w, int h) : Fl_Value_Slider(x,y,w,h)
{
	// set up slider parameters
	value(30);
	range(1,length);
	step(1);
	type(5);				// horizontal, "nice"
	when(FL_WHEN_CHANGED);	// update whenever it changes

	callback((Fl_Callback*) runSliderCB, dr);
}

//
// this callback function is the "guts" of the RunButton -
// notice that it is an "idle" callback, not a widget callback
//
// another nice problem to have - most likely, we'll be too fast
// don't draw more than 30 times per second
void runButtonCB(RunButton* b)
{
	if (b->value()) {	// only advance time if appropriate
		int FPS = (int)(b->framesPerSecSlider->value());
		if( FPS<=0 ) FPS=1;
		if (clock() - b->lastRedraw > CLOCKS_PER_SEC/FPS) {
			b->lastRedraw = clock();
			int t = (int) b->slider->value();
			int m = (int) b->slider->maximum();			
			t++;
			if (t>m) t=1;	// wrap around
			b->slider->value(t);
			// note: FlTk doesn't have the slider call its callback
			// when the value is changed internally
			(b->slider->callback())(b->slider,b->slider->user_data());
		}
	}
}

// create the run button - the main thing it does is install the idle
// handler
RunButton::RunButton(Fl_Slider* time, Fl_Slider* framesPerSec, int x, int y, int w, int h) : Fl_Light_Button(x,y,w,h,"Play")
{	
	Fl::add_idle((void (*)(void*))runButtonCB,this);
	slider = time;
	framesPerSecSlider = framesPerSec;	
	lastRedraw = 0;
}
RunButton::~RunButton()
{
	Fl::remove_idle((void (*)(void*))runButtonCB,this);
}
