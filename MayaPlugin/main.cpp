#include "Linker.h"
#include <vector>

MCallbackIdArray idList;

//called for each time a node is transformed
void hasTransformed(MObject &transformNode, 
	MDagMessage::MatrixModifiedFlags &modified, void* clientData)
{
	MStatus ms;

	if (ms == MStatus::kSuccess) {

		MFnTransform transNode(transformNode, &ms);

		if (ms == MStatus::kSuccess) {
			
			//print the transformed node
			MGlobal::displayInfo(transNode.name() + " HAS BEEN TRANSFORMED");
		}
	}
}

//whenever a vertex has been changed
void attributePlugVertex(MNodeMessage::AttributeMessage msg, MPlug & plug, MPlug & otherPlug, void* clientData)
{
	//Excluding things in the plugs that are not vertices
	if (msg & MNodeMessage::AttributeMessage::kAttributeSet && !plug.isArray() && plug.isElement()) {
		MStatus ms;

		//get the node the plug is connected to
		MObject node = plug.node(&ms);

		if (ms == MStatus::kSuccess) {

			MFnMesh mesh(node, &ms);

			if (ms == MStatus::kSuccess) {
				MPoint p;

				//get the vertices of the mesh and p. p = vertex coordinates (x, y, z)
				ms = mesh.getPoint(plug.logicalIndex(), p, MSpace::kObject);

				if (ms == MStatus::kSuccess) {

					//print the mesh's vertex point that changed and its new coordinates 
					MGlobal::displayInfo("VERTEX CHANGED: " + plug.name()
						+ " " + p.x + " " + p.y + " " + p.z);
				}
			}

		}
		
	}
}

//prints the name of a node whenever the name changes
void changeNameFunc(MObject &node, const MString &str, void * clientData)
{
	MStatus ms;
	if (ms == MStatus::kSuccess) {

		MFnMesh mesh(node, &ms);

		if (ms == MStatus::kSuccess) {
			MGlobal::displayInfo("NAME CHANGED: " + mesh.name());
		}
	}

}

//prints the time elapsed every 5 seconds
void timeElapsedFunction(float elapsedTime, float lastTime, void* clientData)
{

	MString s = "";
	s += elapsedTime;

	MGlobal::displayInfo("ELAPSED TIME: " + s);
}

//Whenever a node has been created 
void mNodeFunction(MObject &node, void* clientData)
{

	MStatus res = MS::kSuccess;

	//check if the node holds a mesh
	if (node.hasFn(MFn::kMesh)){

		//call when the name of the node changes
		MCallbackId nameId = MNodeMessage::addNameChangedCallback(node, changeNameFunc, NULL, &res);
		
		if (res == MS::kSuccess) {
			//append callback ID to a list
			idList.append(nameId);

			MFnMesh mesh(node);

			//prints the name of the mesh whenever a node has been created
			MGlobal::displayInfo("NODE CREATED:" + mesh.name());
		}
		else {
			MGlobal::displayInfo("add name callback Failed");
		}


		MCallbackId attributeId = MNodeMessage::addAttributeChangedCallback(node, attributePlugVertex, NULL, &res);

		if (res == MS::kSuccess) {
			idList.append(attributeId);
		}
		else {
			MGlobal::displayInfo("attribute callback Failed");
		}
	}

	//Check if the node holds a transform
	if (node.hasFn(MFn::kTransform)){

		MFnTransform meshTransform(node);

		//get the the node's transform, allways child of the mesh in the hierarchy
		MDagPath pathNode = MDagPath::getAPathTo(meshTransform.child(0));
		
		//check if worldtransformation matrix is changed in Maya
		MCallbackId transformId = MDagMessage::addWorldMatrixModifiedCallback(pathNode, hasTransformed, NULL, &res);

		if (res == MS::kSuccess) {

			//append callback ID to a list
			idList.append(transformId);
		}

		else {
			MGlobal::displayInfo("transform callback Failed");
		}

	}

}

//called when plugin is loaded
EXPORT MStatus initializePlugin(MObject obj)
{
	//This is for checking errors
	MStatus res = MS::kSuccess;

	MFnPlugin myPlugin(obj, "Maya Plugin", "1.0", "Any", &res);
	if (MFAIL(res)) {
		CHECK_MSTATUS(res);
	}

	MCallbackId nodeId = MDGMessage::addNodeAddedCallback(mNodeFunction, kDefaultNodeType, NULL, &res);
	
	//callback Succeeded
	if (res == MS::kSuccess) {
		
		//append callback ID to a list
		idList.append(nodeId);	
	}
	else {
		MGlobal::displayInfo("node callback Failed");
	}

	MCallbackId timeId = MTimerMessage::addTimerCallback(5.0, timeElapsedFunction, NULL, &res);

	if (res == MS::kSuccess) {
		//append callback ID to a list
		idList.append(timeId);

	}
	else {
		MGlobal::displayInfo("time elapsed callback Failed");
	}

	MGlobal::displayInfo("MAYA PLUGIN LOADED!");

	return res;
}

//called when plugin is unloaded
EXPORT MStatus uninitializePlugin(MObject obj)
{
	MFnPlugin plugin(obj);

	MGlobal::displayInfo("MAYA PLUGIN UNLOADED!");

	MMessage::removeCallbacks(idList);

	return MS::kSuccess;
}
