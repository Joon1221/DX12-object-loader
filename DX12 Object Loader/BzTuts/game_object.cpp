#include "stdafx.h"
#include <string>
#include <regex>

using namespace std;

#include "d3dApp.h"
#include "kj_app.h"

#include "game_object.h"

#include "util.h"

#include "transform.h"
#include "mesh_renderer.h"
#include "animator.h"
#include "mesh.h"
#include "renderable.h"
#include "box_collider.h"
#include "line.h"
#include "avatar.h"

extern D3DApp* gd3dApp;

//-------------------------------------------------------------------------
// On Screen Text
//-------------------------------------------------------------------------
Font LoadFont(LPCWSTR filename, int windowWidth, int windowHeight)
{
	/* Source code removed for privacy purposes */
}
//-------------------------------------------------------------------------

//================================================================================
//================================================================================
// Algorithm from: http://geomalgorithms.com/a06-_intersect-2.html
//     - checks if ray intersects with triangle in 3D space 
// Additional Sources: http://egloos.zum.com/expert3d/v/598593
//     - checks if intersection point is inside or outside of triangle
//================================================================================
//================================================================================
bool intersect3DTriangleWithRay(XMFLOAT3 rayPos, XMFLOAT3 rayDir, XMFLOAT3 P1, XMFLOAT3 P2, XMFLOAT3 P3, XMFLOAT3 *intersectionPoint) {
	//--------------------------------------------------------------------------------
	// calculate equation of plane containing the triangle (3 points)
	//--------------------------------------------------------------------------------
	XMFLOAT3 P1toP2(P2.x - P1.x, P2.y - P1.y, P2.z - P1.z);
	XMFLOAT3 P1toP3(P3.x - P1.x, P3.y - P1.y, P3.z - P1.z);

	//char tempBuf[256];
	//sprintf(tempBuf, "Equation of plane: | %d(x-[%4.1f]) + %d(y-[%4.1f]) + %d(z-[%4.1f]) = 0 | \r\n", a, P1.x, b, P1.y, c, P1.z);
	//outputConsole(tempBuf);

	// in the form ax + by + cz = d
	float a = P1toP2.y * P1toP3.z - P1toP2.z * P1toP3.y;
	float b = -1 * (P1toP2.x * P1toP3.z - P1toP2.z * P1toP3.x);
	float c = P1toP2.x * P1toP3.y - P1toP2.y * P1toP3.x;
	float d = a * P1.x + b * P1.y + c * P1.z;

	//char tempBuf[256];
	//sprintf(tempBuf, "Equation of plane:  | %dx + %dy + %dz = %d | \r\n", a, b, c, d);
	//outputConsole(tempBuf);
	//--------------------------------------------------------------------------------
	// calculate intersection point
	//--------------------------------------------------------------------------------

	//get distance between two points
	float dx = rayDir.x;
	float dy = rayDir.y;
	float dz = rayDir.z;

	//solve for t  
	float t = (d - (a * rayPos.x + b * rayPos.y + c * rayPos.z)) / (a * dx + b * dy + c * dz); //indicates how many "distances" of rayDir are travelled across the line

																							   // intersection point
	float rayX = rayPos.x + t * dx;
	float rayY = rayPos.y + t * dy;
	float rayZ = rayPos.z + t * dz;

	//char tempBuf[256];
	//sprintf(tempBuf, "Equation of ray:      | %4.2f + t(%4.2f) | \r\n", rayPos.x, dx);
	//outputConsole(tempBuf);
	//sprintf(tempBuf, "                               | %4.2f + t(%4.2f) | \r\n", rayPos.y, dy);
	//outputConsole(tempBuf);
	//sprintf(tempBuf, "                               | %4.2f + t(%4.2f) | \r\n", rayPos.z, dz);
	//outputConsole(tempBuf);
	//sprintf(tempBuf, "Intersection Point: (%4.2f , %4.2f , %4.2f) \r\n", rayX, rayY, rayZ);
	//outputConsole(tempBuf);

	//--------------------------------------------------------------------------------
	// check if intersection point is inside or outside of triangle
	//--------------------------------------------------------------------------------
	// Source: http://egloos.zum.com/expert3d/v/598593
	//    - copied "IsPointBounding(Vector &point, Vector *Poly, long VertexCount)"
	//--------------------------------------------------------------------------------

	float angle = 0.0f;

	const float MATCH_FACTOR = 0.99999f;
	XMFLOAT3 A, B;

	XMFLOAT3 triangle[3] = { P1, P2, P3 };

	for (int i = 0; i < 3; i++) {
		A.x = triangle[i].x - rayX;
		A.y = triangle[i].y - rayY;
		A.z = triangle[i].z - rayZ;
		B.x = triangle[(i + 1) % 3].x - rayX;
		B.y = triangle[(i + 1) % 3].y - rayY;
		B.z = triangle[(i + 1) % 3].z - rayZ;

		angle += acos((A.x * B.x + A.y * B.y + A.z * B.z) / (sqrt(A.x*A.x + A.y*A.y + A.z*A.z) * sqrt(B.x*B.x + B.y*B.y + B.z*B.z)));
	}

	//char tempBuf[256];
	//sprintf(tempBuf, "angle: %f\r\n", angle);
	//outputConsole(tempBuf);

	if (angle >= (MATCH_FACTOR * (2.0 * XM_PI))) {   	// 360도는 2PI
		intersectionPoint->x = rayX;
		intersectionPoint->y = rayY;
		intersectionPoint->z = rayZ;

		return true;
	}

	return false;
}

