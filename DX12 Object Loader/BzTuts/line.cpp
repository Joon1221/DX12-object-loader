#include "line.h"
#include "game_object.h"
#include "util.h"

//#define LINE_WIDTH 0.01f
#define LINE_WIDTH 0.1f

Line::Line() : Mesh() {
}

Line::Line(XMFLOAT3 p1, XMFLOAT3 p2) : Mesh() {
	createLine(p1, p2);
}

Line::~Line() {
}

/* When creating a line, we are actually creating a very thing rectangular prism.
   It is not possible to create a line with 0 width in DX12 meaning a box with a 
   very small width is the only other option of creating a "line" */
void Line::createLine(XMFLOAT3 p1, XMFLOAT3 p2) {
	imagePath = "..\\..\\imp_exp\\ash_uvgrid03.jpg";

	char cStrBuf1[2048];
	sprintf(cStrBuf1, "Line::createLine(): started");
	//msgBoxCstring(cStrBuf1);

	//----------------------------------------------------------------------------
	//----------------------------------------------------------------------------
	// read bone head
	//----------------------------------------------------------------------------
	//----------------------------------------------------------------------------
	float boneHeadCoords[3];
	boneHeadCoords[0] = p1.x;
	boneHeadCoords[1] = p1.y;
	boneHeadCoords[2] = p1.z;

	float boneTailCoords[3];
	boneTailCoords[0] = p2.x;
	boneTailCoords[1] = p2.y;
	boneTailCoords[2] = p2.z;


	//----------------------------------------------------------------------------
	//----------------------------------------------------------------------------
	// get distance between head and tail = curBoneTailAndHeadDist
	// (* not using curBoneTailAndHeadDistXZ)
	// get angle on xz plane              = curBoneDegreeOnXZPlaneRadian
	// get angle on xy plane              = curBoneDegreeOnXYPlaneRadian
	//----------------------------------------------------------------------------
	//----------------------------------------------------------------------------
	float curBoneDegreeOnXZPlaneRadian = atan((boneTailCoords[0] - boneHeadCoords[0]) / (boneTailCoords[2] - boneHeadCoords[2]));
	float curBoneTailAndHeadDist = sqrt((boneTailCoords[0] - boneHeadCoords[0])*(boneTailCoords[0] - boneHeadCoords[0]) + (boneTailCoords[1] - boneHeadCoords[1])*(boneTailCoords[1] - boneHeadCoords[1]) + (boneTailCoords[2] - boneHeadCoords[2])*(boneTailCoords[2] - boneHeadCoords[2]));
	float curBoneDegreeOnXYPlaneRadian = atan((boneTailCoords[1] - boneHeadCoords[1]) / (boneTailCoords[0] - boneHeadCoords[0]));

	//----------------------------------------------------------------------------
	//----------------------------------------------------------------------------
	// 각 점에서 보여질 (선과 직각인) 평면의 (점으로부터의) 4꼭지점의 거리 = 일종의 panning
	// Issue:
	//     아래에서 이점에 distance가 곱해지면서, 실제로 0.1거리가 아닌 0.331에 가까운 거리가 설정된다.
	//     왜냐하면 (0, 0, 0)과 (1, 3, -1)의 거리가 3.31이다.
	//----------------------------------------------------------------------------
	//----------------------------------------------------------------------------
	
	// intialize the x and y coordinates of the 8 vertices of the box  
	// *Note* that these vertices will be rotated using the same algorithm as the camera panning
	// algorithm so that they are in the correct orientation.
	int lineWidth = 0.1f;

	float xOnBonePanningPlaneForHead[4];
	xOnBonePanningPlaneForHead[0] = lineWidth;
	xOnBonePanningPlaneForHead[1] = lineWidth;
	xOnBonePanningPlaneForHead[2] = -lineWidth;
	xOnBonePanningPlaneForHead[3] = -lineWidth;

	float yOnBonePanningPlaneForHead[4];
	yOnBonePanningPlaneForHead[0] = lineWidth;
	yOnBonePanningPlaneForHead[1] = -lineWidth;
	yOnBonePanningPlaneForHead[2] = -lineWidth;
	yOnBonePanningPlaneForHead[3] = lineWidth;

	float xOnBonePanningPlaneForTail[4];
	xOnBonePanningPlaneForTail[0] = lineWidth;
	xOnBonePanningPlaneForTail[1] = lineWidth;
	xOnBonePanningPlaneForTail[2] = -lineWidth;
	xOnBonePanningPlaneForTail[3] = -lineWidth;

	float yOnBonePanningPlaneForTail[4];
	yOnBonePanningPlaneForTail[0] = lineWidth;
	yOnBonePanningPlaneForTail[1] = -lineWidth;
	yOnBonePanningPlaneForTail[2] = -lineWidth;
	yOnBonePanningPlaneForTail[3] = lineWidth;


	XMFLOAT3 averageHead(0.0f, 0.0f, 0.0f);
	XMFLOAT3 averageTail(0.0f, 0.0f, 0.0f);


	for (int i = 0; i < 8; i++) {

		// xy line calculation
		float curBoneTargetRadiusOfCircleOnXZPlane = (1.0f)*cos(curBoneDegreeOnXYPlaneRadian) - (0.0f + yOnBonePanningPlaneForHead[i % 4] / curBoneTailAndHeadDist)*sin(curBoneDegreeOnXYPlaneRadian);
		float curBoneTargetPostionY = (1.0f)*sin(curBoneDegreeOnXYPlaneRadian) + (0.0f + yOnBonePanningPlaneForHead[i % 4] / curBoneTailAndHeadDist)*cos(curBoneDegreeOnXYPlaneRadian);

		// xz line calculation
		float curBoneTargetPostionX = (curBoneTargetRadiusOfCircleOnXZPlane)*cos(curBoneDegreeOnXZPlaneRadian) - (0.0f + xOnBonePanningPlaneForHead[i % 4] / curBoneTailAndHeadDist)*sin(curBoneDegreeOnXZPlaneRadian);
		float curBoneTargetPostionZ = (curBoneTargetRadiusOfCircleOnXZPlane)*sin(curBoneDegreeOnXZPlaneRadian) + (0.0f + xOnBonePanningPlaneForHead[i % 4] / curBoneTailAndHeadDist)*cos(curBoneDegreeOnXZPlaneRadian);

		float curBonePostionX;
		float curBonePostionY;
		float curBonePostionZ;

		// first 4 vertices are near the head of the line
		if (i < 4) {
			curBonePostionX = curBoneTargetPostionX;
			curBonePostionY = curBoneTargetPostionY;
			curBonePostionZ = curBoneTargetPostionZ;
			averageHead.x += curBonePostionX;
			averageHead.y += curBonePostionY;
			averageHead.z += curBonePostionZ;
		}
		// last 4 vertices are near the tail of the line
		else {
			curBonePostionX = boneHeadCoords[0] + curBoneTargetPostionX;
			curBonePostionY = boneHeadCoords[1] + curBoneTargetPostionY;
			curBonePostionZ = boneHeadCoords[2] + curBoneTargetPostionZ;
			averageTail.x += curBonePostionX;
			averageTail.y += curBonePostionY;
			averageTail.z += curBonePostionZ;
		}

		// 계산된 점을 vertices에 add
		finalVertices.push_back(Vertex(curBonePostionX, curBonePostionY, curBonePostionZ, 0, 0));

	}

	// calculate the average x and y value of the head and tail 
	averageHead.x /= 4;
	averageHead.y /= 4;
	averageHead.z /= 4;

	averageTail.x /= 4;
	averageTail.y /= 4;
	averageTail.z /= 4;

	// calculate the final position of the 8 vertices using the average position and original position
	for (int i = 0; i < 8; i++) {
		if (i < 4) {
			finalVertices[i].pos.x -= averageHead.x - boneHeadCoords[0];
			finalVertices[i].pos.y -= averageHead.y - boneHeadCoords[1];
			finalVertices[i].pos.z -= averageHead.z - boneHeadCoords[2];

		}
		else {
			finalVertices[i].pos.x -= averageTail.x - boneTailCoords[0];
			finalVertices[i].pos.y -= averageTail.y - boneTailCoords[1];
			finalVertices[i].pos.z -= averageTail.z - boneTailCoords[2];
		}
	}

	//------------------------------------------------------------
	// Intialize Indices
	//------------------------------------------------------------
	// manually intialize the indices of the quad faces so that they face outward
	// *Note* that changing the order of the indices will flip the orientation
	// of the quadface/triface making it visible on the wrong side (it will be 
	// visible from the inside not the outside)

	int indices[4];
	indices[0] = 3;
	indices[1] = 2;
	indices[2] = 1;
	indices[3] = 0;
	quadFaces.push_back(QuadFace(indices));
	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 5;
	indices[3] = 4;
	quadFaces.push_back(QuadFace(indices));
	indices[0] = 4;
	indices[1] = 5;
	indices[2] = 6;
	indices[3] = 7;
	quadFaces.push_back(QuadFace(indices));
	indices[0] = 7;
	indices[1] = 6;
	indices[2] = 2;
	indices[3] = 3;
	quadFaces.push_back(QuadFace(indices));
	indices[0] = 4;
	indices[1] = 7;
	indices[2] = 3;
	indices[3] = 0;
	quadFaces.push_back(QuadFace(indices));
	indices[0] = 6;
	indices[1] = 5;
	indices[2] = 1;
	indices[3] = 2;
	quadFaces.push_back(QuadFace(indices));
}

