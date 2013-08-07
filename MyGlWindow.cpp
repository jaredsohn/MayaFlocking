#include <Fl/Fl.h>
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <Fl/Fl_Double_Window.h>
#include <Fl/Fl_Button.h>
#include "stdio.h"
#include "math.h"
#include "3DUtils.H"
#include "MyArcBall.H"
#include "MyGlWindow.h"
#include "Flock.h"

static int button = -10;
MyGlWindow::MyGlWindow(int x, int y, int w, int h):Fl_Gl_Window(x,y,w,h){  

  camZoom=25.0;
  camY=120.0f;
  camX=20.0f;
  lookatX = 0.0f;
  lookatY = 0.0f;
  lookatZ = 0.0f;
  selectedObject=-1;
  
  lightX=0.0f;
  lightY=200.0f;
  lightZ=0.0f;

  centroid[0] = 0;
  centroid[1] = 0;
  centroid[2] = 0;

  //Light points down
  lightDirX=0.0f;
  lightDirY=-1.0f;
  lightDirZ=0.0f;

  init();

  drawing = false;
}

int MyGlWindow::getNumFrames(){
	return numFrames;
}

void MyGlWindow::setDrawBounds(int n){
	GUI_drawBounds=n;
}

void MyGlWindow::setDrawShadows(int n){
	GUI_drawShadows=n;
}

void MyGlWindow::setDrawReflections(int n){
	GUI_drawReflections=n;
}

void MyGlWindow::setNumFrames(int n, int newscene){
	if( (n > numFrames) || newscene){
		numFrames=n;
		timeSlider->maximum(numFrames);
	}
}

void MyGlWindow::init(){
	GUI_pickMode=PICK_PATH_POSITION;	
	GUI_showWhilePick=1;
	GUI_viewMode=DRAW_LINES;
	GUI_showPath=1;
	numFrames=1;
	currSkeleton=-1;
	GUI_camMode=CAM_MODE_MANUAL;
	GUI_drawReflections=1;
	GUI_drawShadows=1;
	GUI_drawBounds=1;

	initBoids();
}

void MyGlWindow::setShowPath(int n){
	GUI_showPath=n;
}

void MyGlWindow::setPickMode(int mode){
	GUI_pickMode=mode;
}

void MyGlWindow::setViewMode(int mode){	
	GUI_viewMode=mode;	
}

int MyGlWindow::doPicking(int mx, int my){
	return -1;
}

void MyGlWindow::getCamXYZ(float *x, float *y, float *z){	
	*x=camX;
	*y=camY;
	*z=camZoom;
}

void MyGlWindow::drawSetupTransform(bool proj_identity) {
    glMatrixMode(GL_PROJECTION);
	glViewport(0,0,w(),h());
    if ( proj_identity )
        glLoadIdentity();

	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
	//glEnable(GL_POLYGON_SMOOTH);
	//glHint(GL_POLYGON_SMOOTH_HINT, GL_DONT_CARE);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_DONT_CARE);
	glEnable(GL_NORMALIZE);
	
	gluPerspective(FOV, w()/h(), 0.1, 2000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt( camX,camY,camZoom, 0, 0, 0, 0,1,0 );	

	if( camZoom<3 ) camZoom=3;
	gluLookAt( centroid[0], centroid[1], centroid[2] - camZoom, centroid[0],centroid[1],centroid[2], 0,1,0);	
}

void MyGlWindow::drawSetupLighting(){

	float ambientLight[] =       { 0.3f, 0.3f, 0.3f, 1.0f };
	float diffuseLight[] =       { 0.3f, 0.3f, 0.3f, 1.0f };
	float spotLightPosition[] =  { lightX, lightY, lightZ, 1.0f };
	float spotLightDirection[] = { lightDirX, lightDirY, lightDirZ };
	float matAmbient[] =         { 1.0f, 1.0f, 1.0f, 1.0f };
	float matDiffuse[] =         { 0.9f, 0.9f, 0.9f, 1.0f };
	float matSpecular[] =        { 0.4f, 0.4f, 0.4f, 1.0f };
	float matShininess[] =       { 20.0f };
	float spotLightCutOff[] =    { 90.0f };
	float spotLightExpon[]  =    { 10.0f };

	glLightfv(GL_LIGHT0, GL_AMBIENT,  ambientLight);
	glLightfv(GL_LIGHT0, GL_DIFFUSE,  diffuseLight);
	glLightfv(GL_LIGHT0, GL_POSITION, spotLightPosition);
	glLightfv(GL_LIGHT0, GL_SPOT_CUTOFF, spotLightCutOff);
	glLightfv(GL_LIGHT0, GL_SPOT_EXPONENT, spotLightExpon);
	glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, spotLightDirection);

	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	glMaterialfv(GL_FRONT, GL_SPECULAR,  matSpecular);
	glMaterialfv(GL_FRONT, GL_SHININESS, matShininess);

}

