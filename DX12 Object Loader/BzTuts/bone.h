#pragma once

#include "stdafx.h"
//#include "game_object.h"

class ObjModel;

// data loading은 avatar.cpp에서 함.
class Bone {
public:
	XMFLOAT3 pos;

	// ********NOTE********
	// rotation data is stored as a quaternion
	// when used in calculations, the first 3 values are used to find the value of w
	// XMFLOAT4 is converted to XMVECTOR when dealing calculations involving XMMATRIX
	// ********************
	XMFLOAT4 rot; 

	string name; 

	Bone *parentBone;
	vector<Bone *> childBones;

	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------
	// Member Functions
	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------
	Bone();
	Bone(string filePath);
	virtual ~Bone();

	virtual bool Init();

	//virtual bool loadFile(string filePath);

	virtual void Update();
	virtual void Render();
	virtual void UpdatePipeline();

	virtual void UpdateMatrices();

	virtual void Cleanup();

	//XMFLOAT3 getPos();

	//string returnHeirarchy();

};