// y and z values of both points need to be the same
void Line::createXAxisLine(XMFLOAT3 p1, XMFLOAT3 p2) {
	if (p1.x < p2.x) {
		XMFLOAT3 temp = p1;
		p1 = p2;
		p2 = temp;
	}
	imagePath = "..\\..\\imp_exp\\ash_uvgrid03.jpg";

	float lineWidth = LINE_WIDTH;


	//------------------------------------------------------------
	// Intialize Positions
	//------------------------------------------------------------
	// manually intialize position of final vertices
		
	// head
	finalVertices.push_back(Vertex(p1.x, p1.y + lineWidth, p1.z + lineWidth, 0, 0));
	finalVertices[finalVertices.size()-1].boneIndices[0] = 255;
	finalVertices.push_back(Vertex(p1.x, p1.y + lineWidth, p1.z - lineWidth, 0, 0));
	finalVertices[finalVertices.size()-1].boneIndices[0] = 255;
	finalVertices.push_back(Vertex(p1.x, p1.y - lineWidth, p1.z - lineWidth, 0, 0));
	finalVertices[finalVertices.size()-1].boneIndices[0] = 255;
	finalVertices.push_back(Vertex(p1.x, p1.y - lineWidth, p1.z + lineWidth, 0, 0));
	finalVertices[finalVertices.size()-1].boneIndices[0] = 255;

	// tail
	finalVertices.push_back(Vertex(p2.x, p2.y + lineWidth, p2.z + lineWidth, 0, 0));
	finalVertices[finalVertices.size()-1].boneIndices[0] = 255;
	finalVertices.push_back(Vertex(p2.x, p2.y + lineWidth, p2.z - lineWidth, 0, 0));
	finalVertices[finalVertices.size()-1].boneIndices[0] = 255;
	finalVertices.push_back(Vertex(p2.x, p2.y - lineWidth, p2.z - lineWidth, 0, 0));
	finalVertices[finalVertices.size()-1].boneIndices[0] = 255;
	finalVertices.push_back(Vertex(p2.x, p2.y - lineWidth, p2.z + lineWidth, 0, 0));
	finalVertices[finalVertices.size()-1].boneIndices[0] = 255;


	//------------------------------------------------------------
	// Intialize Indices
	//------------------------------------------------------------
	// manually intialize the indices of the quad faces so that they face outward
	// *Note* that changing the order of the indices will flip the orientation
	// of the quadface/triface making it visible on the wrong side (it will be 
	// visible from the inside not the outside)
	int indices[4];
	indices[0] = 3;
	indices[1] = 2;
	indices[2] = 1;
	indices[3] = 0;
	quadFaces.push_back(QuadFace(indices));
	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 5;
	indices[3] = 4;
	quadFaces.push_back(QuadFace(indices));
	indices[0] = 4;
	indices[1] = 5;
	indices[2] = 6;
	indices[3] = 7;
	quadFaces.push_back(QuadFace(indices));
	indices[0] = 7;
	indices[1] = 6;
	indices[2] = 2;
	indices[3] = 3;
	quadFaces.push_back(QuadFace(indices));
	indices[0] = 4;
	indices[1] = 7;
	indices[2] = 3;
	indices[3] = 0;
	quadFaces.push_back(QuadFace(indices));
	indices[0] = 6;
	indices[1] = 5;
	indices[2] = 1;
	indices[3] = 2;
	quadFaces.push_back(QuadFace(indices));
}

