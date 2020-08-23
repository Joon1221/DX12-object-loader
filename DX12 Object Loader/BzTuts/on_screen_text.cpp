#include "on_screen_text.h"
#include "d3dApp.h"
#include "game_object.h"

#include <regex>

extern D3DApp* gd3dApp;

OnScreenText::OnScreenText() : Renderable() {
	//Init();
}

OnScreenText::OnScreenText(string filePath) : Renderable() {
}

OnScreenText::~OnScreenText() {
}

bool OnScreenText::Init() {
	/* Source code removed for privacy purposes */
	return true;
}

bool OnScreenText::loadFile(string filePath) { return false;  }

void OnScreenText::Render() {
	
}
