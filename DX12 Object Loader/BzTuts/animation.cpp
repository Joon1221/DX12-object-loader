#include "animation.h"
#include "util.h"

Animation::Animation() {
	curFrame = 0;
	numTicksToSkipPerFrame = NUM_TICKS_TO_SKIP_PER_FRAME;
}

Animation::Animation(string filePath) {
	curFrame = 0;
	numTicksToSkipPerFrame = NUM_TICKS_TO_SKIP_PER_FRAME;
	loadMD5Anim(filePath);
}

Animation::~Animation() {
}

// update the position and rotation data of the bones to those of the next frame
void Animation::Update() {
	for (int i = 0; i < numJoints; ++i) {
		avatar->bones[i]->pos = baseFrame.pos[i];
		avatar->bones[i]->rot = baseFrame.rot[i];

		//========================================================================================================================
		//========================================================================================================================
		// In the algorithm from https://www.3dgep.com/loading-and-animating-md5-models-with-opengl/#The_md5anim_File, all 
		// of the frame values are stored into one giant container. And so a start index and a j value is needed to access
		// the information. The frameStartIndex value is given in the bone hierarchy (0 for the 1st bone, 6 for the 2nd, 
		// and etc going up by multiples of 6), and everytime a value is read, the j value is increased in order to access
		// the next value. This however is not needed as our algorithm reads each frame and stores the data into a pos and 
		// rot vector. As such, we can simply access the position and rotation values of each bone at each frame without
		// having to use the frameStartIndex and the j value.
		//========================================================================================================================
		//========================================================================================================================

		if (flags[i] & 1) 
		{
			avatar->bones[i]->pos.x = frames[curFrame].pos[i].x;
		}
		if (flags[i] & 2)
		{
			avatar->bones[i]->pos.y = frames[curFrame].pos[i].y;
		}
		if (flags[i] & 4)
		{
			avatar->bones[i]->pos.z = frames[curFrame].pos[i].z;
		}
		if (flags[i] & 8)
		{
			avatar->bones[i]->rot.x = frames[curFrame].rot[i].x;
		}
		if (flags[i] & 16)
		{
			avatar->bones[i]->rot.y = frames[curFrame].rot[i].y;
		}
		if (flags[i] & 32)
		{
			avatar->bones[i]->rot.z = frames[curFrame].rot[i].z;
		}
		ComputeQuatW(avatar->bones[i]->rot);

		if (avatar->bones[i]->parentBone != NULL) // Has a parent joint
		{
			XMFLOAT3 rotPos = rotateVectorByQuat(avatar->bones[i]->parentBone->rot, avatar->bones[i]->pos);

			avatar->bones[i]->pos.x = avatar->bones[i]->parentBone->pos.x + rotPos.x;
			avatar->bones[i]->pos.y = avatar->bones[i]->parentBone->pos.y + rotPos.y;
			avatar->bones[i]->pos.z = avatar->bones[i]->parentBone->pos.z + rotPos.z;


			XMStoreFloat4(&avatar->bones[i]->rot, XMVector4Normalize(XMLoadFloat4(&quatMult(avatar->bones[i]->parentBone->rot, avatar->bones[i]->rot))));
		}
	}

	numTicksToSkipPerFrame--;
	if (numTicksToSkipPerFrame <= 0) {
		curFrame++;
		if (curFrame >= numFrames) {
			curFrame = 0;
		}
		numTicksToSkipPerFrame = NUM_TICKS_TO_SKIP_PER_FRAME;
	}
}


