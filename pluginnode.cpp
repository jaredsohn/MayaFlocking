//Flocking Node Plugin for Maya
//Note: This has not been completed.

//TODO (partial...also search for FIXME's):
//===============================================
// clean up remaining AnimCube stuff
// Don't let user keyframe startframe
// *** Get input from other boids via attributes ***
// Design better way of choosing boids


#include <maya/MFloatArray.h>
#include <maya/MTime.h>
#include <maya/MFnMesh.h>
#include <maya/MPoint.h>
#include <maya/MFloatPoint.h>
#include <maya/MFloatPointArray.h>
#include <maya/MIntArray.h>
#include <maya/MDoubleArray.h>
#include <maya/MFnUnitAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnPlugin.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MDagPath.h>
#include <maya/MPxNode.h>
#include <maya/MItDag.h>
#include <maya/MObject.h>
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MFnMeshData.h>

#include <iostream.h>

#include "flock.h"

static Flock flock1;

MStatus getPlugs(MDagPath &dagPath, MPlug *txPlug, MPlug *tyPlug, MPlug *tzPlug, MPlug *sxPlug, MPlug *syPlug, MPlug *szPlug);
MStatus loadInitSetup();
MStatus findDagPath(char *sought, MDagPath &dagPath);

MStatus returnStatus;

#define McheckErr(stat,msg)			\
	if ( MS::kSuccess != stat ) {	\
		cerr << msg;				\
		return MS::kFailure;		\
	}

class flock : public MPxNode
{
public:
					flock() {};
	virtual 		~flock() {};
	virtual MStatus compute(const MPlug& plug, MDataBlock& data);
	static  void*	creator();
	static  MStatus initialize();

	static MObject	time;
	static MObject	outputMesh;
	static MTypeId	id;

	static MObject boidPrefix;
	static MObject obstaclePrefix;
	static MObject maxVel;
	static MObject maxAcc;
	static MObject avoidDist;
	static MObject startFrame; //The frame where flocking begins

	//These are attribute arrays
	//FIXME: note: these will be outputs
	//FIXME: inputs should be the nodes at time START
	static MFloatArray boidsX;
	static MFloatArray boidsY;
	static MFloatArray boidsZ;
	static MFloatArray boidsRadius;

	/*
	static MObject obstaclesX;
	static MObject obstaclesY;
	static MObject obstaclesZ;
	static MObject obstaclesRadius;
	*/

	static int curFrame;

protected:
	MObject createMesh(const MTime& time, MObject& outData, MStatus& stat);
};

MObject flock::time;
MObject flock::outputMesh;
MTypeId flock::id( 0x80006 );
MObject flock::maxVel;
MObject flock::maxAcc;
MObject flock::avoidDist;
MObject flock::startFrame;

MFloatArray flock::boidsX;
MFloatArray flock::boidsY;
MFloatArray flock::boidsZ;
MFloatArray flock::boidsRadius;

int flock::curFrame;

void* flock::creator()
{
	return new flock;
}

MStatus flock::initialize()
{
	curFrame = -1;
	
	MFnUnitAttribute unitAttr;
	MFnTypedAttribute typedAttr;
	MFnNumericAttribute nAttr;

	MStatus returnStatus;

	flock::time = unitAttr.create( "time", "tm",
										  MFnUnitAttribute::kTime,
										  0.0, &returnStatus );
	McheckErr(returnStatus, "ERROR creating flock time attribute\n");


	flock::maxVel = nAttr.create( "maxVel", "vel",
												 MFnNumericData::kFloat, 3.0f,
												 &returnStatus ); 
	McheckErr(returnStatus, "ERROR creating flock maxVel attribute\n");

	flock::maxAcc = nAttr.create( "maxAcc", "acc",
												 MFnNumericData::kFloat, .0.6f,
												 &returnStatus ); 
	McheckErr(returnStatus, "ERROR creating flock maxAcc attribute\n");


	flock::avoidDist = nAttr.create( "avoidDist", "dis",
												 MFnNumericData::kFloat, 1.0f,
												 &returnStatus ); 
	McheckErr(returnStatus, "ERROR creating flock avoidDist attribute\n");


	flock::startFrame = nAttr.create( "startFrame", "sf",
												 MFnNumericData::kLong, 1.0f,
												 &returnStatus ); 
	McheckErr(returnStatus, "ERROR creating flock startFrame attribute\n");


	flock::outputMesh = typedAttr.create( "outputMesh", "out",
												 MFnData::kMesh,
												 &returnStatus ); 

	McheckErr(returnStatus, "ERROR creating flock output attribute\n");
	typedAttr.setStorable(false);

	returnStatus = addAttribute(flock::time);
	McheckErr(returnStatus, "ERROR adding time attribute\n");


	returnStatus = addAttribute(flock::maxVel);
	McheckErr(returnStatus, "ERROR adding maxVel attribute\n");

	returnStatus = addAttribute(flock::maxAcc);
	McheckErr(returnStatus, "ERROR adding maxAcc attribute\n");

	returnStatus = addAttribute(flock::avoidDist);
	McheckErr(returnStatus, "ERROR adding avoidDist attribute\n");

	returnStatus = addAttribute(flock::startFrame);
	McheckErr(returnStatus, "ERROR adding startFrame attribute\n");


	returnStatus = addAttribute(flock::outputMesh);
	McheckErr(returnStatus, "ERROR adding outputMesh attribute\n");

	returnStatus = attributeAffects(flock::time,
								    flock::outputMesh);

    McheckErr(returnStatus, "ERROR in attributeAffects\n");

	return MS::kSuccess;
}

