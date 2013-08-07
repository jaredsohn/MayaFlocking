// some useful routines for writing 3D interactive programs
// written for CS638 - Michael Gleicher, November 1999
#include "3DUtils.H"
#include "math.h"
#include <windows.h>
#include <GL/gl.h>

#include <Fl/Fl.h>

// a quick and dirty hack for rotating the eyepoint
// note: although this is modeled after the virtual sphere, its
// much simple and (probably) not as good
// it takes a lookat/eye pair and moves the eye around the lookat point
//	based on some wierd notions of how far to go, given how far the mouse moved
//  from the beginning
// the fundamental flaw of this method is that it rotates around fixed axes
// so, in a way, its instructive to use it to learn about gimbal lock
// I often find that I cannot rotate the world the way that i want with this 
// interface
void orbit(int mx, int my, double lx, double ly, double lz, 
						   double ex, double ey, double ez,
						   double& rx, double& ry, double& rz)
{
	// 
	// first - if the mouse motion is small, return to starting position
	// this should make it easier to move around 1 axis
	if (mx<0) {
		mx = mx+5;
		if (mx>0) mx = 0;
	} else {
		mx = mx-5;
		if (mx<0) mx = 0;
	}
	if (my<0) {
		my = my+5;
		if (my>0) my = 0;
	} else {
		mx = mx-5;
		if (my<0) my = 0;
	}
	// make the center of the coordinate system be the lookat point so that's
	// where we rotate about
	ex -= lx;
	ey -= ly;
	ez -= lz;
	// horizontal motions rotate around the vertical (y axis) by an amount
	// proportional to the amount the mouse moved - the amount is pretty
	// arbitrary
	double hz = ((double) mx) / 200;
	double sx = sin(hz);
	double cx = cos(hz);
	rx = cx * ex - sx * ez;
	rz = sx * ex + cx * ez;
	// note: rotating around the z (or x) axis isn't really right, but its
	// easy
	double vt = ((double) my) / 200;
	double sz = sin(vt);
	double cz = cos(vt);
	ex = rx;	// for safe keeping
	rx = cz * ex - sz * ey;
	ry = sz * ex + cz * ey;

	// shift the result back so that the lookat point is in the right place
	rx += lx;
	ry += ly;
	rz += lz;
}

void drawArrow(double x, double y, double z, double size){
	glPushMatrix();
	glTranslated(x,y,z);
	glBegin(GL_TRIANGLES);
		glNormal3d(0,1,0);

		glColor3d(1.0,0.0,0.0);
		glVertex3d(0.0,size,0.0);
		glColor3d(0.6,0.2,0.8);
		glVertex3d(size/4,0.0,size/4);
		glVertex3d(size/4,0.0,-size/4);
		
		glColor3d(1.0,0.0,0.0);
		glVertex3d(0.0,size,0.0);
		glColor3d(0.6,0.2,0.8);
		glVertex3d(-size/4,0.0,-size/4);
		glVertex3d(size/4,0.0,-size/4);

		glColor3d(1.0,0.0,0.0);
		glVertex3d(0.0,size,0.0);
		glColor3d(0.6,0.2,0.8);
		glVertex3d(-size/4,0.0,size/4);
		glVertex3d(-size/4,0.0,-size/4);

		glColor3d(1.0,0.0,0.0);
		glVertex3d(0.0,size,0.0);
		glColor3d(0.6,0.2,0.8);
		glVertex3d(-size/4,0.0,size/4);
		glVertex3d(size/4,0.0,size/4);
	glEnd();

	glBegin(GL_QUADS);
		glColor3d(0.4,0.0,0.6);
		glVertex3d(size/4,0.0,size/4);
		glVertex3d(size/4,0.0,-size/4);
		glVertex3d(-size/4,0.0,-size/4);
		glVertex3d(-size/4,0.0,size/4);
	glEnd();

	glPopMatrix();
}

// draw a cube
// note: this isn't necesarily the fastest way since I recompute each
// vertex
// also, notice that I don't keep all my polygons with the same orientations!
void drawCube(double x, double y, double z, double l)
{
	// we'll use a transform to move the cube to the right place, however
	// we won't use it for the scale since that might mess up lighting
	glPushMatrix();
	glTranslated(x,y,z);
	glBegin(GL_QUADS);
	glNormal3d( 0,0,1);
	glVertex3d( l/2, l/2, l/2);
	glVertex3d(-l/2, l/2, l/2);
	glVertex3d(-l/2,-l/2, l/2);
	glVertex3d( l/2,-l/2, l/2);
	glNormal3d( 0,0,-1);
	glVertex3d( l/2, l/2, -l/2);
	glVertex3d(-l/2, l/2, -l/2);
	glVertex3d(-l/2,-l/2, -l/2);
	glVertex3d( l/2,-l/2, -l/2);
	glNormal3d( 0,1,0);
	glVertex3d( l/2, l/2, l/2);
	glVertex3d(-l/2, l/2, l/2);
	glVertex3d(-l/2, l/2,-l/2);
	glVertex3d( l/2, l/2,-l/2);
	glNormal3d( 0,-1,0);
	glVertex3d( l/2,-l/2, l/2);
	glVertex3d(-l/2,-l/2, l/2);
	glVertex3d(-l/2,-l/2,-l/2);
	glVertex3d( l/2,-l/2,-l/2);
	glNormal3d( 1,0,0);
	glVertex3d( l/2, l/2, l/2);
	glVertex3d( l/2,-l/2, l/2);
	glVertex3d( l/2,-l/2,-l/2);
	glVertex3d( l/2, l/2,-l/2);
	glNormal3d(-1,0,0);
	glVertex3d(-l/2, l/2, l/2);
	glVertex3d(-l/2,-l/2, l/2);
	glVertex3d(-l/2,-l/2,-l/2);
	glVertex3d(-l/2, l/2,-l/2);
	glEnd();
	glPopMatrix();
}

// a shadow matrix multiplied onto the stack
// notice: we smash the y value to near 0, but not quite
// the shadows actually float slightly above the floor
// the is a really crummy hack, but it avoids problems with the Z-buffer
// there are better ways to do this
// the matrix is:
//	1  0  0  0
//  0  0  0  .01	--- .01 is the float
//  0  0  1  0
//  0  0  0  1
//double sm[16] = {1,0,0,0, 0,0,0,0, 0,0,1,0, 0,.01,0,1};
double sm[16] = {1,0,0,0, 
				 0,0,0,0, 
				 0,0,1,0, 
				 0,.01,0,1};
void shadowMatrix()
{
	glMultMatrixd(sm);
}


