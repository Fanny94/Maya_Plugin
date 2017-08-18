#include "Linker.h"
#include <vector>

MCallbackIdArray idList;

void AddTransform(MObject &transformNode, 
	MDagMessage::MatrixModifiedFlags &modified, void* clientData)
{
	MStatus ms;

	if (ms == MStatus::kSuccess) {

		MFnTransform transNode(transformNode, &ms);
		if (ms == MStatus::kSuccess) {
	
			MGlobal::displayInfo(transNode.name() + " Has been transformed");
		}
	}
}

void AttributePlugVertex(MNodeMessage::AttributeMessage msg, MPlug & plug, MPlug & otherPlug, void* clientData)
{

	if (msg & MNodeMessage::AttributeMessage::kAttributeSet && !plug.isArray() && plug.isElement()) {
		MStatus ms;
		MObject node = plug.node(&ms);

		if (ms == MStatus::kSuccess) {

			MFnMesh mesh(node, &ms);
			if (ms == MStatus::kSuccess) {
				MPoint p;
				ms = mesh.getPoint(plug.logicalIndex(), p, MSpace::kObject);

				if (ms == MStatus::kSuccess) {

					MGlobal::displayInfo("Name of mesh: " + mesh.name() + " Vertex changed: " + plug.name()
						+ " " + p.x + " " + p.y + " " + p.z);
				}
			}

		}
		
	}
}

void ChangeNameFunc(MObject &node, const MString &str, void * clientData)
{
	MGlobal::displayInfo("name changed");

	MFnMesh meshNode(node);
	MString m = ("name of mesh: " + meshNode.name());
	MGlobal::displayInfo(m);

}

void TimeElapsedFunction(float elapsedTime, float lastTime, void* clientData)
{

	MString s = "";
	s += elapsedTime;

	MGlobal::displayInfo("Elapsed Time: " + s);
}

void MNodeFunction(MObject &node, void* clientData)
{


	MStatus res = MS::kSuccess;
	if (node.hasFn(MFn::kMesh)){

		MCallbackId nameId = MNodeMessage::addNameChangedCallback(node, ChangeNameFunc, NULL, &res);
		
		if (res == MS::kSuccess) {
			idList.append(nameId);
			MGlobal::displayInfo("add name callback Succeeded");

			MFnMesh mesh(node);
			MGlobal::displayInfo("a node has been created" + mesh.name());
		}
		else {
			MGlobal::displayInfo("add name callback Failed");
		}

		MCallbackId attributeId = MNodeMessage::addAttributeChangedCallback(node, AttributePlugVertex, NULL, &res);

		if (res == MS::kSuccess) {
			idList.append(attributeId);
			MGlobal::displayInfo("attribute callback suceeeded");
		}
		else {
			MGlobal::displayInfo("attribute callback Failed");
		}
	}

	if (node.hasFn(MFn::kTransform)){

		MFnTransform meshTransform(node);
		MDagPath pathNode = MDagPath::getAPathTo(meshTransform.child(0));
		 
		MCallbackId transformId = MDagMessage::addWorldMatrixModifiedCallback(pathNode, AddTransform, NULL, &res);

		if (res == MS::kSuccess) {

			idList.append(transformId);
			MGlobal::displayInfo("transform callback Succeeded");
		}

		else {
			MGlobal::displayInfo("transform callback Failed");
		}

	}

}

EXPORT MStatus initializePlugin(MObject obj)
{
	//this indicates errors
	MStatus res = MS::kSuccess;

	MFnPlugin myPlugin(obj, "Maya Plugin", "1.0", "Any", &res);
	if (MFAIL(res)) {
		CHECK_MSTATUS(res);
	}

	MCallbackId nodeId = MDGMessage::addNodeAddedCallback(MNodeFunction, kDefaultNodeType, NULL, &res);
	

	if (res == MS::kSuccess) {
		idList.append(nodeId);
		MGlobal::displayInfo("added node callback Succeeded");
	}
	else {
		MGlobal::displayInfo("added node callback Failed");
	}

	MCallbackId timeId = MTimerMessage::addTimerCallback(5.0, TimeElapsedFunction, NULL, &res);

	if (res == MS::kSuccess) {
		idList.append(timeId);
		MGlobal::displayInfo("time elapsed callback Succeeded");
	}
	else {
		MGlobal::displayInfo("time elapsed callback Failed");
	}
	MGlobal::displayInfo("Maya plugin loaded!");

	return res;
}

EXPORT MStatus uninitializePlugin(MObject obj)
{
	MFnPlugin plugin(obj);

	MGlobal::displayInfo("Maya plugin unloaded!");

	MMessage::removeCallbacks(idList);

	return MS::kSuccess;
}
