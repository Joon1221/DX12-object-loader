#pragma once

#include "stdafx.h"
#include "component.h"
#include "mesh.h"

//using namespace std;

class Line : public Mesh {
public:
	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------
	// Member Variables
	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------

	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------
	// Member Functions
	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------
	Line();
	Line(XMFLOAT3 p1, XMFLOAT3 p2);
	virtual ~Line();

	void createLine(XMFLOAT3 p1, XMFLOAT3 p2);
	void createXAxisLine(XMFLOAT3 p1, XMFLOAT3 p2);
	void createYAxisLine(XMFLOAT3 p1, XMFLOAT3 p2);
	void createZAxisLine(XMFLOAT3 p1, XMFLOAT3 p2);
};