void MyGlWindow::drawShadows(){
	//Draw Shadows
	glDisable(GL_LIGHTING);
	glPushMatrix();

		glTranslated(0.0, 1.51f, 0.0);

		double ground[4] = { 0.0, 1.0, 0.0, 1.0 };
		double light[4] = { lightX, lightY, lightZ, 1.0 };
		double dot;
		double shadowMat[4][4];
	
		dot = ground[0] * light[0] +
			  ground[1] * light[1] +
			  ground[2] * light[2] +
			  ground[3] * light[3];
    
		shadowMat[0][0] = dot - light[0] * ground[0];
		shadowMat[1][0] = 0.0 - light[0] * ground[1];
		shadowMat[2][0] = 0.0 - light[0] * ground[2];
		shadowMat[3][0] = 0.0 - light[0] * ground[3];
    
		shadowMat[0][1] = 0.0 - light[1] * ground[0];
		shadowMat[1][1] = dot - light[1] * ground[1];
		shadowMat[2][1] = 0.0 - light[1] * ground[2];
		shadowMat[3][1] = 0.0 - light[1] * ground[3];
    
		shadowMat[0][2] = 0.0 - light[2] * ground[0];
		shadowMat[1][2] = 0.0 - light[2] * ground[1];
		shadowMat[2][2] = dot - light[2] * ground[2];
		shadowMat[3][2] = 0.0 - light[2] * ground[3];
    
		shadowMat[0][3] = 0.0 - light[3] * ground[0];
		shadowMat[1][3] = 0.0 - light[3] * ground[1];
		shadowMat[2][3] = 0.0 - light[3] * ground[2];
		shadowMat[3][3] = dot - light[3] * ground[3];		

		glColor4f(0,0,0,0);
		glMultMatrixd((const GLdouble*)shadowMat);

	glPopMatrix();
	glEnable(GL_DEPTH_TEST);
}

void MyGlWindow::draw()
{	
  int temp = (int)timeSlider->value();
  if (temp != curFrame)
  {
	curFrame = temp;		
	flock.maxVel = (float)velSlider->value();
	flock.maxAcc = (float)accSlider->value();
	flock.avoidDist = (float)avoidSlider->value();
	flock.avoidWeight = (float)weightAvoid->value();
	flock.centroidWeight = (float)weightCentering->value();
	flock.headingWeight = (float)weightVelMatch->value();
	flock.targetWeight = (float)weightTarget->value();
	flock.neighborDist = (float)neighborDistSlider->value();

	flock.tick();

	flock.getCentroid(&centroid[0]);
	lightX = centroid[0];
	lightY = centroid[1] + 200;
	lightZ = centroid[2];
  }
	
  glClearColor(0,0,0,0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  
	
  drawSetupLighting();
  drawSetupTransform();

  //Draw Reflections
  if( GUI_drawReflections )
	drawReflections();

  if( GUI_drawShadows )
	drawShadows();
  
  drawFloor();
  drawLight();

  drawBoids();
}

void MyGlWindow::drawReflections(){
	
	glClearStencil(0x0);
	glEnable(GL_STENCIL_TEST);
	glClear(GL_STENCIL_BUFFER_BIT);
	glStencilFunc(GL_ALWAYS, 0x1, 0x1);
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

	glDisable(GL_DEPTH_TEST);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	float halfFloorWidth = FLOOR_WIDTH/2.0f;
	float halfFloorHeight = FLOOR_HEIGHT/2.0f;
	glBegin(GL_QUADS);
		glVertex3f(-halfFloorWidth, 0.0f, halfFloorHeight);
		glVertex3f( halfFloorWidth, 0.0f, halfFloorHeight);
		glVertex3f( halfFloorWidth, 0.0f,-halfFloorHeight);
		glVertex3f(-halfFloorWidth, 0.0f,-halfFloorHeight);
	glEnd();
	glEnable(GL_DEPTH_TEST);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);


	glStencilFunc(GL_EQUAL, 0x1, 0x1);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	
	glDisable(GL_STENCIL_TEST);
}

