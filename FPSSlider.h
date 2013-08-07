#ifndef FPSSLIDER_H
#define FPSSLIDER_H

#include <Fl/Fl_Value_Slider.H>

class FPSSlider : public Fl_Value_Slider {
public:		
	FPSSlider(Fl_Widget* draw, int length, int x, int y, int w,int h);
};

#endif