MObject flock::createMesh(const MTime& time,
							  MObject& outData,
							  MStatus& stat)

{
	int				numVertices, frame;
	float			cubeSize;
	MFloatPointArray		points;
	MFnMesh			meshFS;

	// Scale the cube
	frame = (int)time.as( MTime::uiUnit() );
	if (frame == 0)
	  frame = 1;
	cubeSize		    		= 0.5f * (float)2.0f;

	const int numFaces			= 6;
	numVertices					= 8;
	const int numFaceConnects	= 24;

	MFloatPoint vtx_1( -cubeSize, -cubeSize, -cubeSize );
	MFloatPoint vtx_2(  cubeSize, -cubeSize, -cubeSize );
	MFloatPoint vtx_3(  cubeSize, -cubeSize,  cubeSize );
	MFloatPoint vtx_4( -cubeSize, -cubeSize,  cubeSize );
	MFloatPoint vtx_5( -cubeSize,  cubeSize, -cubeSize );
	MFloatPoint vtx_6( -cubeSize,  cubeSize,  cubeSize );
	MFloatPoint vtx_7(  cubeSize,  cubeSize,  cubeSize );
	MFloatPoint vtx_8(  cubeSize,  cubeSize, -cubeSize );
	points.append( vtx_1 );
	points.append( vtx_2 );
	points.append( vtx_3 );
	points.append( vtx_4 );
	points.append( vtx_5 );
	points.append( vtx_6 );
	points.append( vtx_7 );
	points.append( vtx_8 );

	// Set up an array containing the number of vertices
	// for each of the 6 cube faces (4 verticies per face)
	//
	int face_counts[numFaces] = { 4, 4, 4, 4, 4, 4 };
	MIntArray faceCounts( face_counts, numFaces );

	// Set up and array to assign vertices from points to each face 
	//
	int face_connects[ numFaceConnects ] = {	0, 1, 2, 3,
												4, 5, 6, 7,
												3, 2, 6, 5,
												0, 3, 5, 4,
												0, 4, 7, 1,
												1, 7, 6, 2	};
	MIntArray faceConnects( face_connects, numFaceConnects );

	MObject newMesh = meshFS.create(numVertices, numFaces,
									points, faceCounts, faceConnects,
									outData, &stat);
	return newMesh;
}

