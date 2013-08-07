//Commandline flocking plugin
//Written by Jared Sohn (sohn@cs.wisc.edu)
//For CS838 at UW-Madison
//May 2002

#include <iostream.h>
#include <maya/MFnAnimCurve.h>
#include <maya/MTime.h>
#include <maya/MFnPlugin.h>
#include <maya/MString.h>
#include <maya/MArgList.h>
#include <maya/MPxCommand.h>
#include <maya/MGlobal.h>
#include <maya/MDagPath.h>
#include <maya/MItSelectionList.h>
#include <maya/MSelectionList.h>
#include <maya/MPlug.h>
#include <maya/MFnNurbsCurve.h>
#include <maya/MFnMotionPath.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MPxNode.h> 
#include <maya/MTypeId.h> 
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>


#include "boid.h"
#include "flock.h"

float boidMaxVel;
float boidMaxAcc;
float boidAvoidDist;

int startFrame, endFrame;

Flock flock1;


#define CHECK(s) do { if ( s.error() ) cerr << __FILE__ << ":" << __LINE__ << ":" << s.errorString().asChar()  << "\n"; } while(0)

Boid *loadBoid(MObject depnode, MTime start);
MStatus exportBoid(MObject depnode, int index);

class flock : public MPxCommand
{
public:
    MStatus        doIt( const MArgList& args );
    static void*   creator() { return new flock; }
};
 
MStatus initializePlugin( MObject obj ) {
	MFnPlugin plugin( obj, "Alias|Wavefront", "1.0", "Any" );
    plugin.registerCommand( "flock", flock::creator );
    return MS::kSuccess;
}
 
MStatus uninitializePlugin( MObject obj ) {
	MFnPlugin plugin( obj );
    plugin.deregisterCommand( "flock" );
 
	return MS::kSuccess;
}

MStatus flock::doIt( const MArgList& arglist ) {
    MStatus stat;
	MObject depnode;
	MDagPath node;
	MObject component;
	MSelectionList list;
	MFnDagNode nodeFn;
	MTime time;
	int i;
	Point *newobs;
	Boid *newboid;
	float boidRadius;
	bool abort = false;

	flock1.clear();

	//Set some default values
	startFrame = 0;
	endFrame = 0;
	boidMaxVel = 0;
	boidMaxAcc = 0;
	boidAvoidDist = 0;

	if (arglist.length() >= 10)
	{
		startFrame = (int)arglist.asDouble(0, &stat);
		endFrame = (int)arglist.asDouble(1, &stat);		
		flock1.ignoreY = (arglist.asInt(2, &stat) != 0);
		flock1.maxVel = (float)arglist.asDouble(3, &stat);
		flock1.maxAcc = arglist.asDouble(4, &stat);
		flock1.avoidDist = arglist.asDouble(5, &stat);
		boidRadius = arglist.asDouble(6, &stat);
		flock1.targets.push_back(new Point(arglist.asDouble(7, &stat),arglist.asDouble(8, &stat),arglist.asDouble(9, &stat)));
		
		cerr << "Computing boids' paths from frame " << startFrame << " to " << endFrame << "...\n";
		if (flock1.ignoreY) cerr << "Ignoring changes over y axis.\n";
		cerr << "Max velocity = " << flock1.maxVel << ", max acceleration = " << flock1.maxAcc << ", collision tolerance = " << flock1.avoidDist << "\n";
		cerr << "Target = (" << flock1.targets[0]->x << ", " << flock1.targets[0]->y << ", " << flock1.targets[0]->z << ")\n";		
	} else
	{
		cerr << "Invalid syntax. The correct syntax for this plugin is\n"; 
		cerr << "flock [startframe] [endframe] [ignore y axis? (1=ignore, 0=use)]\n";
		cerr << "[max velocity] [max accel] [collision distance] [boid radius] \n";
		cerr << "[target X] [target Y] [target Z]\n";
		cerr << "[obstacle1 X] [obstacle1 Y] [obstacle1 Z] [obstacle1 radius]\n"; 
		cerr << "[obstacle2 X] [obstacle2 Y] [obstacle2 Z] [obstacle2 radius] ...\n\n";
	}

	i = 10;	
	while ((i+3) <= arglist.length())
	{	
		newobs = new Point(arglist.asDouble(i, &stat), arglist.asDouble(i+1, &stat), arglist.asDouble(i+2,&stat), arglist.asDouble(i+3, &stat));
		flock1.obstacles.push_back(newobs);
		cerr << "Obstacle at (" << newobs->x << ", " << newobs->y << ", " << newobs->z << ") of size " << newobs->radius << ".\n";
		i+=4;
	}

	CHECK (stat);

	time.setValue(startFrame);
	MGlobal::getActiveSelectionList(list);
	if (list.length() == 0)
	{
		cerr << "Error: No boids selected.\n";
		abort = true;
	}
	for ( unsigned int index = 0; index < list.length(); index++ )
	{
		list.getDagPath( index, node, component );
		nodeFn.setObject( node );
		//cerr << "Loading boid '" << nodeFn.name().asChar() << "'...\n";

		depnode = node.transform(&stat);
		CHECK( stat );
		
		newboid = loadBoid(depnode, time);
		if (newboid == NULL)
		{
			return MS::kFailure;
		} else
		{
			newboid->radius = boidRadius;
			flock1.boids.push_back(newboid);
		}
	}

	// Now, all of the boids are loaded into memory.  We can start to calculate the boids' paths now.
	for (int frame = startFrame +1; frame <= endFrame; frame++)
	{
		if (abort) break;
		flock1.tick();
		cerr << "For frame " << frame << " boid is @ (" << flock1.boids[0]->pos(X) << ", " << flock1.boids[0]->pos(Y) << ", " << flock1.boids[0]->pos(Z) << "\n";
	}

	// Finally, we're all done computing the paths.  Let's write them out.
	MGlobal::getActiveSelectionList(list);
	for ( index = 0; index < list.length(); index++ )
	{
		list.getDagPath( index, node, component );
		nodeFn.setObject( node );
		//cerr << "Writing boid path for '" << nodeFn.name().asChar() << "'...\n";

		depnode = node.transform(&stat);
		CHECK( stat );

		stat = exportBoid(depnode, index);
		CHECK( stat );
	}
	MGlobal::setActiveSelectionList(list);

	return stat;
}

