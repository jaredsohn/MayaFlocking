#include "FPSSlider.H"
#include <Fl/Fl.h>

// the guts of the slider - 
//void runSliderCB(Fl_Widget*, Fl_Widget* o)
//{
//	o->damage(1);
//}

FPSSlider::FPSSlider(Fl_Widget* dr, int length, int x, int y, int w, int h) : Fl_Value_Slider(x,y,w,h)
{

	// set up slider parameters
	value(0);
	range(0,length);
	step(1);
	type(5);				// horizontal, "nice"
	when(FL_WHEN_CHANGED);	// update whenever it changes

	//callback((Fl_Callback*) runSliderCB, dr);
}