MStatus flock::compute(const MPlug& plug, MDataBlock& data)
{
	/* Get time */
	MDataHandle timeData = data.inputValue( time, &returnStatus ); 
	McheckErr(returnStatus, "Error getting time data handle\n");
	MTime time = timeData.asTime();
	int renderFr = (int)time.as( MTime::uiUnit() );

	/* Get start frame */
	MDataHandle _startFrame = data.inputValue( startFrame, &returnStatus ); 
	McheckErr(returnStatus, "Error getting start frame handle\n");
	int startFr = _startFrame.asInt();

	float radius;
	MPlug *txPlug = NULL, *tyPlug = NULL, *tzPlug = NULL, *sxPlug = NULL, *syPlug = NULL, *szPlug = NULL;
	float tx,ty,tz,sx,sy,sz;
	MDagPath dagPath;

	MStatus returnStatus;
	MObject depnode;

	if (plug == outputMesh) {
		/* Get output object */
		MDataHandle outputHandle = data.outputValue(outputMesh, &returnStatus);
		McheckErr(returnStatus, "ERROR getting polygon data handle\n");	    

		MFnMeshData dataCreator;
		MObject newOutputData = dataCreator.create(&returnStatus);
		McheckErr(returnStatus, "ERROR creating outputData");

		//FIXME: only draw if not rendering (look at MGlobal::MMayaState)
		MObject newMesh = createMesh(time, newOutputData, returnStatus);
		McheckErr(returnStatus, "ERROR creating new Cube");
		
		//cerr << "Looking for boids...\n";

		if (curFrame < startFr )
		{
			curFrame = startFr;
		}

		if ((curFrame < renderFr) || (renderFr != startFr))
		{
			// Calculate from between curFrame and time

			if (renderFr == startFr) // Load location/size from scene
			{ 
				loadInitSetup();
			}

			//now, run the algorithm for the remaining frames
			for (int i = startFr + 1; i <= renderFr; i++) {
				cerr << "Updating boids from frame " << startFr + 1 << " to " << renderFr << "...\n";

				// Clear old settings
				flock1.targets.clear();

				// Load current params
				//FIXME: base this on keyframes if we have any (or is it already?)

				/* Get params */
				MDataHandle temp = data.inputValue( maxVel, &returnStatus ); 
				McheckErr(returnStatus, "Error getting maxVel handle\n");
				flock1.maxVel = temp.asFloat();

				temp = data.inputValue( maxAcc, &returnStatus ); 
				McheckErr(returnStatus, "Error getting maxAcc handle\n");
				flock1.maxAcc = temp.asFloat();

				temp = data.inputValue( avoidDist, &returnStatus ); 
				McheckErr(returnStatus, "Error getting avoidDist handle\n");
				flock1.avoidDist = temp.asFloat();

				// Set the current target for the boids
				//FIXME -- For now, the control must be called flock
				char flockName[10] = "flock\0";				
				returnStatus = findDagPath(&flockName[0], dagPath);
				McheckErr(returnStatus, "ERROR find dag path");
				getPlugs(dagPath, txPlug, tyPlug, tzPlug, sxPlug, syPlug, szPlug);
				McheckErr(returnStatus, "ERROR getting plugs");							
				returnStatus = txPlug->getValue(tx);
				returnStatus = tyPlug->getValue(ty);
				returnStatus = tzPlug->getValue(tz);
				returnStatus = sxPlug->getValue(sx);
				returnStatus = syPlug->getValue(sy);
				returnStatus = szPlug->getValue(sz);
				//delete txPlug; delete tyPlug; delete tzPlug;
				//delete sxPlug; delete syPlug; delete szPlug;

				McheckErr(returnStatus, "ERROR getting plug values");
				
				if (sx > sy)
				{
					radius = (sx / 2.0f);
				} else
				{
					radius = (sy / 2.0f);
				}
				if ((radius * 2.0f) < sz)
				{
					radius = sz / 2.0f;
				}				
				flock1.targets.push_back(new Point(tx,ty,tz, radius));

				flock1.tick();

				curFrame++;
			}
		}

		//Set positions for the boids at the current frame
		for (int i = 0; i < flock1.boids.size(); i++)
		{
			findDagPath(flock1.boids[i]->name, dagPath);
			McheckErr(returnStatus, "ERROR finding DagPath");

			MPlug *txPlug = NULL, *tyPlug = NULL, *tzPlug = NULL, *sxPlug = NULL, *syPlug = NULL, *szPlug = NULL;
			getPlugs(dagPath, txPlug, tyPlug, tzPlug, sxPlug, syPlug, szPlug);
			McheckErr(returnStatus, "ERROR getting plugs");							
			
			returnStatus = txPlug->setValue(flock1.boids[i]->history[renderFr - startFr]->x);
			returnStatus = tyPlug->setValue(flock1.boids[i]->history[renderFr - startFr]->y);
			returnStatus = tzPlug->setValue(flock1.boids[i]->history[renderFr - startFr]->z);
			//delete txPlug; delete tyPlug; delete tzPlug;
			//delete sxPlug; delete syPlug; delete szPlug;
			McheckErr(returnStatus, "ERROR setting plug values");
		}

		outputHandle.set(newOutputData);
		data.setClean( plug );
	} else
		return MS::kUnknownParameter;

	return MS::kSuccess;
}