Boid *loadBoid(MObject depnode, MTime start)
{
	Boid *newBoid = new Boid();
	
	MStatus stat;

	MFnDependencyNode depfunc(depnode, &stat);
	CHECK( stat );
	
	MObject translateAttr[3];
	translateAttr[0] = depfunc.attribute( MString("translateX"), &stat );
	translateAttr[1] = depfunc.attribute( MString("translateY"), &stat );
	translateAttr[2] = depfunc.attribute( MString("translateZ"), &stat );

	CHECK(stat);

	MPlug txPlug( depnode, translateAttr[0] );
	MPlug tyPlug( depnode, translateAttr[1] );
	MPlug tzPlug( depnode, translateAttr[2] );

	MFnAnimCurve acFnX( txPlug, &stat);
	MFnAnimCurve acFnY( tyPlug, &stat);
	MFnAnimCurve acFnZ( tzPlug, &stat);

	if (stat.error())
	{
		cerr << "ERROR: You need to create initial keyframes on translateX,\n";
		cerr << "translateY, and translateZ for this plugin to work correctly.\n\n";
		cerr << "Do this by using the following procedure:\n";
		cerr << "  1. Keep your boids selected.\n";
		cerr << "  2. Press s to keyframe translateX, translateY, translateZ\n";
		cerr << "  3. Advance a few frames\n";
		cerr << "  4. Move the selected boids in the x, y, and z directions\n";
		cerr << "  5. Press s again\n";
		cerr << "  6. Rerun the plugin.\n\n";
		return NULL;
	}

	for (int i = 0; i < 4; i++)
	{
		if (i == 3)
		{
			newBoid->storeHistory = true;
		}

		newBoid->setPos(X, acFnX.evaluate(start, &stat));
		newBoid->setPos(Y, acFnY.evaluate(start, &stat));
		newBoid->setPos(Z, acFnZ.evaluate(start, &stat));
		
		CHECK(stat);

		newBoid->tick();
	}
	return newBoid;
}

MStatus exportBoid(MObject depnode, int index)
{
	MTime time;
	MStatus stat;

	MFnDependencyNode depfunc(depnode, &stat);
	CHECK( stat );
	
	MObject translateAttr[3];
	translateAttr[0] = depfunc.attribute( MString("translateX"), &stat );
	translateAttr[1] = depfunc.attribute( MString("translateY"), &stat );
	translateAttr[2] = depfunc.attribute( MString("translateZ"), &stat );
	CHECK(stat);

	MPlug txPlug( depnode, translateAttr[0] );
	MPlug tyPlug( depnode, translateAttr[1] );
	MPlug tzPlug( depnode, translateAttr[2] );

	MFnAnimCurve acFnX( txPlug, &stat);
	MFnAnimCurve acFnY( tyPlug, &stat);
	MFnAnimCurve acFnZ( tzPlug, &stat);

	if (stat.error())
	{
		cerr << "ERROR: You need to create initial keyframes on TranslateX, TranslateY, and TranslateZ for this\n";
		cerr << "plugin to work correctly. Do this by following this procedure:\n";
		cerr << "  1. Keep your boids selected.\n";
		cerr << "  2. Press s to keyframe translateX, translateY, translateZ\n";
		cerr << "  3. Advance a few frames\n";
		cerr << "  4. Move the selected boids in the x, y, and z directions\n";
		cerr << "  5. Press s again\n";
		cerr << "  6. Rerun the plugin.\n\n";
		return MStatus::kFailure;
	}	

	for (int t = startFrame; t <= endFrame; t++)
	{
		time.setValue(t);	
		acFnX.addKeyframe(time, flock1.boids[index]->history[t-startFrame]->x);
		acFnY.addKeyframe(time, flock1.boids[index]->history[t-startFrame]->y);
		acFnZ.addKeyframe(time, flock1.boids[index]->history[t-startFrame]->z);
	}	
	cerr << "\n";
	return stat;
}