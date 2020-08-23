#pragma once

#include "stdafx.h"
#include <string>
#include <map>

//using namespace std;

//-------------------------------------------------------------------------
// On Screen Text
//-------------------------------------------------------------------------
struct FontKerning
{
	int firstid; // the first character
	int secondid; // the second character
	float amount; // the amount to add/subtract to second characters x
};
struct FontChar
{
	// the unicode id
	int id;

	// these need to be converted to texture coordinates 
	// (where 0.0 is 0 and 1.0 is textureWidth of the font)
	float u; // u texture coordinate
	float v; // v texture coordinate
	float twidth; // width of character on texture
	float theight; // height of character on texture

	float width; // width of character in screen coords
	float height; // height of character in screen coords

	// these need to be normalized based on size of font
	float xoffset; // offset from current cursor pos to left side of character
	float yoffset; // offset from top of line to top of character
	float xadvance; // how far to move to right for next character
};
struct Font
{
	std::wstring name; // name of the font
	std::wstring fontImage;
	int size; // size of font, lineheight and baseheight will be based on this as if this is a single unit (1.0)
	float lineHeight; // how far to move down to next line, will be normalized
	float baseHeight; // height of all characters, will be normalized
	int textureWidth; // width of the font texture
	int textureHeight; // height of the font texture
	int numCharacters; // number of characters in the font
	FontChar* CharList; // list of characters
	int numKernings; // the number of kernings
	FontKerning* KerningsList; // list to hold kerning values
	ID3D12Resource* textureBuffer; // the font texture resource
	D3D12_GPU_DESCRIPTOR_HANDLE srvHandle; // the font srv

	// these are how much the character is padded in the texture. We
	// add padding to give sampling a little space so it does not accidentally
	// padd the surrounding characters. We will need to subtract these paddings
	// from the actual spacing between characters to remove the gaps you would otherwise see
	float leftpadding;
	float toppadding;
	float rightpadding;
	float bottompadding;

	// this will return the amount of kerning we need to use for two characters
	float GetKerning(wchar_t first, wchar_t second)
	{
		for (int i = 0; i < numKernings; ++i)
		{
			if ((wchar_t)KerningsList[i].firstid == first && (wchar_t)KerningsList[i].secondid == second)
				return KerningsList[i].amount;
		}
		return 0.0f;
	}

	// this will return a FontChar given a wide character
	FontChar* GetChar(wchar_t c)
	{
		for (int i = 0; i < numCharacters; ++i)
		{
			if (c == (wchar_t)CharList[i].id)
				return &CharList[i];
		}
		return nullptr;
	}
};
struct TextVertex {
	TextVertex(float r, float g, float b, float a, float u, float v, float tw, float th, float x, float y, float w, float h) : color(r, g, b, a), texCoord(u, v, tw, th), pos(x, y, w, h) {}
	XMFLOAT4 pos;
	XMFLOAT4 texCoord;
	XMFLOAT4 color;
};
//-------------------------------------------------------------------------

struct Weight {
	Weight(int boneIndex, float weight, float x, float y, float z) : boneIndex(boneIndex), weight(weight), pos(x, y, z) {}
	Weight() {
		boneIndex = -1;
		weight = 0.0f;
	}

	int boneIndex;
	float weight;

	XMFLOAT3 pos;
};
struct Vertex {
	Vertex(float x, float y, float z, float u, float v, int weightIndex, int numWeights) : pos(x, y, z), texCoord(u, v) {
		this->weightIndex = weightIndex;
		this->numWeights = numWeights;

		numWeights = 0;
	}
	Vertex(float x, float y, float z, float u, float v) : pos(x, y, z), texCoord(u, v) {}
	Vertex() {
		pos.x = 0.0f;
		pos.y = 0.0f;
		pos.z = 0.0f;

		texCoord.x = 0.0f;
		texCoord.y = 0.0f;

		numWeights = 0;

		weights.x = 0.0f;
		weights.y = 0.0f;
		weights.z = 0.0f;
		weights.w = 0.0f;
	}

	XMFLOAT3 pos;
	XMFLOAT2 texCoord;

	//-----------------------
	// for gpu sknning by kj 
	//-----------------------
	XMFLOAT4 weights;
	
	XMFLOAT4 weightPosX;
	XMFLOAT4 weightPosY;
	XMFLOAT4 weightPosZ;
	BYTE boneIndices[4];
	//-----------------------

	int weightIndex;
	int numWeights;
};
struct TriFace {
	TriFace() {
		indices[0] = 0;
		indices[1] = 0;
		indices[2] = 0;
	}
	TriFace(int pIndices[]) {
		indices[0] = pIndices[0];
		indices[1] = pIndices[1];
		indices[2] = pIndices[2];
	}
	TriFace(int i0, int i1, int i2) {
		indices[0] = i0;
		indices[1] = i1;
		indices[2] = i2;
	}
	int indices[3];
};
struct QuadFace {
	QuadFace() {
		indices[0] = -1;
		indices[1] = -1;
		indices[2] = -1;
		indices[3] = -1;
	}
	QuadFace(int pIndices[]) {
		indices[0] = pIndices[0];
		indices[1] = pIndices[1];
		indices[2] = pIndices[2];
		indices[3] = pIndices[3];
	}
	int indices[4];
};
struct BoundingBox {
	BoundingBox() {
		for (int i = 0; i < 8; i++) {
			vertices[i] = Vertex();
		}
		for (int i = 0; i < 6; i++) {
			quadFaces[i] = QuadFace();
		}
		for (int i = 0; i < 8; i++) {
			triFaces[i] = TriFace();
		}
	}
	Vertex vertices[8];
	TriFace triFaces[6];
	QuadFace quadFaces[6];
};

class Component;

class GameObject {
public:
	//vector<Component *> components;
	map<string, Component *> components;

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
	GameObject();
	GameObject(string objFilePathWithFilname);
	virtual ~GameObject();

	virtual bool Init();

	virtual bool loadFile(string filePath);
	virtual bool loadMesh(string filePath, bool boxColliderOn);

	virtual void Update();
	virtual void Render();
	virtual void UpdatePipeline();

	virtual void Cleanup();

	Component *getComponent(string componentName);
};

bool intersect3DTriangleWithRay(XMFLOAT3 rayPos, XMFLOAT3 rayDir, XMFLOAT3 P1, XMFLOAT3 P2, XMFLOAT3 P3, XMFLOAT3 *intersectionPoint);
Font LoadFont(LPCWSTR filename, int windowWidth, int windowHeight);
