#include "box_collider.h"
#include "mesh.h"
#include "avatar.h"
#include "animator.h"
#include "util.h"
#include "line.h"

BoxCollider::BoxCollider() : Collider() {
	showOutline = true;
}

BoxCollider::~BoxCollider() {
}

bool BoxCollider::Init() {
	char cStrBuf1[1024];

	//-------------------------------------------------------------
	// Get Outline
	//-------------------------------------------------------------
	// get outermost coordinates so box collider completely covers the object

	Vertex smallestXYZ;
	Vertex largestXYZ;

	// when host object is Animator
	if (hostObject->getComponent("Mesh") == NULL) {
		Animator *animator = (Animator *)hostObject->getComponent("Animator");
		smallestXYZ = animator->avatar->meshes[0]->finalVertices.at(0);
		largestXYZ = animator->avatar->meshes[0]->finalVertices.at(0);
		for (int i = 0; i < animator->avatar->numMeshes; i++) {
			for (Vertex& v : animator->avatar->meshes[i]->finalVertices) {
				if (v.pos.x < smallestXYZ.pos.x) {
					smallestXYZ.pos.x = v.pos.x;
				}
				if (v.pos.y < smallestXYZ.pos.y) {
					smallestXYZ.pos.y = v.pos.y;
				}
				if (v.pos.z < smallestXYZ.pos.z) {
					smallestXYZ.pos.z = v.pos.z;
				}

				if (v.pos.x > largestXYZ.pos.x) {
					largestXYZ.pos.x = v.pos.x;
				}
				if (v.pos.y > largestXYZ.pos.y) {
					largestXYZ.pos.y = v.pos.y;
				}

				if (v.pos.z > largestXYZ.pos.z) {
					largestXYZ.pos.z = v.pos.z;
				}
			}
		}
	}
	// when host object is Mesh
	else {
		smallestXYZ = ((Mesh *)hostObject->getComponent("Mesh"))->finalVertices.at(0);
		largestXYZ = ((Mesh *)hostObject->getComponent("Mesh"))->finalVertices.at(0);
		for (Vertex& v : ((Mesh *)hostObject->getComponent("Mesh"))->finalVertices) {
			if (v.pos.x < smallestXYZ.pos.x) {
				smallestXYZ.pos.x = v.pos.x;
			}
			if (v.pos.y < smallestXYZ.pos.y) {
				smallestXYZ.pos.y = v.pos.y;
			}
			if (v.pos.z < smallestXYZ.pos.z) {
				smallestXYZ.pos.z = v.pos.z;
			}

			if (v.pos.x > largestXYZ.pos.x) {
				largestXYZ.pos.x = v.pos.x;
			}
			if (v.pos.y > largestXYZ.pos.y) {
				largestXYZ.pos.y = v.pos.y;
			}

			if (v.pos.z > largestXYZ.pos.z) {
				largestXYZ.pos.z = v.pos.z;
			}
		}
	}


	//--------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------
	// example from https://www.geogebra.org/3d/yn3z8jar
	//--------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------
	// intialize corners of box

	corners[0] = largestXYZ; // point A
	corners[1] = Vertex(smallestXYZ.pos.x, largestXYZ.pos.y, largestXYZ.pos.z, 0, 0); // B
	corners[2] = Vertex(largestXYZ.pos.x, largestXYZ.pos.y, smallestXYZ.pos.z, 0, 0); // C
	corners[3] = Vertex(smallestXYZ.pos.x, largestXYZ.pos.y, smallestXYZ.pos.z, 0, 0); // D
	corners[4] = Vertex(largestXYZ.pos.x, smallestXYZ.pos.y, largestXYZ.pos.z, 0, 0); // E
	corners[5] = Vertex(smallestXYZ.pos.x, smallestXYZ.pos.y, largestXYZ.pos.z, 0, 0); // F
	corners[6] = Vertex(largestXYZ.pos.x, smallestXYZ.pos.y, smallestXYZ.pos.z, 0, 0); // G
	corners[7] = smallestXYZ; // H	

	// manually index the 12 edges of the box
	int headIndex[12] = {
		0, 2, 4, 6, 0, 1, 4, 5, 0, 1, 2, 3
	};

	int tailIndex[12] = {
		1, 3, 5, 7, 2, 3, 6, 7, 4, 5, 6, 7
	};

	for (int i = 0; i < 12; i++) {
		outline[i] = new Line();
		if (i < 4) {
			outline[i]->createXAxisLine(XMFLOAT3(corners[headIndex[i]].pos.x, corners[headIndex[i]].pos.y, corners[headIndex[i]].pos.z), XMFLOAT3(corners[tailIndex[i]].pos.x, corners[tailIndex[i]].pos.y, corners[tailIndex[i]].pos.z));
		}
		else if (i >= 4 && i < 8) {
			outline[i]->createZAxisLine(XMFLOAT3(corners[headIndex[i]].pos.x, corners[headIndex[i]].pos.y, corners[headIndex[i]].pos.z), XMFLOAT3(corners[tailIndex[i]].pos.x, corners[tailIndex[i]].pos.y, corners[tailIndex[i]].pos.z));
		}
		else {
			outline[i]->createYAxisLine(XMFLOAT3(corners[headIndex[i]].pos.x, corners[headIndex[i]].pos.y, corners[headIndex[i]].pos.z), XMFLOAT3(corners[tailIndex[i]].pos.x, corners[tailIndex[i]].pos.y, corners[tailIndex[i]].pos.z));
		}
		outline[i]->Init();
	}

	return true;
}

void BoxCollider::Render() {
	if (showOutline) {
		for (int i = 0; i < 12; i++) {
			outline[i]->Render();
		}
		//outline->Render();
	}
}
