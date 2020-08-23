#pragma once

#include "stdafx.h"
#include "avatar.h"

//using namespace std;

class Avatar;


// holds information of positions and rotations in a given frame
struct Frame {
	Frame() {

	}

	vector<XMFLOAT3> pos;
	vector<XMFLOAT4> rot;
};

#define NUM_TICKS_TO_SKIP_PER_FRAME 50

class Animation {
public:
	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------
	// Member Variables
	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------

	Avatar *avatar; // pointer of host

	string command;
	int numFrames;
	int curFrame;
	int numJoints;
	int frameRate;
	int numAnimatedComponents;

	Frame baseFrame;
	vector<Frame> frames;
	vector<int> flags;

	// frame을 skip시키기 위한 임시 변수
	int numTicksToSkipPerFrame;

	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------
	// Member Functions
	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------
	Animation();
	Animation(string filePath);
	virtual ~Animation();

	void Update();

	bool loadMD5Anim(string filePath);
};
