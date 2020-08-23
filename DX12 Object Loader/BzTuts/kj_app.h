#pragma once

//#define MOVE_BONE_TO_MOUSE_LEFT_CLICK_CODE_TURN_ON

#include "stdafx.h"

#include "d3dApp.h"
#include "kj_app.h"

#include "game_object.h"

#include "bone.h"
#include "util.h"


#define EQUALS_FLOAT(A, B) !(A - 0.0001f <= B && A + 0.0001f >= B)

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#define CAMERA_DEFAULT_DISTANCE_FROM_VIEW_OBJECT 10.0f

#define CAMERA_DEFAULT_X 0.0f
#define CAMERA_DEFAULT_Y 0.0f
#define CAMERA_DEFAULT_Z (CAMERA_DEFAULT_DISTANCE_FROM_VIEW_OBJECT * -1.0f)

#define MOUSE_ROTATION_VERTICAL_DIRECTION 1.0f
#define MOUSE_ROTATION_HORIZONTAL_DIRECTION -1.0f

#define MOUSE_ROTATION_VERTICAL_SPEED 1.0f
#define MOUSE_ROTATION_HORIZONTAL_SPEED 1.0f

#define MOUSE_ROTATION_VERTICAL_RESOLUTION 10.0f
#define MOUSE_ROTATION_HORIZONTAL_RESOLUTION 10.0f

#define MOUSE_PANNING_VERTICAL_DIRECTION 1.0f
#define MOUSE_PANNING_HORIZONTAL_DIRECTION -1.0f

#define MOUSE_PANNING_VERTICAL_SPEED 0.1f
#define MOUSE_PANNING_HORIZONTAL_SPEED 0.1f

#define MOUSE_PANNING_VERTICAL_RESOLUTION 5.0f
#define MOUSE_PANNING_HORIZONTAL_RESOLUTION 5.0f

class KjApp : public D3DApp
{
public:
	KjApp(HINSTANCE hInstance);
	~KjApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void Cleanup();

	void OnCreate(HWND hwnd);
	void OnCommand(WPARAM wParam);
	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);
	void OnMouseWheel(WPARAM zDelta);
	void OnKeyDown(WPARAM keyState);