void MyGlWindow::drawLight(int name){
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
}

static int mDownX, mDownY;
static double md_ndc_x, md_ndc_y;

// where the eye started to move things around
static double eyeStartX, eyeStartY, eyeStartZ;
int MyGlWindow::handle(int e)
{
  switch(e) {
  case FL_SHOW:		// you must handle this, or not be seen!
	  show();
	  return 1;
  case FL_PUSH:
	  mDownX = Fl::event_x();	// remeber where the mouse went down
	  mDownY = Fl::event_y();
	  button = Fl::event_button();
	  switch(button) {
	  //left mouse button
	  case 1:		  
		  selectedObject=doPicking(mDownX,mDownY);
		  if( selectedObject==-1 ){			  
			  getMouseNDC(mouseDownX, mouseDownY);
		  }
		  break;
	  //middle mouse button
	  case 2:		
		getMouseNDC(mouseDownX, mouseDownY);
		break;
	  //right mouse button
	  case 3:		
		getMouseNDC(md_ndc_x,md_ndc_y);
		ball.down((float)md_ndc_x,(float)md_ndc_y);
		break;
	  };
	  damage(1);
	  return 1;
  case FL_RELEASE:
	  switch(button) {	  
	  case 3:
		  break;
	  };
	  damage(1);
	  return 1;
  case FL_DRAG: // if the user drags the mouse
	  switch(button) {
	  case 1:
		  if( selectedObject!=-1 ){			
			int mx = Fl::event_x();
			int my = Fl::event_y();			
			double mxx,myy;
			getMouseNDC(mxx,myy);
		  }else{			
			getMouseNDC(mouseDragX,mouseDragY);
			camX+=(mouseDownX-mouseDragX)*300;
			camY+=(mouseDownY-mouseDragY)*300;
			if( camY < 1.0f ) camY=1.0f;
			mouseDownX=mouseDragX;
			mouseDownY=mouseDragY;
		  }
		  damage(1);
		  break;
	  case 2:
		  double x;
		  getMouseNDC(mouseDragX,mouseDragY);		  
		  camZoom+=(mouseDownY-mouseDragY)*300;
		  mouseDownX=mouseDragX;
		  mouseDownY=mouseDragY;
		  damage(1);
		  break;
	  case 3:
		double y;
		getMouseNDC(x,y);
		ball.computeNow((float)x,(float)y);
		damage(1);
	  }
	  return 1;
  default:
	return 0;
  }
}

void MyGlWindow::drawFloor()
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);

	if( GUI_drawReflections ){
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}else{
		glDisable(GL_BLEND);
	}

	bool  color=true;
	float color1[] = { 0.0f, 1.0f, 0.5f, 0.8f };
	float color2[] = { 0.0f, 0.5f, 0.3f, 0.8f };	

	float checkerSize = 20.0f;
	float halfFloorWidth = FLOOR_WIDTH/2.0f;
	float halfFloorHeight = FLOOR_HEIGHT/2.0f;
	float tex_X0, tex_X1, tex_Z0, tex_Z1;

	if( TEXTURE_FLOOR ){
		glColor4f(1.0f, 1.0f, 1.0f, 0.8f);
	}

	float textureWidth = (checkerSize*(TEXTURE_FLOOR_REPEAT/2.0f))/(float)FLOOR_WIDTH;
		
	glPushMatrix();
		for(float i=-halfFloorWidth; i<halfFloorWidth; i+=checkerSize){			

			tex_X0 = (i+halfFloorWidth)*(textureWidth/checkerSize);
			tex_X1 = tex_X0+textureWidth;			

			glBegin(GL_TRIANGLE_STRIP);
			for(float j=-halfFloorHeight; j<halfFloorHeight; j+=checkerSize){

				if( TEXTURE_FLOOR ){
					
					color=!color;
					if( color ) glColor4f(color1[0],color1[1],color1[2],color1[3]); else glColor4f(color2[0],color2[1],color2[2],color2[3]);
					
					tex_Z0 = (j+halfFloorWidth)*(textureWidth/checkerSize);
					tex_Z1 = tex_Z0+textureWidth;

					glNormal3f(0.0f, 1.0f, 0.0f);
					glTexCoord2f(tex_X0, tex_Z0); glVertex3f(i,             0.0f, j);
					glTexCoord2f(tex_X1, tex_Z0); glVertex3f(i+checkerSize, 0.0f, j);
					glTexCoord2f(tex_X0, tex_Z1); glVertex3f(i,             0.0f, j+checkerSize);
					glTexCoord2f(tex_X1, tex_Z1); glVertex3f(i+checkerSize, 0.0f, j+checkerSize);

				}else{
					color=!color;
					if( color ) glColor4f(color1[0],color1[1],color1[2],color1[3]); else glColor4f(color2[0],color2[1],color2[2],color2[3]);
					glNormal3f(0.0f, 1.0f, 0.0f);										
					glVertex3f(i,             0.0f, j);
					glVertex3f(i+checkerSize, 0.0f, j);
					glVertex3f(i,             0.0f, j+checkerSize);
					glVertex3f(i+checkerSize, 0.0f, j+checkerSize);					
				}
			}
			glEnd();
		}		
	glPopMatrix();

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);	

}

