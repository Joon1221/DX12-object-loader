#include "bone.h"
#include "util.h"

Bone::Bone() {}
Bone::Bone(string filePath) {}
Bone::~Bone() {}

bool Bone::Init() { 
	//MessageBox(NULL, L"Hello1", L"", MB_OK | MB_ICONERROR);
	//ObjModel::Init();
	return true; 
}

#define BONE_HEAD_PLANE_SIZE 0.2f
#define BONE_HEAD_TAIL_RATIO 0.2f
#define BONE_TAIL_PLANE_SIZE (BONE_HEAD_PLANE_SIZE*BONE_HEAD_TAIL_RATIO)

void Bone::Update() {
	//ObjModel::Update();
}
void Bone::Render() {
	//ObjModel::Render();
}
void Bone::UpdatePipeline() {
	//ObjModel::UpdatePipeline();
}

void Bone::UpdateMatrices() {
	//ObjModel::UpdateMatrices();
}

void Bone::Cleanup() {
	//ObjModel::Cleanup();
}
