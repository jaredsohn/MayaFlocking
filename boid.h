#if !defined(__BOID_H)
#define __BOID_H

#include <iostream.h>
#include "assert.h"
#include <vector>

enum Axis {X=0, Y=1, Z=2};

struct Point
{
	Point()
	{
		x = 0;
		y = 0;
		z = 0;
		radius = 0;
	}

	Point(float newx, float newy, float newz)
	{
		x = newx;
		y = newy;
		z = newz;
		radius = 0;
	}

	Point(float newx, float newy, float newz, float newradius)
	{		
		x = newx;
		y = newy;
		z = newz;
		radius = newradius;
	}

	float x;
	float y;
	float z;

	float radius; //This makes our point into a sphere, if desired
};

class Boid
{

public:

	bool storeHistory;
	std::vector<Point *> history;

    Boid();
	~Boid();

	float pos(Axis axis);
	float vel(Axis axis);
	float acc(Axis axis);

	void setPos(Axis axis, float val);
	
	//Determine the velocity that the boid needs to go to reach newpos
	float *computeVel(float newpos[3]);

	// Return the desired position so that the acceleration matches acc
	float *matchAcc(float acc[3]);

	// Return the desired position so that the velocity matches vel
	float *matchVel(float vel[3]);

	void tick();				 
	int & id() { return _id; }
	void clearHistory();

	static int nextid;

	char *name;

	float radius;

	float colorR, colorG, colorB;	

 protected:   

	int _id; 
	float _pos[12]; //store four intervals of data
	int _posIndex;
	

	float _getPos(Axis axis, int offset)
	{
		int cur = (3 * _posIndex) + axis + (3 * offset);
		while (cur >= 12) cur -= 12;
		while (cur < 0) cur += 12;
		return _pos[cur];
	}
};

#endif /* __BOID_H */