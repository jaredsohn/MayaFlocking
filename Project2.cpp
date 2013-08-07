/* Flocking
 * Written by Jared Sohn (sohn@cs.wisc.edu)
 * CS838, UW-Madison
 * May 2002
 *
 * See http://www.cs.wisc.edu/~cs838-1/Students/sohn/p2/flocking.html
 * for documentation on this program and the accompanying Maya plugins.
 *
 * Much of the 3D framework is from CS838 Project 1 by Jared Sohn and
 * Chris Mizerak.  Further, that project was based on code from Mike Gleicher.
 */

#include <FL/Fl_Gl_Window.h>
#include <Fl/Fl.h>
#include <FL/Fl_Box.h>
#include <Fl/Fl_Double_Window.h>
#include <Fl/Fl_Button.h>
#include <Fl/Fl_Menu_Bar.h>
#include <FL/Fl_Menu_Item.H>
#include <FL/fl_file_chooser.H>
#include <FL/Fl_Menu_Button.H>
#include "stdio.h"
#include "RunButton.h"
#include "MyGlWindow.h"

#define WINDOW_WIDTH  640
#define WINDOW_HEIGHT 480

#define MENU_NONE			 0
#define MENU_EXIT            1

#define MAX_MENU_ITEMS 100

int numFrames;
MyGlWindow* gl;
char *fileName;
Fl_Menu_Bar* mainMenu;
Fl_Menu_Item FileMenu[MAX_MENU_ITEMS];

void resetBoids(Fl_Widget *w, MyGlWindow* wnd){
	wnd->initBoids();
	wnd->damage(1);
}

void setMenu(int numSkeletons){
	FileMenu[0].text = "&File";
		FileMenu[0].shortcut_ = FL_ALT+'f';
		FileMenu[0].flags = FL_SUBMENU;
		FileMenu[0].user_data_ = (void *)MENU_NONE;

		FileMenu[1].text = "E&xit";
		FileMenu[1].shortcut_ = FL_ALT+'x';
		FileMenu[1].callback_ = mainMenu->callback();
		FileMenu[1].user_data_ = (void *)MENU_EXIT;

		mainMenu->menu(FileMenu);
}

void menuFunction(Fl_Widget *w, void* menuSelect){
	
	int menuChoice = (int)menuSelect;

	if( menuChoice==MENU_EXIT )
	{
		exit(0);
	}
}

main()
{	
	Fl_Value_Slider *slider;
	numFrames=100;

	Fl_Double_Window* wind = new Fl_Double_Window(10,10,WINDOW_WIDTH+160,WINDOW_HEIGHT+70,"CS838 Project #2 - Flocking -- By Jared Sohn - Spring 2002");

	wind->begin();		// put widgets inside of the window
		
		gl = new MyGlWindow(150,25,WINDOW_WIDTH,WINDOW_HEIGHT);
		gl->mode( FL_RGB | FL_DOUBLE | FL_ALPHA | FL_DEPTH | FL_STENCIL );

		//Menu Bar
		mainMenu = new Fl_Menu_Bar(0,0,WINDOW_WIDTH+160,20,"Menu");
			
		mainMenu->callback((Fl_Callback*) menuFunction, gl);

		setMenu(0);

		//Main Timeline Slider & Button
		RunSlider* rt = new RunSlider(gl,numFrames-1,230,WINDOW_HEIGHT+35,550,25);
		gl->timeSlider = rt;

		FPSSlider* fps = new FPSSlider(gl,120,10,WINDOW_HEIGHT+35,120,25);		
		gl->fSlider = fps;

		slider = new Fl_Value_Slider(5,30,140,25,"Maximum Velocity");
		slider->value(3);
		slider->range(0,25);
		slider->step(0.25);
		slider->type(5);
		slider->when(FL_WHEN_CHANGED);
		gl->velSlider = slider;

		slider = new Fl_Value_Slider(5,80,140,25,"Maximum Acceleration");
		slider->value(1.0);
		slider->range(0,25);
		slider->step(0.25);
		slider->type(5);
		slider->when(FL_WHEN_CHANGED);
		gl->accSlider = slider;

		slider = new Fl_Value_Slider(5,130,140,25,"Avoidance Distance");
		slider->value(1.5);
		slider->range(0,10);
		slider->step(0.25);
		slider->type(5);
		slider->when(FL_WHEN_CHANGED);
		gl->avoidSlider = slider;

		slider = new Fl_Value_Slider(5,180,140,25,"Neighbor Distance");
		slider->value(200.0);
		slider->range(0,250);
		slider->step(1.0);
		slider->type(5);
		slider->when(FL_WHEN_CHANGED);
		gl->neighborDistSlider = slider;

		slider = new Fl_Value_Slider(5,300,140,25,"Avoidance Weight");
		slider->value(0.1);
		slider->range(0,1);
		slider->step(0.01);
		slider->type(5);
		slider->when(FL_WHEN_CHANGED);
		gl->weightAvoid = slider;

		slider = new Fl_Value_Slider(5,350,140,25,"Centering Weight");
		slider->value(0.01);
		slider->range(0,1);
		slider->step(0.01);
		slider->type(5);
		slider->when(FL_WHEN_CHANGED);
		gl->weightCentering = slider;

		slider = new Fl_Value_Slider(5,400,140,25,"Vel Matching Weight");
		slider->value(0.13);
		slider->range(0,1);
		slider->step(0.01);
		slider->type(5);
		slider->when(FL_WHEN_CHANGED);
		gl->weightVelMatch = slider;

		slider = new Fl_Value_Slider(5,450,140,25,"Target Weight");
		slider->value(0.1);
		slider->range(0,1);
		slider->step(0.01);
		slider->type(5);
		slider->when(FL_WHEN_CHANGED);
		gl->weightTarget = slider;

		//Buttons
		Fl_Button* resetButton = new Fl_Button(5,230,140,25,"Reset Simulation");
		resetButton->callback((Fl_Callback*) resetBoids, gl);

		RunButton* rb = new RunButton(rt,fps,140,WINDOW_HEIGHT+35,80,25);		

		Fl_Box* fpsLabel = new Fl_Box(FL_NO_BOX,10,WINDOW_HEIGHT+15,80,25, "");
		fpsLabel->label("Frames/Sec:");

	wind->end();

	wind->show();	// this actually opens the window

	Fl::run();
	delete wind;

	return 1;
}