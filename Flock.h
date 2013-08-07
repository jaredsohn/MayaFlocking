#ifndef __FLOCK_H
#define __FLOCK_H 1

#include "boid.h"
#include <vector>

class Flock
{
public:
	std::vector<Boid *> boids;
	std::vector<Point *> obstacles;
	std::vector<Point *> targets;

	Flock();
	~Flock();
	float maxVel;
	float maxAcc;
	float avoidDist;
	float neighborDist;

	float avoidWeight;
	float headingWeight;
	float centroidWeight;
	float targetWeight;

	void getCentroid(float *centroid);

	void clear();
	void tick();

	bool ignoreY; // set to true if you don't want stuff to float
private:
	float *avoidBoids(Boid *boid);
	float *approachTargets(Boid *boid);
	float *computeCentroidVel(Boid *boid);
	float *computeVelocities(Boid *boid);
	float solveMagnitude(float magnitude, float acc[3], float newacc[3]);
	float getDist2(Boid *boid1, Boid *boid2);
	bool areNeighbors(Boid *boid1, Boid *boid2);
};

#endif