MStatus initializePlugin(MObject obj)
{
	MStatus   status;
	MFnPlugin plugin(obj, "Alias|Wavefront - Example", "3.0", "Any");

	status = plugin.registerNode("flock", flock::id,
						 flock::creator, flock::initialize);
	if (!status) {
		status.perror("registerNode");
		return status;
	}

	return status;
}

MStatus uninitializePlugin(MObject obj)
{
	MStatus	  status;
	MFnPlugin plugin(obj);
	Boid *temp = NULL;

	//Clean up boids
	flock1.boids.erase(flock1.boids.begin(),flock1.boids.end()); // Remove all elements. 

	status = plugin.deregisterNode(flock::id);
	if (!status) {
		status.perror("deregisterNode");
		return status;
	}

	return status;
}

MStatus getPlugs(MDagPath &dagPath, MPlug *txPlug, MPlug *tyPlug, MPlug *tzPlug, MPlug *sxPlug, MPlug *syPlug, MPlug *szPlug)
{
	MObject transformNode;

	transformNode = dagPath.transform(&returnStatus);
	McheckErr(returnStatus, "ERROR getting transform node");
	
	//cerr << "We have a transform node!\n";

	MFnDependencyNode depfunc(transformNode, &returnStatus);
	McheckErr(returnStatus, "ERROR getting dependency node");
	
	MObject translateAttr[3];
	MObject scaleAttr[3];
	translateAttr[0] = depfunc.attribute( MString("translateX"),&returnStatus);
	translateAttr[1] = depfunc.attribute( MString("translateY"), &returnStatus);
	translateAttr[2] = depfunc.attribute( MString("translateZ"), &returnStatus);
	scaleAttr[0] = depfunc.attribute( MString("scaleX"),&returnStatus);
	scaleAttr[1] = depfunc.attribute( MString("scaleY"), &returnStatus);
	scaleAttr[2] = depfunc.attribute( MString("scaleZ"), &returnStatus);
	McheckErr(returnStatus, "ERROR getting translate and scaling attributes");

	txPlug = new MPlug( transformNode, translateAttr[0] );
	tyPlug = new MPlug( transformNode, translateAttr[1] );
	tzPlug = new MPlug( transformNode, translateAttr[2] );
	sxPlug = new MPlug( transformNode, scaleAttr[0] );
	syPlug = new MPlug( transformNode, scaleAttr[1] );
	szPlug = new MPlug( transformNode, scaleAttr[2] );

	return MS::kSuccess;
}

