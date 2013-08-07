#include "boid.h"

int Boid::nextid = 0;

Boid::Boid()
{
	float posX, posY, posZ;

	name = NULL;

	storeHistory = false;

	posX = (float)(50 * rand() / (float)RAND_MAX);
	posY = 1.0f;
	posZ = (float)(50 * rand() / (float)RAND_MAX);

	colorR = (float)(1.0f * rand() / (float)RAND_MAX);
	colorG = (float)(1.0f * rand() / (float)RAND_MAX);
	colorB = (float)(1.0f * rand() / (float)RAND_MAX);
	
	_posIndex = 0;

	radius = 0.25; //sized for rats

	//Give each boid a random history (note: the velocity is probably going to be really high)
	for (int i = 0; i <= 3; i++)
	{
		tick();
		setPos(X, posX);
		setPos(Y, posY);
		setPos(Z, posZ);
	}

	//assign an id
	this->nextid++;
	_id = this->nextid;
}

Boid::~Boid()
{
	clearHistory();
	delete name;
}
  
float Boid::pos(Axis axis) { 
	if ((axis < X) || (axis > Z)) 
	{
		assert("Invalid axis in boid.pos()");
	}
	return _getPos(axis, -1);
}

void Boid::setPos(Axis axis, float val) {
	_pos[_posIndex * 3 + axis] = val;
}

float Boid::vel(Axis axis)
{
	return (_getPos(axis, -1) - _getPos(axis, -2));
}

float Boid::acc(Axis axis)
{	
	float curVel = (_getPos(axis, -1) - _getPos(axis, -2));
	float prevVel = (_getPos(axis, -2) - _getPos(axis, -3));

	return (curVel - prevVel);
}

float *Boid::computeVel(float newpos[3])
{
	float *vel = new float[3];

	for (int i = 0; i <= 2; i++)
	{
		vel[i] = (newpos[i] - _getPos((Axis)i, -1));
	}
	return vel;
}

float *Boid::matchAcc(float acc[3])
{
	float *newPos = new float[3];

	for (int i = 0; i <= 2; i++)
		newPos[i] = acc[i]  + _getPos((Axis)i, -2);

	return newPos;
}

float *Boid::matchVel(float vel[3])
{
	float *newPos = new float[3];

	for (int i = X; i <= Z; i++)
	{
		newPos[i] = vel[i] + _getPos((Axis)i, -1);
	}
	return newPos;
}

void Boid::tick()
{
	Point *b = new Point();

	if (storeHistory)
	{
		b->x = _getPos(X, 0);
		b->y = _getPos(Y, 0);
		b->z = _getPos(Z, 0);
		history.push_back(b);
	}
	_posIndex++;
	if (_posIndex > 3) _posIndex = 0;
}
				
void Boid::clearHistory()
{
	for (int i = 0; i < history.size(); i++)
	{
		delete(history[i]);
	}
	history.clear();
}