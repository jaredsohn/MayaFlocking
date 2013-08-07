#ifndef __MYGLWINDOW_H
#define __MYGLWINDOW_H
#include <FL/Fl_Gl_Window.h>
#include <FL/FL_Slider.h>
#include "RunButton.h"
#include "3DUtils.H"
#include "MyArcBall.H"
#include "constants.h"
#include "Flock.h"

class AnimationPath;

class MyGlWindow : public Fl_Gl_Window
{
	private:
		int curFrame;
		ArcBall ball;

		int _refFrameRate;					

		double camZoom;
		double camX, camY;
		double lookatX, lookatY, lookatZ;
		double lightX, lightY, lightZ;
		double lightDirX, lightDirY, lightDirZ;

		int GUI_pickMode;
		int GUI_showWhilePick;
		int GUI_viewMode;
		int GUI_showPath;
		int GUI_camMode;
		int GUI_drawReflections;
		int GUI_drawShadows;
		int GUI_drawBounds;

		int numFrames;
		int currSkeleton;
		
		double mouseDownX, mouseDownY;
		double mouseDragX, mouseDragY;
		int selectedObject;

		void draw();
		void drawReflections();
		void drawLight(int name=-1);
		void drawShadows();
		int handle(int);
		void drawFloor();
		void getMouseNDC(double& x, double& y);
		int doPicking(int mx, int my);
		void drawSetupTransform(bool proj_identity=true);
		void drawSetupLighting();
		void GetWorldCoordinates( double winx, double winy, double winz, double &worldx, double &worldy, double &worldz );
		void init();
		void getCamXYZ(float *x, float *y, float *z);
		void drawSkeletons(int shadows=0, int reflect = 0);

		bool drawing;
		float centroid[3];

	public:		
		Fl_Value_Slider *velSlider;
		Fl_Value_Slider *accSlider;
		Fl_Value_Slider *avoidSlider;

		
		RunSlider *timeSlider;
		Fl_Slider *fSlider;
		MyGlWindow(int x, int y, int w, int h);

		int &refFrameRate() { return _refFrameRate; }

		void setPickMode(int mode);
		void setViewMode(int mode);
		void setShowPath(int n);
		void setCamMode(int mode);

		void setDrawShadows(int n);
		void setDrawReflections(int n);
		void setDrawBounds(int n);

		void setNumFrames(int n, int newscene=0);

		int getNumFrames();

		void drawBoids();
		void initBoids();

		Flock flock;

		Fl_Value_Slider *weightAvoid;
		Fl_Value_Slider *weightCentering;
		Fl_Value_Slider *weightVelMatch;
		Fl_Value_Slider *weightTarget;

		Fl_Value_Slider *neighborDistSlider;
};

#endif