// x and z values of both points need to be the same
void Line::createYAxisLine(XMFLOAT3 p1, XMFLOAT3 p2) {
	if (p1.y < p2.y) {
		XMFLOAT3 temp = p1;
		p1 = p2;
		p2 = temp;
	}
	imagePath = "..\\..\\imp_exp\\ash_uvgrid03.jpg";

	float lineWidth = LINE_WIDTH;


	//------------------------------------------------------------
	// Intialize Positions
	//------------------------------------------------------------
	// manually intialize position of final vertices

	// head
	finalVertices.push_back(Vertex(p1.x + lineWidth, p1.y, p1.z + lineWidth, 0, 0));
	finalVertices[finalVertices.size()-1].boneIndices[0] = 255;
	finalVertices.push_back(Vertex(p1.x - lineWidth, p1.y, p1.z + lineWidth, 0, 0));
	finalVertices[finalVertices.size()-1].boneIndices[0] = 255;
	finalVertices.push_back(Vertex(p1.x - lineWidth, p1.y, p1.z - lineWidth, 0, 0));
	finalVertices[finalVertices.size()-1].boneIndices[0] = 255;
	finalVertices.push_back(Vertex(p1.x + lineWidth, p1.y, p1.z - lineWidth, 0, 0));
	finalVertices[finalVertices.size()-1].boneIndices[0] = 255;

	// tail
	finalVertices.push_back(Vertex(p2.x + lineWidth, p2.y, p2.z + lineWidth, 0, 0));
	finalVertices[finalVertices.size() - 1].boneIndices[0] = 255;
	finalVertices.push_back(Vertex(p2.x - lineWidth, p2.y, p2.z + lineWidth, 0, 0));
	finalVertices[finalVertices.size() - 1].boneIndices[0] = 255;
	finalVertices.push_back(Vertex(p2.x - lineWidth, p2.y, p2.z - lineWidth, 0, 0));
	finalVertices[finalVertices.size() - 1].boneIndices[0] = 255;
	finalVertices.push_back(Vertex(p2.x + lineWidth, p2.y, p2.z - lineWidth, 0, 0));
	finalVertices[finalVertices.size() - 1].boneIndices[0] = 255;


	//------------------------------------------------------------
	// Intialize Indices
	//------------------------------------------------------------
	// manually intialize the indices of the quad faces so that they face outward
	// *Note* that changing the order of the indices will flip the orientation
	// of the quadface/triface making it visible on the wrong side (it will be 
	// visible from the inside not the outside)
	int indices[4];
	indices[0] = 3;
	indices[1] = 2;
	indices[2] = 1;
	indices[3] = 0;
	quadFaces.push_back(QuadFace(indices));
	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 5;
	indices[3] = 4;
	quadFaces.push_back(QuadFace(indices));
	indices[0] = 4;
	indices[1] = 5;
	indices[2] = 6;
	indices[3] = 7;
	quadFaces.push_back(QuadFace(indices));
	indices[0] = 7;
	indices[1] = 6;
	indices[2] = 2;
	indices[3] = 3;
	quadFaces.push_back(QuadFace(indices));
	indices[0] = 4;
	indices[1] = 7;
	indices[2] = 3;
	indices[3] = 0;
	quadFaces.push_back(QuadFace(indices));
	indices[0] = 6;
	indices[1] = 5;
	indices[2] = 1;
	indices[3] = 2;
	quadFaces.push_back(QuadFace(indices));
}


