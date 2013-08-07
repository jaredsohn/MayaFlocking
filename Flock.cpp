#include "Flock.h"
#include <math.h>

Flock::Flock()
{
	neighborDist = 200;
	avoidWeight = 0.1f;
	headingWeight = 0.125f;
	centroidWeight = 0.01f;
	targetWeight = 0.1f;

	ignoreY = true;
}

void Flock::tick()
{	
	float *headingVel;
	float *centroidVel;
	float *avoidVel;
	float *targetVel;
	
	float *newpos;
	float temp[3];
	float acc[3];
	float newacc[3];
	float magnitude;
	float weight;

	for (int i = 0; i < boids.size(); i++)
	{
		centroidVel = NULL;
		headingVel = NULL;
		avoidVel = NULL;
		targetVel = NULL;
		
		//Compute velocities
		centroidVel = computeCentroidVel(boids[i]);		
		headingVel = computeVelocities(boids[i]); 
		avoidVel = avoidBoids(boids[i]);
		targetVel = approachTargets(boids[i]);
	
		// Goal #1 - Obstacle Avoidance
		for (int j = X; j<=Z; j++)
		{
			newacc[j]  = (avoidWeight * avoidVel[j]);			
			acc[j] = newacc[j];
		}		
		magnitude = sqrt(acc[0] * acc[0] + acc[1] * acc[1] + acc[2] * acc[2]);
		if (magnitude > maxAcc)
		{
			for (j = X; j<=Z; j++)
			{
				acc[j] -= newacc[j];
			}
			weight = solveMagnitude(maxAcc, acc, newacc);			
			//cerr << "weight = " << weight << "...\n";
			for (j = X; j<=Z; j++)
			{
				acc[j] += weight * newacc[j];					
			}
			magnitude = maxAcc;
		}
		
		// Goal #2 - Match heading
		if (magnitude < maxAcc)
		{
			for (j = X; j<=Z; j++)
			{
				newacc[j] = headingWeight * (headingVel[j] - boids[i]->vel((Axis)j));
				acc[j] += newacc[j];
			}
			magnitude = sqrt(acc[0] * acc[0] + acc[1] * acc[1] + acc[2] * acc[2]);			
			if (magnitude > maxAcc)
			{ 
				for (j = X; j<=Z; j++)
				{
					acc[j] -= newacc[j];
				}
				weight = solveMagnitude(maxAcc, acc, newacc);
				for (j = X; j<=Z; j++)
				{
					acc[j] += weight * newacc[j];					
				} 
				magnitude = maxAcc; 
			}
		}
		
		// Goal #3 - Approach centroid
		if ((magnitude < maxAcc) && (centroidVel != NULL))
		{
			for (j = X; j<=Z; j++)
			{
				newacc[j] = centroidWeight * centroidVel[j];
				acc[j] += newacc[j];
			}
			magnitude = sqrt(acc[0] * acc[0] + acc[1] * acc[1] + acc[2] * acc[2]);
			if (magnitude > maxAcc)
			{
				for (j = X; j<=Z; j++)
				{
					acc[j] -= newacc[j];
				}
				weight = solveMagnitude(maxAcc, acc, newacc);
				for (j = X; j<=Z; j++)
				{
					acc[j] += weight * newacc[j];					
				}
			}
		}

		// If acceleration too high, decrease it
		if (magnitude > maxAcc)
		{
			cerr << "Acceleration magnitude of " << magnitude << " was too big!\n";
			acc[0] = acc[0] / magnitude * maxAcc;
			acc[1] = acc[1] / magnitude * maxAcc;
			acc[2] = acc[2] / magnitude * maxAcc;
		}
	
		// Goal #4 - approach target(s)
		if (magnitude < maxAcc)
		{
			for (j = X; j<=Z; j++)
			{
				newacc[j] = targetWeight * (targetVel[j] - boids[i]->vel((Axis)j));
				acc[j] += newacc[j];
			}			
			magnitude = sqrt(acc[0] * acc[0] + acc[1] * acc[1] + acc[2] * acc[2]);			
			cerr << "magnitude = " << magnitude << "\n";
			if (magnitude > maxAcc)
			{ 
				for (j = X; j<=Z; j++)
				{
					acc[j] -= newacc[j];
				}
				weight = solveMagnitude(maxAcc, acc, newacc);
				for (j = X; j<=Z; j++)
				{
					acc[j] += weight * newacc[j];					
				} 
				magnitude = maxAcc; 
			}
		}

		cerr << "acc = " << acc[0] << ", " << acc[1] << ", " << acc[2] << "\n";

		for (j = X; j <= Z; j++)
		{
			if ((j == Y) && (ignoreY == true))
			{
				temp[j] = boids[i]->vel((Axis)j);
			} else
			{
				temp[j] = boids[i]->vel((Axis)j) + acc[j];
			}
		}
		
		magnitude = sqrt(temp[0] * temp[0] + temp[1] * temp[1] + temp[2] * temp[2]);

		// Cap the velocity's magnitude to MAXVEL
		if (magnitude > maxVel)
		{
			cerr << "Velocity magnitude of " << magnitude << " was too big!\n";
			temp[0] = temp[0] / magnitude * maxVel;
			temp[1] = temp[1] / magnitude * maxVel;
			temp[2] = temp[2] / magnitude * maxVel;
		}

				
		newpos = boids[i]->matchVel(temp); 
		for (j = 0; j <= 2; j++)
		{					
			boids[i]->setPos((Axis)j, newpos[j]);
		}
		delete newpos;
		delete centroidVel;
		delete headingVel;
		delete avoidVel;
		delete targetVel;
	}

	for (i = 0; i < boids.size(); i++)
	{
		boids[i]->tick();
	}
}

