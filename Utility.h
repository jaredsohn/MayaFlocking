/**********************************
Utility.h
created by Bob Marinier 11/30/00
cs559, project3

This file contains widely used methods that do not
lend themselves to belonging to a particular class
 *
 * $Id: Utility.h,v 1.1 2002/03/24 01:02:54 sohn Exp $
************************************/

#if !defined(_UTILITY_H)
#define _UTILITY_H

#include <math.h>
#include <GL/glut.h>
#include <sys/time.h>
#include <sys/timeb.h>

//this calculates the distance between two points
//each of the pointers passed in should be to 3x1 arrays of doubles
GLfloat Distance3(const GLfloat* p1, const GLfloat* p2);

double Distance3(const double* p1, const double* p2);

//computes the cross product of two vectors
GLfloat* crossProd(const GLfloat* v, const GLfloat* w);

//computes the cross product of two vectors
double* crossProd(const double* v, const double* w);

//computes the dot product of two vectors
GLfloat dotProd(const GLfloat v0, const GLfloat v1, const GLfloat v2, const GLfloat* w);

//makes the magnitude of the vector 1
void normalize(GLfloat* v);

void normalize(double* v);

//Rounds a number to the nearest integer
//rounds to the nearest integer
//d<.5 round down, d>=.5 round up
long Round(const float d);

#if defined(__unix__) || defined(__hpux)
#define _timeb timeb
#define _ftime(tb) \
	do { \
		struct timeval tv; \
		gettimeofday(&tv, NULL); \
		(tb)->time = tv.tv_sec; \
		(tb)->millitm = tv.tv_usec / 1000; \
		(tb)->timezone = 0; \
		(tb)->dstflag = 0; \
	} while(0)
#endif

//gets the difference between two times, accurate to a millisecond
//the returned value is a float in seconds
float diffTime(const timeb* f, const timeb* s);

//a replacement for strtok()
char **dsplit(const char *string, const char *delims);
void destroy_tokens(char **tok_array);


/*
 * bounded random value, a <= r <= b
 */
float brand(float a, float b);


#define MAX_TOKENS 50 /* change to taste */ 

#endif