// x and y values of both points need to be the same
void Line::createZAxisLine(XMFLOAT3 p1, XMFLOAT3 p2) {
	if (p1.z < p2.z) {
		XMFLOAT3 temp = p1;
		p1 = p2;
		p2 = temp;
	}

	imagePath = "..\\..\\imp_exp\\ash_uvgrid03.jpg";

	float lineWidth = LINE_WIDTH;

	finalVertices.push_back(Vertex(p1.x + lineWidth, p1.y + lineWidth, p1.z, 0, 0));
	finalVertices[finalVertices.size() - 1].boneIndices[0] = 255;
	finalVertices.push_back(Vertex(p1.x + lineWidth, p1.y - lineWidth, p1.z, 0, 0));
	finalVertices[finalVertices.size() - 1].boneIndices[0] = 255;
	finalVertices.push_back(Vertex(p1.x - lineWidth, p1.y - lineWidth, p1.z, 0, 0));
	finalVertices[finalVertices.size() - 1].boneIndices[0] = 255;
	finalVertices.push_back(Vertex(p1.x - lineWidth, p1.y + lineWidth, p1.z, 0, 0));
	finalVertices[finalVertices.size() - 1].boneIndices[0] = 255;

	finalVertices.push_back(Vertex(p2.x + lineWidth, p2.y + lineWidth, p2.z, 0, 0));
	finalVertices[finalVertices.size() - 1].boneIndices[0] = 255;
	finalVertices.push_back(Vertex(p2.x + lineWidth, p2.y - lineWidth, p2.z, 0, 0));
	finalVertices[finalVertices.size() - 1].boneIndices[0] = 255;
	finalVertices.push_back(Vertex(p2.x - lineWidth, p2.y - lineWidth, p2.z, 0, 0));
	finalVertices[finalVertices.size() - 1].boneIndices[0] = 255;
	finalVertices.push_back(Vertex(p2.x - lineWidth, p2.y + lineWidth, p2.z, 0, 0));
	finalVertices[finalVertices.size() - 1].boneIndices[0] = 255;

	int indices[4];
	indices[0] = 3;
	indices[1] = 2;
	indices[2] = 1;
	indices[3] = 0;
	quadFaces.push_back(QuadFace(indices));
	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 5;
	indices[3] = 4;
	quadFaces.push_back(QuadFace(indices));
	indices[0] = 4;
	indices[1] = 5;
	indices[2] = 6;
	indices[3] = 7;
	quadFaces.push_back(QuadFace(indices));
	indices[0] = 7;
	indices[1] = 6;
	indices[2] = 2;
	indices[3] = 3;
	quadFaces.push_back(QuadFace(indices));
	indices[0] = 4;
	indices[1] = 7;
	indices[2] = 3;
	indices[3] = 0;
	quadFaces.push_back(QuadFace(indices));
	indices[0] = 6;
	indices[1] = 5;
	indices[2] = 1;
	indices[3] = 2;
	quadFaces.push_back(QuadFace(indices));
}