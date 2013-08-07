#ifndef Utils3D_H
#define Utils3D_H

// some useful routines for writing 3D interactive programs
// written for CS638 - Michael Gleicher, November 1999

// extended in October 2001, to include arcball support

// a simple way to move a camera around
// mx,my is the amount the mouse moved
// l(x,y,z) is the lookat point
// e(x,y,z) is the eye point
// r(x,y,z) is the new resulting eye point
// you should really use my ArcBall class instead - it is much better
// (no singularities, ...)
void orbit(int mx, int my, double lx, double ly, double lz, 
						   double ex, double ey, double ez,
						   double& rx, double& ry, double& rz);

// draw a cube
// centered at x,y,z
// sides of length l
void drawCube(double x, double y, double z, double l);
void drawArrow(double x, double y, double z, double size);

// multiply a "shadow matrix" onto the stack to smash the objects onto
// the floor
void shadowMatrix();

#endif