//////////////////////////////////////////////////////////////////
// get the mouse in NDC
// note: this is inefficient since I get the window's viewport too!
void MyGlWindow::getMouseNDC(double& x, double& y)
{
	// notice, we put everything into doubles so we can do the math
	  double mx = Fl::event_x();	// remeber where the mouse went down
	  double my = Fl::event_y();

	  // we will assume that the viewport is the same as the window size
	  double wd = w();
	  double hd = h();

	  // remember that FlTk has Y going the wrong way!
	  my = hd-my;
	
	  x = (mx / wd) * 2.0 - 1;
	  y = (my / hd) * 2.0 - 1;
}


void MyGlWindow::GetWorldCoordinates( double winx, double winy, double winz,
                            double &worldx, double &worldy, double &worldz )
{
    double mvmatrix[16], projmatrix[16];
    int viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix);
    glGetDoublev(GL_PROJECTION_MATRIX, projmatrix);
    gluUnProject(   winx, winy, winz, mvmatrix,
                    projmatrix, viewport, 
                    &worldx, &worldy, &worldz );
}

void MyGlWindow::drawBoids()
{	
	//Draw obstacles
	glColor3f(0,0,0);
	for (int j = 0; j < flock.obstacles.size(); j++)
	{
		glPushMatrix();
			glDisable(GL_LIGHT0);
			glTranslatef(flock.obstacles[j]->x, flock.obstacles[j]->y, flock.obstacles[j]->z);
			glScalef(flock.obstacles[j]->radius,0,flock.obstacles[j]->radius);
			glutSolidSphere(1,10,10);
			glEnable(GL_LIGHT0);
		glPopMatrix();
	}
	//Draw Targets
	glColor3f(1.0,0,0);
	for (j = 0; j < flock.targets.size(); j++)
	{
		glPushMatrix();
			glTranslatef(flock.targets[j]->x, 0, flock.targets[j]->z);
			glScalef(8,0.5,8);
			glutSolidSphere(1,10,10);
		glPopMatrix();
	}

	//Draw boids	
	for (int i = 0; i < flock.boids.size(); i++)
	{
		glColor3f(flock.boids[i]->colorR, flock.boids[i]->colorG, flock.boids[i]->colorB);
		glPushMatrix();
			glTranslatef(flock.boids[i]->pos(X), flock.boids[i]->pos(Y), flock.boids[i]->pos(Z));			
			glutSolidSphere(1.0,20,20);
		glPopMatrix();
	}
}

void MyGlWindow::initBoids()
{
	for (int i = 0; i < flock.boids.size(); i++)
	{
		delete flock.boids[i];
	}
	flock.boids.clear();
	for (i = 0; i < 100; i++)
	{
		flock.boids.push_back(new Boid());
	}

	for (i = 0; i < flock.obstacles.size(); i++)
	{
		delete flock.obstacles[i];
	}
	flock.obstacles.clear();
	flock.obstacles.push_back(new Point(125,0.5,125, 6));
	flock.obstacles.push_back(new Point(40,0.5,30, 6));
	flock.obstacles.push_back(new Point(85,0.5,85, 6));
	flock.obstacles.push_back(new Point(60,0.5,75, 6));
	
	for (i = 0; i < flock.targets.size(); i++)
	{
		delete flock.targets[i];
	}
	flock.targets.clear();
	flock.targets.push_back(new Point(200,0.125,200));
}