// This will return true if boid1 and boid2 are neighbors. This is determined by
// checking if their distance squared is less than or equal to some amount
bool Flock::areNeighbors(Boid *boid1, Boid *boid2)
{
	if (boid1->id() == boid2->id()) return false;

	return ((
		(boid1->pos(X) - boid2->pos(X))*(boid1->pos(X) - boid2->pos(X)) + 
		(boid1->pos(Y) - boid2->pos(Y))*(boid1->pos(Y) - boid2->pos(Y)) + 
		(boid1->pos(Z) - boid2->pos(Z))*(boid1->pos(Z) - boid2->pos(Z))) <= neighborDist * neighborDist);
}

// Return the square distance between two boids
float Flock::getDist2(Boid *boid1, Boid *boid2)
{
	return ((
		(boid1->pos(X) - boid2->pos(X))*(boid1->pos(X) - boid2->pos(X)) + 
		(boid1->pos(Y) - boid2->pos(Y))*(boid1->pos(Y) - boid2->pos(Y)) + 
		(boid1->pos(Z) - boid2->pos(Z))*(boid1->pos(Z) - boid2->pos(Z))));
}

// Compute the location of the centroid for neighboring boids
// Note: we get rounding errors here
float *Flock::computeCentroidVel(Boid *boid)
{
	float *totals = new float[3];
	float *newtotals;
	int numNeighbors = 0;
	for (int i = X; i <= Z; i++) totals[i] = 0;

	for (i = 0; i < boids.size(); i++)
	{
		if (areNeighbors(boid, boids[i]))
		{
			numNeighbors++;
			totals[X] += boids[i]->pos(X);
			totals[Y] += boids[i]->pos(Y);
			totals[Z] += boids[i]->pos(Z);
		}
	}
	if (numNeighbors)
	{
		totals[X] /= numNeighbors;
		totals[Y] /= numNeighbors;
		totals[Z] /= numNeighbors;
	} else
	{
		return NULL;
	}

	newtotals = boid->computeVel(totals);

	return newtotals;
}