private:
	//-------------------------------------------------------------------------
	// DX12(kj): (main) initialize
	//-------------------------------------------------------------------------
	void BuildRootSignature();
	void BuildShadersAndInputLayout();

	void BuildMaterials();

	//void BuildRenderItems(); // 아래의 initObject()가 그 역할
	//bool initObject(int objIndex); // helper function of InitD3D()
	void BuildDescriptorHeaps();
	void LoadTextures(int objIndex);
	//void BuildCarGeometry();

	void BuildPSOs();
	void BuildFrameResources();

	//-------------------------------------------------------------------------
	// DX11
	//-------------------------------------------------------------------------
	//void BuildGeometryBuffers();	// dx11
	//void BuildFX();				// dx11
	//void BuildVertexLayout();		// dx11

	//-------------------------------------------------------------------------
	// DX12(kj): main interface
	//-------------------------------------------------------------------------
	void Update(double delta); // update the game logic
	void UpdatePipeline(int objIndex); // update the direct3d pipeline (update command lists)
	void Render(); // execute the command list
	//-------------------------------------------------------------------------
	// DX12(kj): private helper functions
	//-------------------------------------------------------------------------
	void UpdateCameraNewVer0001();
	void UpdateCameraOriginal();
	void UpdateCamera();
	void UpdateCameraByMovingCameraAndCameraTarget();
	//-------------------------------------------------------------------------


	//-------------------------------------------------------------------------
	// On Screen Text
	//-------------------------------------------------------------------------
	Font LoadFont(LPCWSTR filename, int windowWidth, int windowHeight); // load a font

	void RenderText(Font font, std::wstring text, XMFLOAT2 pos, XMFLOAT2 scale = XMFLOAT2(1.0f, 1.0f), XMFLOAT2 padding = XMFLOAT2(0.5f, 0.0f), XMFLOAT4 color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
	//-------------------------------------------------------------------------

public:
	//-------------------------------------------------------------------------
	// DX12(kj)
	//-------------------------------------------------------------------------

	//-------------------------------------------------------------------------
	// On Screen Text
	//-------------------------------------------------------------------------

	//int maxNumTextCharacters = 1024; // the maximum number of characters you can render during a frame. This is just used to make sure
	//								// there is enough memory allocated for the text vertex buffer each frame
	//
	ID3D12PipelineState* textPSO; // pso containing a pipeline state

	D3D12_SHADER_BYTECODE textPixelShaderBytecode;
	D3D12_SHADER_BYTECODE textVertexShaderBytecode;
	D3D12_INPUT_ELEMENT_DESC textInputLayout[3] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 }
	};

	D3D12_INPUT_LAYOUT_DESC textInputLayoutDesc;

	UINT8* textVBGPUAddress[frameBufferCount]; // this is a pointer to each of the constant buffer resource heaps
	//-------------------------------------------------------------------------

	//BYTE * imageData;

	D3D12_SHADER_BYTECODE vertexShaderBytecode;
	D3D12_SHADER_BYTECODE pixelShaderBytecode;
	D3D12_INPUT_ELEMENT_DESC inputLayout[7] =
	{
		// DXGI format for position seems a bit odd as on cpu side it is stated as an float3 but is used as a
		// float4 on the vertex shader. Changing either of the two to match the other results in an error so 
		// we are leaving it as it is.
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },

		//-----------------------
		// for gpu sknning by kj 
		//-----------------------
		{ "WEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "WEIGHTS_POS_X", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "WEIGHTS_POS_Y", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 52, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "WEIGHTS_POS_Z", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 68, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BONE_INDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT, 0, 84, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		//-----------------------
	};

	ID3D12PipelineState* pipelineStateObject; // pso containing a pipeline state
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;

	ID3D12RootSignature* rootSignature; // root signature defines data shaders will access
	D3D12_VIEWPORT viewport; // area that output from rasterizer will be stretched to.
	D3D12_RECT scissorRect; // the area to draw in. pixels outside that area will not be drawn onto

	ID3D12DescriptorHeap *dsDescriptorHeap;
	ID3D12Resource* depthStencilBuffer; // currently not used:  per object??????????????????????? 가져와야 하나?

	// this is the structure of our constant buffer.
	struct ConstantBufferPerObject {
		XMFLOAT4X4 wvpMat;
		//-----------------------
		// for gpu sknning by kj 
		//-----------------------
		//XMFLOAT4 test;
		XMFLOAT4 bonePos[256];
		XMFLOAT4 boneRot[256];
		//-----------------------
	};

	// Constant buffers must be 256-byte aligned which has to do with constant reads on the GPU.
	// We are only able to read at 256 byte intervals from the start of a resource heap, so we will
	// make sure that we add padding between the two constant buffers in the heap (one for cube1 and one for cube2)
	// Another way to do this would be to add a float array in the constant buffer structure for padding. In this case
	// we would need to add a float padding[50]; after the wvpMat variable. This would align our structure to 256 bytes (4 bytes per float)
	// The reason i didn't go with this way, was because there would actually be wasted cpu cycles when memcpy our constant
	// buffer data to the gpu virtual address. currently we memcpy the size of our structure, which is 16 bytes here, but if we
	// were to add the padding array, we would memcpy 64 bytes if we memcpy the size of our structure, which is 50 wasted bytes
	// being copied.
	int ConstantBufferPerObjectAlignedSize = (sizeof(ConstantBufferPerObject) + 255) & ~255;

	ConstantBufferPerObject cbPerObject; // this is the constant buffer data we will send to the gpu 
										 // (which will be placed in the resource we created above)

	ID3D12Resource* constantBufferUploadHeaps[frameBufferCount]; // this is the memory on the gpu where constant buffers for each frame will be placed

	UINT8* cbvGPUAddress[frameBufferCount]; // this is a pointer to each of the constant buffer resource heaps

	XMFLOAT4X4 cameraProjMat; // this will store our projection matrix
	XMFLOAT4X4 cameraViewMat; // this will store our view matrix

	XMFLOAT4X4 uiCameraProjMat; // (chai:20180706) ui용은 cameraProjMat보다 near clipping plane이 더 가까워야 하므로 따로 만든다.

	XMFLOAT4 cameraPosition; // this is our cameras position vector
	XMFLOAT4 cameraTarget; // a vector describing the point in space our camera is looking at
	XMFLOAT4 cameraUp; // the worlds up vector




	XMFLOAT4X4 cube2WorldMat; // our first cubes world matrix (transformation matrix)
	XMFLOAT4X4 cube2RotMat; // this will keep track of our rotation for the second cube
	XMFLOAT4 cube2PositionOffset; // our second cube will rotate around the first cube, so this is the position offset from the first cube

	

	//-----------------------------------------------------------------------------
	//-----------------------------------------------------------------------------
	//-----------------------------------------------------------------------------
	// Global Variables: Custom
	//-----------------------------------------------------------------------------
	//-----------------------------------------------------------------------------
	//-----------------------------------------------------------------------------

	//-----------------------------------------------------------------------------
	// for blender cube
	//-----------------------------------------------------------------------------

	XMFLOAT4X4 myCube1WorldMat; // our first cubes world matrix (transformation matrix)
								//XMFLOAT4X4 myCube1RotMat; // this will keep track of our rotation for the first cube
	XMFLOAT4 myCube1Position; // our first cubes position in space

	void DisplayError(LPTSTR lpszFunction);

	char g_buffer[280 + 1];
	int numObjModels = 0;
	vector<GameObject *> objModels;

	//-----------------------------------------------------------------------------
	// for mouse camera: ver0042 by kj
	//-----------------------------------------------------------------------------
	int frames = 0;

	int curCursorPosX = 0;
	int curCursorPosY = 0;
	int prevCursorPosX = 0;
	int prevCursorPosY = 0;

	float curCameraDegreeOnXZPlane = 270.0f; // camera angle between x and z plane
	float curCameraDegreeOnXYPlane = 0.0f;   // camera angle between x and y plane

	float xOnCameraPanningPlane = 0.0f;  // camera panning x offset
	float yOnCameraPanningPlane = 0.0f; // camera panning y offset
	
										
	float curCameraDistanceFromViewObject = 10.0f; // camera distance from centre 

	int objectSelected = -1;

	short zDelta = 0;
	int initHwndsCount = 0;

	//-------------------------------------------------------------------------
	// DX11
	//-------------------------------------------------------------------------
	//ID3D11Buffer * mBoxVB;
	//ID3D11Buffer* mBoxIB;

	//ID3DX11Effect* mFX;
	//ID3DX11EffectTechnique* mTech;
	//ID3DX11EffectMatrixVariable* mfxWorldViewProj;

	//ID3D11InputLayout* mInputLayout;

	//XMFLOAT4X4 mWorld;
	//XMFLOAT4X4 mView;
	//XMFLOAT4X4 mProj;

	//float mTheta;
	//float mPhi;
	//float mRadius;

	POINT mLastMousePos;
};