MStatus loadInitSetup()
{
	cerr << "Reinitializing boids from Maya scene\n";
	MPlug *txPlug = NULL, *tyPlug = NULL, *tzPlug = NULL, *sxPlug = NULL, *syPlug = NULL, *szPlug = NULL;
	float radius;
	MDagPath dagPath;

	//Clean up boids
	flock1.boids.erase(flock1.boids.begin(),flock1.boids.end()); // Remove all elements. 
	
	MItDag dagIter( MItDag::kBreadthFirst, MFn::kInvalid, &returnStatus);		

	for ( ; !dagIter.isDone(); dagIter.next())
	{						
		//cerr << "found something...what is it?\n";
		returnStatus = dagIter.getPath( dagPath ); 
		McheckErr(returnStatus, "ERROR getting DAG path");
		
		MFnDagNode dagNode( dagPath, &returnStatus );
		McheckErr(returnStatus, "ERROR getting DAG node");

		//FIXME: Do a better job of figuring out what to include in selection
		//make sure that we include groups, too
		//
		// this object cannot be intermediate, and it must be a mesh
		// and it can't be a transform. 
		// Intermediate objects are special meshes
		// that are not drawn used for mesh morphs or something.
		//if ( dagNode.isIntermediateObject()) continue;
		//if ( !dagPath.hasFn( MFn::kMesh )) continue;
		//if ( dagPath.hasFn( MFn::kTransform )) continue;			

		if (strncmp(dagNode.name().asChar(), "boid", 4) == 0)
		{
			cerr << "I found a boid!..." << dagNode.name() << "\n";																

			MPlug *txPlug = NULL, *tyPlug = NULL, *tzPlug = NULL, *sxPlug = NULL, *syPlug = NULL, *szPlug = NULL;
			getPlugs(dagPath, txPlug, tyPlug, tzPlug, sxPlug, syPlug, szPlug);
			McheckErr(returnStatus, "ERROR getting plugs");							
			
			float tx,ty,tz,sx,sy,sz;

			returnStatus = txPlug->getValue(tx);
			returnStatus = tyPlug->getValue(ty);
			returnStatus = tzPlug->getValue(tz);
			returnStatus = sxPlug->getValue(sx);
			returnStatus = syPlug->getValue(sy);
			returnStatus = szPlug->getValue(sz);
			//delete txPlug; delete tyPlug; delete tzPlug;
			//delete sxPlug; delete syPlug; delete szPlug;

			McheckErr(returnStatus, "ERROR evaluating plugs");							

			if (sx > sy)
			{
				radius = (sx / 2.0f);
			} else
			{
				radius = (sy / 2.0f);
			}
			if ((radius * 2.0f) < sz)
			{
				radius = sz / 2.0f;
			}
		
			Boid *b = new Boid();
			b->name = strcpy(new char[strlen(dagNode.name().asChar()) + 1], dagNode.name().asChar());
			for (int i = 0; i < 4; i++)
			{
				if (i == 3) b->storeHistory = false;
				b->setPos(X, tx);
				b->setPos(Y, ty);
				b->setPos(Z, tz);					
				b->tick();
			}
			b->radius = radius;
			flock1.boids.push_back(b);

			cerr << "Created boid named " << b->name << " at (" << b->pos(X) << ", " << b->pos(Y) << ", " << b->pos(Z) << ") with radius " << b->radius << "...\n";
		} else if (strncmp(dagNode.name().asChar(), "obstacle", 8) == 0)
		{
			//cerr << "I found an obstacle!..." << dagNode.name() << "\n";																

			getPlugs(dagPath, txPlug, tyPlug, tzPlug, sxPlug, syPlug, szPlug);
			McheckErr(returnStatus, "ERROR getting plugs");							
			
			float tx,ty,tz,sx,sy,sz;

			returnStatus = txPlug->getValue(tx);
			returnStatus = tyPlug->getValue(ty);
			returnStatus = tzPlug->getValue(tz);
			returnStatus = sxPlug->getValue(sx);
			returnStatus = syPlug->getValue(sy);
			returnStatus = szPlug->getValue(sz);
			//delete txPlug; delete tyPlug; delete tzPlug;
			//delete sxPlug; delete syPlug; delete szPlug;
			McheckErr(returnStatus, "ERROR evaluating plugs");							

			if (sx > sy)
			{
				radius = (sx / 2.0f);
			} else
			{
				radius = (sy / 2.0f);
			}
			if ((radius * 2.0f) < sz)
			{
				radius = sz / 2.0f;
			}
			flock1.obstacles.push_back(new Point(tx, ty, tz, radius));
			cerr << "Created obstacle at (" << tx << ", " << ty << ", " << tz << ") with radius " << radius << "...\n";
		}
		dagIter.next();
	}

	return MS::kSuccess;
}

// Note: This is extremely inefficient.  Maybe someday I'll speed this up...
// Also note that most of this code is stolen from loadInitSetup()
MStatus findDagPath(char *sought, MDagPath &dagPath)
{
	MPlug *txPlug = NULL, *tyPlug = NULL, *tzPlug = NULL, *sxPlug = NULL, *syPlug = NULL, *szPlug = NULL;

	MItDag dagIter( MItDag::kBreadthFirst, MFn::kInvalid, &returnStatus);		

	for ( ; !dagIter.isDone(); dagIter.next())
	{						
		returnStatus = dagIter.getPath( dagPath ); 
		McheckErr(returnStatus, "ERROR getting DAG path");
		
		MFnDagNode dagNode( dagPath, &returnStatus );
		McheckErr(returnStatus, "ERROR getting DAG node");

		if (strcmp(dagNode.name().asChar(), sought) == 0)
		{
			//cerr << "I found what I seek!..." << dagNode.name() << "\n";
			return MS::kSuccess;
		}
		dagIter.next();
	}

	return MS::kFailure;
}