// read in md5 animation data
bool Animation::loadMD5Anim(string filePath) {
	
	char cStrBuf1[2048];

	//sprintf(cStrBuf1, "Animation::loadMD5Anim: objFilePathWithFilname = %s", filePath.c_str());
	//msgBoxCstring(cStrBuf1);

	ifstream fin;

	fin.open(filePath.c_str());
	if (fin.fail()) {
		MessageBox(NULL, L"fin", L"Error", MB_OK | MB_ICONERROR);
		exit(1);
	}

	char buf[1024];

	string curToken;
	int curMeshIndex = -1;
	while (!fin.eof()) {
		fin >> curToken;
		//sprintf(cStrBuf1, "Animation::loadMD5Anim: curToken = %s", curToken.c_str());
		//msgBoxCstring(cStrBuf1);
		if (curToken == "MD5Version") {
			fin >> curToken;
			//sprintf(cStrBuf1, "Animation::loadMD5Anim(): md5Version = %s", curToken.c_str());
			//msgBoxCstring(cStrBuf1);
		}
		else if (curToken == "commandline") {
			fin.getline(buf, 1024);
			//sprintf(cStrBuf1, "Animation::loadMD5Anim(): buf = %s", buf);
			//msgBoxCstring(cStrBuf1);
		}
		else if (curToken == "numFrames") {
			fin >> numFrames;
			//sprintf(cStrBuf1, "Animation::loadMD5Anim(): numFrames = %d", numFrames);
			//msgBoxCstring(cStrBuf1);
		}
		else if (curToken == "numJoints") {
			fin >> numJoints;
			//sprintf(cStrBuf1, "Animation::loadMD5Anim(): numJoints = %d", numJoints);
			//msgBoxCstring(cStrBuf1);
		}
		else if (curToken == "frameRate") {
			fin >> frameRate;
			//sprintf(cStrBuf1, "Animation::loadMD5Anim(): frameRate = %d", frameRate);
			//msgBoxCstring(cStrBuf1);
		}
		else if (curToken == "numAnimatedComponents") {
			fin >> numAnimatedComponents;
			//sprintf(cStrBuf1, "Animation::loadMD5Anim(): numAnimatedComponents = %d", numAnimatedComponents);
			//msgBoxCstring(cStrBuf1);
		}
		else if (curToken == "hierarchy") {
			fin >> curToken; // read {
			for (int i = 0; i < numJoints; i++) {
				fin >> curToken;
				while (curToken.find("\"", 1) == string::npos) {
					string temp;
					fin >> temp;
					curToken += " " + temp;
				}
				string name = curToken.substr(1, curToken.size() - 2);

				int parentIndex;
				fin >> parentIndex;

				int flag;
				fin >> flag;
				flags.push_back(flag);
				
				int startIndex;
				fin >> startIndex;

				fin.getline(buf, 1024); // read any extra comments 

				//sprintf(cStrBuf1, "Animation::loadMD5Anim(): name = %s", name);
				//msgBoxCstring(cStrBuf1);			
				//sprintf(cStrBuf1, "Animation::loadMD5Anim(): flag = %d", flags[i]);
				//msgBoxCstring(cStrBuf1);		
				//sprintf(cStrBuf1, "Animation::loadMD5Anim(): startIndex = %d", startIndex);
				//msgBoxCstring(cStrBuf1);		
				//sprintf(cStrBuf1, "Animation::loadMD5Anim(): flags = %d", flags);
				//msgBoxCstring(cStrBuf1);		
			}
			fin >> curToken; // read }
		}
		else if (curToken == "bounds") {
			fin >> curToken; // read {
			for (int i = 0; i < numFrames; i++) {

				fin >> curToken; // read (

				XMFLOAT3 boundMin;

				fin >> boundMin.x;
				fin >> boundMin.y;
				fin >> boundMin.z;

				fin >> curToken; // read )
				fin >> curToken; // read (

				XMFLOAT3 boundMax;

				fin >> boundMax.x;
				fin >> boundMax.y;
				fin >> boundMax.z;

				fin >> curToken; // read )
				fin.getline(buf, 1024); // read any extra comments 

				//sprintf(cStrBuf1, "Animation::loadMD5Anim(): boundMin.x = %f", boundMin.x);
				//msgBoxCstring(cStrBuf1);
				//sprintf(cStrBuf1, "Animation::loadMD5Anim(): boundMin.y = %f", boundMin.y);
				//msgBoxCstring(cStrBuf1);
				//sprintf(cStrBuf1, "Animation::loadMD5Anim(): boundMin.z = %f", boundMin.z);
				//msgBoxCstring(cStrBuf1);
				//sprintf(cStrBuf1, "Animation::loadMD5Anim(): boundMax.x = %f", boundMax.x);
				//msgBoxCstring(cStrBuf1);
				//sprintf(cStrBuf1, "Animation::loadMD5Anim(): boundMax.y = %f", boundMax.y);
				//msgBoxCstring(cStrBuf1);
				//sprintf(cStrBuf1, "Animation::loadMD5Anim(): boundMax.z = %f", boundMax.z);
				//msgBoxCstring(cStrBuf1);
			}
		}
		else if (curToken == "baseframe") {
			fin >> curToken; // read {
			baseFrame = Frame();
			for (int i = 0; i < numJoints; i++) {

				fin >> curToken; // read (

				XMFLOAT3 pos;
				fin >> pos.x;
				fin >> pos.y;
				fin >> pos.z;
				baseFrame.pos.push_back(pos);

				fin >> curToken; // read )
				fin >> curToken; // read (

				XMFLOAT4 rot;
				fin >> rot.x;
				fin >> rot.y;
				fin >> rot.z;

				ComputeQuatW(rot);

				baseFrame.rot.push_back(rot);

				fin >> curToken; // read )
				fin.getline(buf, 1024); // read any extra comments 

				//sprintf(cStrBuf1, "Animation::loadMD5Anim(): baseFrame.pos[i].x = %f", baseFrame.pos[i].x);
				//msgBoxCstring(cStrBuf1);
				//sprintf(cStrBuf1, "Animation::loadMD5Anim(): baseFrame.pos[i].y = %f", baseFrame.pos[i].y);
				//msgBoxCstring(cStrBuf1);
				//sprintf(cStrBuf1, "Animation::loadMD5Anim(): baseFrame.pos[i].z = %f", baseFrame.pos[i].z);
				//msgBoxCstring(cStrBuf1);
				//sprintf(cStrBuf1, "Animation::loadMD5Anim(): baseFrame.rot[i].x = %f", baseFrame.rot[i].x);
				//msgBoxCstring(cStrBuf1);
				//sprintf(cStrBuf1, "Animation::loadMD5Anim(): baseFrame.rot[i].y = %f", baseFrame.rot[i].y);
				//msgBoxCstring(cStrBuf1);
				//sprintf(cStrBuf1, "Animation::loadMD5Anim(): baseFrame.rot[i].z = %f", baseFrame.rot[i].z);
				//msgBoxCstring(cStrBuf1);
			}
		}
		else if (curToken == "frame") {
			fin >> curToken; // curFrame
			fin >> curToken; // read {
			Frame curFrame = Frame();
			for (int i = 0; i < numJoints; i++) {

				XMFLOAT3 pos;
				fin >> pos.x;
				fin >> pos.y;
				fin >> pos.z;
				curFrame.pos.push_back(pos);

				XMFLOAT4 rot;

				fin >> rot.x;
				fin >> rot.y;
				fin >> rot.z;

				ComputeQuatW(rot);

				curFrame.rot.push_back(rot);

				fin.getline(buf, 1024); // read any extra comments 

				//sprintf(cStrBuf1, "Animation::loadMD5Anim(): pos.x = %f", pos.x);
				//msgBoxCstring(cStrBuf1);
				//sprintf(cStrBuf1, "Animation::loadMD5Anim(): pos.y = %f", pos.y);
				//msgBoxCstring(cStrBuf1);
				//sprintf(cStrBuf1, "Animation::loadMD5Anim(): pos.z = %f", pos.z);
				//msgBoxCstring(cStrBuf1);
				//sprintf(cStrBuf1, "Animation::loadMD5Anim(): rot.x = %f", rot.x);
				//msgBoxCstring(cStrBuf1);
				//sprintf(cStrBuf1, "Animation::loadMD5Anim(): rot.y = %f", rot.y);
				//msgBoxCstring(cStrBuf1);
				//sprintf(cStrBuf1, "Animation::loadMD5Anim(): rot.z = %f", rot.z);
				//msgBoxCstring(cStrBuf1);
			}
			frames.push_back(curFrame);
		}
	}

	return true;
}

