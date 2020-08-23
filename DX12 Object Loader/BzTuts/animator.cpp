#include "animator.h"
#include "avatar.h"
#include "util.h"

Animator::Animator() : Renderable() {
}

Animator::Animator(string filePath) : Renderable() {
	avatar = new Avatar();
	loadMD5Mesh(filePath);
}

Animator::~Animator() {
}

bool Animator::Init() {
	for (int i = 0; i < avatar->meshes.size(); i++) {
		if (!avatar->meshes[i]->Init()) {
			return false;
		}
	}
	return true;
}

void Animator::Render() {
	avatar->Render();
}

bool Animator::loadMD5Mesh(string filePath) {
	return avatar->loadMD5Mesh(filePath);
}

bool Animator::loadMD5Anim(string filePath) {
	return avatar->loadMD5Anim(filePath);
}