//We assume that totals is already initialized to an
//array of at least size 3.
void Flock::getCentroid(float *totals)
{
	int numNeighbors = 0;

	for (int i = X; i <= Z; i++) totals[i] = 0;

	for (i = 0; i < boids.size(); i++)
	{
		numNeighbors++;
		totals[X] += boids[i]->pos(X);
		totals[Y] += boids[i]->pos(Y);
		totals[Z] += boids[i]->pos(Z);
	}
	if (numNeighbors)
	{
		totals[X] /= numNeighbors;
		totals[Y] /= numNeighbors;
		totals[Z] /= numNeighbors;
	}
}

// Compute the total velocity that a boid should move for object avoidance
float *Flock::avoidBoids(Boid *boid)
{
	float *totals = new float[3];
	float dist2;
	int numNeighbors = 0;
	for (int i = X; i <= Z; i++) totals[i] = 0;

	for (i = 0; i < boids.size(); i++)
	{
		dist2 = getDist2(boid, boids[i]);
		if (dist2 < ((avoidDist + boid->radius + boids[i]->radius) * (avoidDist + boid->radius + boids[i]->radius)))
		{
			//FIXME...should not include radii in totals
			// should figure out portion of radius going in each direction based on orientations
			// this will make collisions more realistic (right now, they bounce off too quickly)
			// We should do the same for obstacles
			//
			//For now this is not a problem, but this is radius dependent
			totals[X] += boid->pos(X) - boids[i]->pos(X); // - 2 * boid->radius;
			totals[Y] += boid->pos(Y) - boids[i]->pos(Y); // - 2 * boid->radius;
			totals[Z] += boid->pos(Z) - boids[i]->pos(Z); // - 2 * boid->radius;
		}
	}
	
	for (i = 0; i < obstacles.size(); i++)
	{
		dist2 = (boid->pos(X) - obstacles[i]->x) * (boid->pos(X) - obstacles[i]->x) + 
				(boid->pos(Y) - obstacles[i]->y) * (boid->pos(Y) - obstacles[i]->y) + 
				(boid->pos(Z) - obstacles[i]->z) * (boid->pos(Z) - obstacles[i]->z);	
		if (dist2 < (boid->radius + obstacles[i]->radius + avoidDist) * (boid->radius + obstacles[i]->radius + avoidDist))
		{			
			totals[X] += 10*(boid->pos(X) - obstacles[i]->x); //- obstacles[i]->radius - boid->radius;
			totals[Y] += 10*(boid->pos(Y) - obstacles[i]->y);		
			totals[Z] += 10*(boid->pos(Z) - obstacles[i]->z);
		}
	}

	return totals;
}

float *Flock::approachTargets(Boid *boid)
{
	float *totals = new float[3];
	float dist;
	float targetVel[3];

	for (int i = 0; i < 3; i++) totals[i] = 0;
	
	for (i = 0; i <= 2; i++)
	{
		targetVel[i] = maxVel / 2.0f;
	}
	
	for (i = 0; i < targets.size(); i++)
	{
		dist = sqrt(((targets[i]->x - boid->pos(X)) * (targets[i]->x - boid->pos(X))) + ((targets[i]->y - boid->pos(Y)) * (targets[i]->y - boid->pos(Y))) + ((targets[i]->z - boid->pos(Z)) * (targets[i]->z - boid->pos(Z))));
		totals[0] += ((targets[i]->x - boid->pos(X)) / dist * targetVel[0]);
		totals[1] += ((targets[i]->y - boid->pos(Y)) / dist * targetVel[1]);
		totals[2] += ((targets[i]->z - boid->pos(Z)) / dist * targetVel[2]);
	}

	cerr << "target totals = " << totals[0] << " " << totals[1] << " " << totals[2] << "\n";

	return totals;
}

