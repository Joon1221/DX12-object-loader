#pragma once

#include "stdafx.h"
#include "component.h"
#include "renderable.h"
#include "game_object.h"

//using namespace std;

struct FontKerning;
struct FontChar;
struct Font;
struct TextVertex;

class  OnScreenText : public Renderable {
public:
	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------
	// Member Variables
	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------

	int maxNumTextCharacters = 1024; // the maximum number of characters you can render during a frame. This is just used to make sure
									// there is enough memory allocated for the text vertex buffer each frame

	Font arialFont; // this will store our arial font information

	ID3D12Resource* textVertexBuffer[3];
	D3D12_VERTEX_BUFFER_VIEW textVertexBufferView[3]; // a view for our text vertex buffer
	ID3D12Resource* depthStencilBuffer; // currently not used 


	//-------------------------------------------------------------------------
	// Init()/Render() related
	//-------------------------------------------------------------------------
	ID3D12DescriptorHeap *mainDescriptorHeap; // per object??????????????????????? 
	ID3D12DescriptorHeap *dsDescriptorHeap;

	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------
	// Member Functions
	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------
	OnScreenText();
	OnScreenText(string filePath);
	virtual ~OnScreenText();

	virtual bool Init();
	virtual bool loadFile(string filePath);
	virtual void Render();

	void RenderText(Font font, std::wstring text, XMFLOAT2 pos, XMFLOAT2 scale, XMFLOAT2 padding, XMFLOAT4 color);
};