GameObject::GameObject() {
}

GameObject::GameObject(string objFilePathWithFilname) {
	loadFile(objFilePathWithFilname);
}

// load .md5mesh object or .obj object and give them the necessary components
bool GameObject::loadMesh(string filePath, bool boxColliderOn) {

	MeshRenderer *curMeshRenderer = new MeshRenderer();
	Renderable *curRenderable = new Renderable();
	
	// if object is a .md5mesh file
	if (filePath.find(".md5mesh", 0) != std::string::npos) {
		curRenderable = new Animator(filePath);
		components["Animator"] = curRenderable;
	}
	// if object is a .obj file
	else if (filePath.find(".obj", 0) != std::string::npos) {
		curRenderable = new Mesh(filePath);
		components["Mesh"] = curRenderable;
	}

	curMeshRenderer->renderable = curRenderable;
	components["Transform"] = new Transform();
	components["Mesh Renderer"] = (curMeshRenderer);

	if (boxColliderOn) {
		BoxCollider *curBoxCollider = new BoxCollider();
		curBoxCollider->hostObject = this;
		components["Box Collider"] = (curBoxCollider);
	}

	return true;
}

bool GameObject::loadFile(string filePath) {
	return false;
}

GameObject::~GameObject() {
}

bool GameObject::Init() {
	for (map<string, Component *>::iterator it = components.begin(); it != components.end(); ++it) {
		it->second->Init();
	}

	//char cStrBuf1[2048];
	//sprintf(cStrBuf1, "GameObject::Init(): end");
	//msgBoxCstring(cStrBuf1);

	return true;
}

void GameObject::Update() {
}

void GameObject::Render() {
	if (getComponent("Mesh Renderer") != NULL) {
		((MeshRenderer *)getComponent("Mesh Renderer"))->Render();
	}

	if (getComponent("Box Collider") != NULL) {
		((BoxCollider *)getComponent("Box Collider"))->Render();
	}
}

void GameObject::UpdatePipeline() {
	Render();
}

void GameObject::Cleanup() {

}

Component *GameObject::getComponent(string componentName) {
	if (components.find(componentName) == components.end()) {
		return NULL;
	}
	return components[componentName];

}