/*
// Compute the average velocity of all neighboring boids
// this uses a weighted average where boids that are farther away have exponentially less influence
float *computeVelocities(Boid *boid)
{
	float *totals = new float[3];
	float denom = 0;
	float dist;
	double weight = 0;
	for (int i = X; i <= Z; i++) totals[i] = 0;

	for (i = 0; i < boids.size(); i++)
	{
		if (areNeighbors(boid, boids[i]))
		{
			dist = sqrt(getDist2(boid, boids[i]));

			if (dist < 0.1)
			{
				weight = exp(10/0.1);
			} else
			{
				weight = exp(10/dist);
			}
			cerr << "weight="<<weight<<"\n";
			denom += weight;
			totals[X] += weight * boids[i]->vel(X);
			totals[Y] += weight * boids[i]->vel(Y);
			totals[Z] += weight * boids[i]->vel(Z);
		}
	}
	if (denom)
	{
		totals[X] /= denom;
		totals[Y] /= denom;
		totals[Z] /= denom;
	}

	return totals;
}
*/

// Compute the average velocity of all neighboring boids
float *Flock::computeVelocities(Boid *boid)
{
	float *totals = new float[3];
	int numNeighbors = 0;
	for (int i = X; i <= Z; i++) totals[i] = 0;

	for (i = 0; i < boids.size(); i++)
	{
		if (areNeighbors(boid, boids[i]))
		{
			numNeighbors++;
			totals[X] += boids[i]->vel(X);
			totals[Y] += boids[i]->vel(Y);
			totals[Z] += boids[i]->vel(Z);
		}
	}
	if (numNeighbors)
	{
		totals[X] /= numNeighbors;
		totals[Y] /= numNeighbors;
		totals[Z] /= numNeighbors;
	}
	return totals;
}

//Here, we solve a quadratic to determine the weight for newacc so
//that the magnitude of acc equals magnitude

//[(x1 - kx2)^2 + (y1 - ky2)^2 + (z1 - kz2)^2] = magnitude^2
//
//solve for k
float Flock::solveMagnitude(float magnitude, float acc[3], float newacc[3])
{
	float weight;
	float total;
	float a,b,c;

	total = sqrt(acc[0]*acc[0] + acc[1]*acc[1] + acc[2]*acc[2]);
	if (total < 0.01) //If current acceleration is really small, then we will ignore it
	{
		total = sqrt(newacc[0]*newacc[0] + newacc[1]*newacc[1] + newacc[2]*newacc[2]);
		if (total != 0)
		{			
			weight = magnitude / total;
			//cerr << "weight = " << weight << ", total = " << total << ", magnitude = " << magnitude << "\n";
		} else
		{
			weight = 1;
		}
	}
	else
	{
		c = (acc[0] * acc[0] + acc[1] * acc[1] + acc[2] * acc[2]) - (magnitude * magnitude);
		b = -2 * (acc[0] * newacc[0] + acc[1] * newacc[1] + acc[2] * newacc[2]);
		a = (newacc[0] * newacc[0] + newacc[1] * newacc[1] + newacc[2] * newacc[2]);

		//cerr << " a= " << a << " b = " << b << " c = " << c;
		weight = (-b + sqrt(b*b - 4*a*c))/(2*a);

		if (weight > 1) weight = 1;	
	}

	//cerr << "weight = " << weight << " discriminant = " << sqrt(b*b - 4*a*c) << "\n";
	//cerr << "acc = " << acc[0] << ", " << acc[1] << ", " << acc[2] << "...\n";
	//cerr << "newacc = " << newacc[0] << ", " << newacc[1] << ", " << newacc[2] << "...\n";

	return weight;
};

Flock::~Flock()
{
	clear();
}

void Flock::clear()
{
	for (int i = 0; i < targets.size(); i++)
	{
		delete targets[i];
	}
	targets.clear();

	for (i = 0; i < boids.size(); i++)
	{
		delete boids[i];
	}
	boids.clear();
	
	for (i = 0; i < obstacles.size(); i++)
	{
		delete obstacles[i];
	}
	obstacles.clear();
}