#include "d3dApp.h"
#include "kj_app.h"

#include "game_object.h"

#include "bone.h"
#include "util.h"

#include "component.h"
#include "transform.h"
#include "mesh_renderer.h"
#include "mesh.h"
#include "box_collider.h"
#include "animator.h"
#include "avatar.h"
#include "on_screen_text.h"

//#include "d3dx11Effect.h"
//#include "MathHelper.h"

//struct Vertex
//{
//	XMFLOAT3 Pos;
//	XMFLOAT4 Color;
//};

#include <Windows.h>
#include <iostream>
#include <sstream>

void DBOut(const char *file, const int line, const WCHAR *s)
{
	std::wostringstream os_;
	os_ << file << "(" << line << "): ";
	os_ << s;
	OutputDebugStringW(os_.str().c_str());
}

#define DBOUT(s)       DBOut(__FILE__, __LINE__, s)

extern D3DApp* gd3dApp;

LRESULT CALLBACK
subEditProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_RETURN:
			//Do your stuff
			gd3dApp->outputConsole("world\r\n");
			break;  //or return 0; if you don't want to pass it further to def proc
					//If not your key, skip to default:
		}
	default:
		return CallWindowProc(gd3dApp->oldEditProc, wnd, msg, wParam, lParam);
	}
	return 0;
}

HINSTANCE ghInstance;
int gnShowCmd;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// 나중에 D3DApp.cpp의 InitMainWindow()에서 parameter로 넘겨받지 않고..
	// extern으로 사용할 수 있도록 함.
	ghInstance = hInstance;
	gnShowCmd = showCmd;

	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	KjApp theApp(hInstance);

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}

KjApp::KjApp(HINSTANCE hInstance)
	: D3DApp(hInstance)
	//, mBoxVB(0), mBoxIB(0), mFX(0), mTech(0),
	//mfxWorldViewProj(0), mInputLayout(0),
	//mTheta(1.5f*MathHelper::Pi), mPhi(0.25f*MathHelper::Pi), mRadius(5.0f)
{
	mMainWndCaption = L"Box Demo";

	initHwndsCount = 0;

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	//XMMATRIX I = XMMatrixIdentity();
	//XMStoreFloat4x4(&mWorld, I);
	//XMStoreFloat4x4(&mView, I);
	//XMStoreFloat4x4(&mProj, I);

	// Works
	//DBOUT(L"\nKjApp::KjApp()1: hello-----------------------------------------------\n");
	//OutputDebugString(L"KjApp::KjApp()2: hello-----------------------------------------------\n");
	//OutputDebugStringW(L"c:\users\chai31\documents\study\kj\blender_importer_exporter\bztuts\bztuts\kj_app.cpp(99): \n");
}

KjApp::~KjApp()
{
	//ReleaseCOM(mBoxVB);
	//ReleaseCOM(mBoxIB);
	//ReleaseCOM(mFX);
	//ReleaseCOM(mInputLayout);
}

// create root signature
void KjApp::BuildRootSignature() {
	// create a root descriptor, which explains where to find the data for this root parameter
	D3D12_ROOT_DESCRIPTOR rootCBVDescriptor;
	rootCBVDescriptor.RegisterSpace = 0;
	rootCBVDescriptor.ShaderRegister = 0;

	// create a descriptor range (descriptor table) and fill it out
	// this is a range of descriptors inside a descriptor heap
	D3D12_DESCRIPTOR_RANGE  descriptorTableRanges[1]; // only one range right now
	descriptorTableRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // this is a range of shader resource views (descriptors)
	descriptorTableRanges[0].NumDescriptors = 1; // we only have one texture right now, so the range is only 1
	descriptorTableRanges[0].BaseShaderRegister = 0; // start index of the shader registers in the range
	descriptorTableRanges[0].RegisterSpace = 0; // space 0. can usually be zero
	descriptorTableRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // this appends the range to the end of the root signature descriptor tables

																									   // create a descriptor table
	D3D12_ROOT_DESCRIPTOR_TABLE descriptorTable;
	descriptorTable.NumDescriptorRanges = _countof(descriptorTableRanges); // we only have one range
	descriptorTable.pDescriptorRanges = &descriptorTableRanges[0]; // the pointer to the beginning of our ranges array

																   // create a root parameter for the root descriptor and fill it out
	D3D12_ROOT_PARAMETER  rootParameters[2]; // only one parameter right now
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // this is a constant buffer view root descriptor
	rootParameters[0].Descriptor = rootCBVDescriptor; // this is the root descriptor for this root parameter
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; // our pixel shader will be the only shader accessing this parameter for now

																		 // fill out the parameter for our descriptor table. Remember it's a good idea to sort parameters by frequency of change. Our constant
																		 // buffer will be changed multiple times per frame, while our descriptor table will not be changed at all (in this tutorial)
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // this is a descriptor table
	rootParameters[1].DescriptorTable = descriptorTable; // this is our descriptor table for this root parameter
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // our pixel shader will be the only shader accessing this parameter for now

																		// create a static sampler
	D3D12_STATIC_SAMPLER_DESC sampler = {};
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.MipLODBias = 0;
	sampler.MaxAnisotropy = 0;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	sampler.MinLOD = 0.0f;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;
	sampler.ShaderRegister = 0;
	sampler.RegisterSpace = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(_countof(rootParameters), // we have 2 root parameters
		rootParameters, // a pointer to the beginning of our root parameters array
		1, // we have one static sampler
		&sampler, // a pointer to our static sampler (array)
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | // we can deny shader stages here for better performance
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);

	ID3DBlob* errorBuff; // a buffer holding the error data if any
	ID3DBlob* signature;
	HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &errorBuff);
	if (FAILED(hr))
	{
		OutputDebugStringA((char*)errorBuff->GetBufferPointer());
		Running = false;
		//return false;
		return;
	}

	hr = device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	if (FAILED(hr))
	{
		Running = false;
		//return false;
		return;
	}
}

void KjApp::BuildShadersAndInputLayout() {
	// create vertex and pixel shaders

	// when debugging, we can compile the shader files at runtime.
	// but for release versions, we can compile the hlsl shaders
	// with fxc.exe to create .cso files, which contain the shader
	// bytecode. We can load the .cso files at runtime to get the
	// shader bytecode, which of course is faster than compiling
	// them at runtime

	// compile vertex shader
	ID3DBlob* errorBuff; // a buffer holding the error data if any

	ID3DBlob* vertexShader; // d3d blob for holding vertex shader bytecode
	HRESULT hr = D3DCompileFromFile(L"VertexShader.hlsl",
		nullptr,
		nullptr,
		"main",
		"vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&vertexShader,
		&errorBuff);
	if (FAILED(hr))
	{
		OutputDebugStringA((char*)errorBuff->GetBufferPointer());
		Running = false;
		//return false;
		return;
	}

	// fill out a shader bytecode structure, which is basically just a pointer
	// to the shader bytecode and the size of the shader bytecode
	//D3D12_SHADER_BYTECODE vertexShaderBytecode = {};
	vertexShaderBytecode = {};
	vertexShaderBytecode.BytecodeLength = vertexShader->GetBufferSize();
	vertexShaderBytecode.pShaderBytecode = vertexShader->GetBufferPointer();

	// compile pixel shader
	ID3DBlob* pixelShader;
	hr = D3DCompileFromFile(L"PixelShader.hlsl",
		nullptr,
		nullptr,
		"main",
		"ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&pixelShader,
		&errorBuff);
	if (FAILED(hr))
	{
		OutputDebugStringA((char*)errorBuff->GetBufferPointer());
		Running = false;
		//return false;
		return;
	}

	// fill out shader bytecode structure for pixel shader
	//D3D12_SHADER_BYTECODE pixelShaderBytecode = {};
	pixelShaderBytecode = {};
	pixelShaderBytecode.BytecodeLength = pixelShader->GetBufferSize();
	pixelShaderBytecode.pShaderBytecode = pixelShader->GetBufferPointer();

	// fill out an input layout description structure
	inputLayoutDesc = {};

	// we can get the number of elements in an array by "sizeof(array) / sizeof(arrayElementType)"
	inputLayoutDesc.NumElements = sizeof(inputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC);
	inputLayoutDesc.pInputElementDescs = inputLayout;

	// Text PSO

	//-------------------------------------------------------------------------
	// On Screen Text
	//-------------------------------------------------------------------------
	// compile vertex shader
	ID3DBlob* textVertexShader; // d3d blob for holding vertex shader bytecode
	hr = D3DCompileFromFile(L"TextVertexShader.hlsl",
		nullptr,
		nullptr,
		"main",
		"vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&textVertexShader,
		&errorBuff);
	if (FAILED(hr))
	{
		OutputDebugStringA((char*)errorBuff->GetBufferPointer());
		Running = false;
		return;
	}

	// fill out a shader bytecode structure, which is basically just a pointer
	// to the shader bytecode and the size of the shader bytecode
	textVertexShaderBytecode = {};
	textVertexShaderBytecode.BytecodeLength = textVertexShader->GetBufferSize();
	textVertexShaderBytecode.pShaderBytecode = textVertexShader->GetBufferPointer();

	// compile pixel shader
	ID3DBlob* textPixelShader;
	hr = D3DCompileFromFile(L"TextPixelShader.hlsl",
		nullptr,
		nullptr,
		"main",
		"ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&textPixelShader,
		&errorBuff);
	if (FAILED(hr))
	{
		OutputDebugStringA((char*)errorBuff->GetBufferPointer());
		Running = false;
		return;
	}

	// fill out shader bytecode structure for pixel shader
	textPixelShaderBytecode = {};
	textPixelShaderBytecode.BytecodeLength = textPixelShader->GetBufferSize();
	textPixelShaderBytecode.pShaderBytecode = textPixelShader->GetBufferPointer();

	// create input layout

	// The input layout is used by the Input Assembler so that it knows
	// how to read the vertex data bound to it.

	// fill out an input layout description structure
	textInputLayoutDesc = {};

	// we can get the number of elements in an array by "sizeof(array) / sizeof(arrayElementType)"
	textInputLayoutDesc.NumElements = sizeof(textInputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC);
	textInputLayoutDesc.pInputElementDescs = textInputLayout;
	//-------------------------------------------------------------------------

}

void KjApp::BuildPSOs() {
	// create a pipeline state object (PSO)

	// In a real application, you will have many pso's. for each different shader
	// or different combinations of shaders, different blend states or different rasterizer states,
	// different topology types (point, line, triangle, patch), or a different number
	// of render targets you will need a pso

	// VS is the only required shader for a pso. You might be wondering when a case would be where
	// you only set the VS. It's possible that you have a pso that only outputs data with the stream
	// output, and not on a render target, which means you would not need anything after the stream
	// output.

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {}; // a structure to define a pso
	psoDesc.InputLayout = inputLayoutDesc; // the structure describing our input layout
	psoDesc.pRootSignature = rootSignature; // the root signature that describes the input data this pso needs
	psoDesc.VS = vertexShaderBytecode; // structure describing where to find the vertex shader bytecode and how large it is
	psoDesc.PS = pixelShaderBytecode; // same as VS but for pixel shader
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; // type of topology we are drawing
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // format of the render target
	psoDesc.SampleDesc = sampleDesc; // must be the same sample description as the swapchain and depth/stencil buffer
	psoDesc.SampleMask = 0xffffffff; // sample mask has to do with multi-sampling. 0xffffffff means point sampling is done
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT); // a default rasterizer state.
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT); // a default blent state.
	psoDesc.NumRenderTargets = 1; // we are only binding one render target
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT); // a default depth stencil state

	// create the pso
	HRESULT hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineStateObject));
	if (FAILED(hr))
	{
		Running = false;
		//return false;
		return;
	}

	// create the text pipeline state object (PSO)
	D3D12_GRAPHICS_PIPELINE_STATE_DESC textpsoDesc = {};
	textpsoDesc.InputLayout = textInputLayoutDesc;
	textpsoDesc.pRootSignature = rootSignature;
	textpsoDesc.VS = textVertexShaderBytecode;
	textpsoDesc.PS = textPixelShaderBytecode;
	textpsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	textpsoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	textpsoDesc.SampleDesc = sampleDesc;
	textpsoDesc.SampleMask = 0xffffffff;
	textpsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);

	D3D12_BLEND_DESC textBlendStateDesc = {};
	textBlendStateDesc.AlphaToCoverageEnable = FALSE;
	textBlendStateDesc.IndependentBlendEnable = FALSE;
	textBlendStateDesc.RenderTarget[0].BlendEnable = TRUE;

	textBlendStateDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	textBlendStateDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
	textBlendStateDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;

	textBlendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_SRC_ALPHA;
	textBlendStateDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;
	textBlendStateDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;

	textBlendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	textpsoDesc.BlendState = textBlendStateDesc;
	textpsoDesc.NumRenderTargets = 1;
	D3D12_DEPTH_STENCIL_DESC textDepthStencilDesc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	textDepthStencilDesc.DepthEnable = false;
	textpsoDesc.DepthStencilState = textDepthStencilDesc;

	// create the text pso
	hr = device->CreateGraphicsPipelineState(&textpsoDesc, IID_PPV_ARGS(&textPSO));
	if (FAILED(hr))
	{
		Running = false;
		return;
	}
}

void KjApp::BuildMaterials() {

}

void KjApp::BuildDescriptorHeaps() {


	// create a depth stencil descriptor heap so we can get a pointer to the depth stencil buffer
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	//hr = device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsDescriptorHeaps[objIndex]));
	HRESULT hr = gd3dApp->device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsDescriptorHeap));
	if (FAILED(hr))
	{
		gd3dApp->Running = false;
		return;
	}

	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
	depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
	depthOptimizedClearValue.DepthStencil.Stencil = 0;

	//hr = device->CreateCommittedResource(
	hr = gd3dApp->device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		//&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, Width, Height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
		&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, gd3dApp->Width, gd3dApp->Height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthOptimizedClearValue,
		IID_PPV_ARGS(&depthStencilBuffer)
	);
	if (FAILED(hr))
	{
		gd3dApp->Running = false;
		return;
	}
	//dsDescriptorHeaps[objIndex]->SetName(L"Depth/Stencil Resource Heap");
	dsDescriptorHeap->SetName(L"Depth/Stencil Resource Heap");

	//device->CreateDepthStencilView(depthStencilBuffer, &depthStencilDesc, dsDescriptorHeaps[objIndex]->GetCPUDescriptorHandleForHeapStart());
	gd3dApp->device->CreateDepthStencilView(depthStencilBuffer, &depthStencilDesc, dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

}

void KjApp::LoadTextures(int objIndex) {

}

void KjApp::BuildFrameResources() {

}

void KjApp::Cleanup() {
	D3DApp::Cleanup();

	SAFE_RELEASE(pipelineStateObject);
	SAFE_RELEASE(rootSignature);

	for (int i = 0; i < frameBufferCount; ++i)
	{
		SAFE_RELEASE(constantBufferUploadHeaps[i]);
	};
	for (int i = 0; i < numObjModels; i++) {
		objModels[i]->Cleanup();
	}
}

// Box Collider(=Bounding Box)가 제대로 동작하는지 보기 위해..
// 아래의 flag를 끄면, Monkey만 일단 나오게 한다.
#define ALL_OBJECTS_ON

bool KjApp::Init()
{

	//================================================================================
	//================================================================================
	//================================================================================
	// Intialize DX12 Requirements
	//================================================================================
	//================================================================================
	//================================================================================

	if (!D3DApp::Init())
		return false;

	HRESULT hr;

	BuildRootSignature();
	BuildShadersAndInputLayout();	// initObject()안에서 call됨.
	BuildPSOs();
	BuildDescriptorHeaps();		// initObject()안에서 call됨.
	BuildMaterials();				// initObject()안에서 call됨.
	BuildFrameResources();

	//================================================================================
	//================================================================================
	//================================================================================
	// Intialize Objects
	//================================================================================
	//================================================================================
	//================================================================================


#ifdef ALL_OBJECTS_ON

	// object paths
	char objPaths[3][256] = {
		"..\\..\\imp_exp\\grid\\grid.obj",
		"..\\..\\imp_exp\\grid\\grid.obj",
		"..\\..\\imp_exp\\axis\\axis.obj",
		//"..\\..\\imp_exp\\bounding_box.obj",
		//"..\\..\\imp_exp\\character_creation.md5"
		//	"..\\..\\imp_exp\\boblampclean.md5"
	};

	// object uv image paths
	char objImagePaths[3][256] = {
		"..\\..\\imp_exp\\ash_uvgrid03.jpg",
		"..\\..\\imp_exp\\ash_uvgrid03.jpg",
		"..\\..\\imp_exp\\axis\\axis.png",
		//"..\\..\\imp_exp\\ash_uvgrid03.jpg",
		//"..\\..\\imp_exp\\character_creation.png"
	};

	// states whether or not objects should have a box collider
	bool boxColliderOn[3] = {
		false,
		false,
		false,
	};

	// create 3 blank object models
	objModels.push_back(new GameObject());
	objModels.push_back(new GameObject());
	objModels.push_back(new GameObject());

	// load objects
	for (int i = 0; i < objModels.size(); i++) {
		// load mesh
		if (!objModels[i]->loadMesh(objPaths[i], boxColliderOn[i])) {
			MessageBox(NULL, L"Unable to load ObjModel", L"Error:KjApp::init()", MB_OK | MB_ICONERROR);
			return false;
		}

		((Mesh *)(objModels[i]->getComponent("Mesh")))->imagePath = objImagePaths[i];

		// init mesh
		if (!objModels[i]->Init()) {
			MessageBox(NULL, L"Unable to initialize ObjModel", L"Error:KjApp::init()", MB_OK | MB_ICONERROR);
			return false;
		}
		numObjModels++;
	}

	// set values for ui axis
	((Transform *)(objModels[2]->getComponent("Transform")))->uiObject = true;
	((Transform *)(objModels[2]->getComponent("Transform")))->offsetPos = XMVectorSet (-0.04f, -0.03f, 0.1f, 0.0f);
	((Transform *)(objModels[2]->getComponent("Transform")))->scaleMat = XMMatrixScaling(0.01f, 0.01f, 0.01f);

	//-------------------------------------------------------
	// mouse picking triangle
	//-------------------------------------------------------

	// create game object for triangle
	GameObject *tempGameObj = new GameObject();
	objModels.push_back(tempGameObj);
	
	// load triangle
	tempGameObj->loadMesh("..\\..\\imp_exp\\triangle\\triangle.obj", boxColliderOn[3]);
	((Mesh *)(tempGameObj->getComponent("Mesh")))->imagePath = "..\\..\\imp_exp\\ash_uvgrid03.jpg";
	
	// intialize trangle
	tempGameObj->Init();
	numObjModels++;
	//-------------------------------------------------------
	// bob lamp
	//-------------------------------------------------------

	// create game objetc for bob lamp
	GameObject *tempGameObj2 = new GameObject();
	objModels.push_back(tempGameObj2);

	// load bob lamp
	tempGameObj2->loadMesh("..\\..\\imp_exp\\bob\\boblampclean.md5mesh", false);

	// intialize animator
	Animator *curAnimator = (Animator *)(tempGameObj2->getComponent("Animator"));
	curAnimator->avatar->meshes[0]->imagePath = "..\\..\\imp_exp\\bob\\guard1_body.png";
	curAnimator->avatar->meshes[1]->imagePath = "..\\..\\imp_exp\\bob\\guard1_face.png";
	curAnimator->avatar->meshes[2]->imagePath = "..\\..\\imp_exp\\bob\\guard1_helmet.png";
	curAnimator->avatar->meshes[3]->imagePath = "..\\..\\imp_exp\\bob\\iron_grill.png";
	curAnimator->avatar->meshes[4]->imagePath = "..\\..\\imp_exp\\bob\\round_grill.png";
	curAnimator->avatar->meshes[5]->imagePath = "..\\..\\imp_exp\\bob\\guard1_body.png";
		
	// intialize bob lamp
	tempGameObj2->Init();
	((Transform *)tempGameObj2->getComponent("Transform"))->scaleMat = XMMatrixScaling(0.1f, 0.1f, 0.1f);
	((Animator *)tempGameObj2->getComponent("Animator"))->avatar->loadMD5Anim("..\\..\\imp_exp\\bob\\boblampclean.md5anim");
	numObjModels++;

#else 
	GameObject *tempGameObj = new GameObject();
	objModels.push_back(tempGameObj);

	tempGameObj->loadMesh("..\\..\\imp_exp\\monkey.obj");
	((Mesh *)(tempGameObj->getComponent("Mesh")))->imagePath = "..\\..\\imp_exp\\ash_uvgrid03.jpg";

	tempGameObj->Init();
	numObjModels++;
#endif

	//================================================================================
	//================================================================================
	//================================================================================
	// Intialize Constant Buffer Heaps
	//================================================================================
	//================================================================================


	// create the constant buffer resource heap
	// We will update the constant buffer one or more times per frame, so we will use only an upload heap
	// unlike previously we used an upload heap to upload the vertex and index data, and then copied over
	// to a default heap. If you plan to use a resource for more than a couple frames, it is usually more
	// efficient to copy to a default heap where it stays on the gpu. In this case, our constant buffer
	// will be modified and uploaded at least once per frame, so we only use an upload heap

	// first we will create a resource heap (upload heap) for each frame for the cubes constant buffers
	// As you can see, we are allocating 64KB for each resource we create. Buffer resource heaps must be
	// an alignment of 64KB. We are creating 3 resources, one for each frame. Each constant buffer is 
	// only a 4x4 matrix of floats in this tutorial. So with a float being 4 bytes, we have 
	// 16 floats in one constant buffer, and we will store 2 constant buffers in each
	// heap, one for each cube, thats only 64x2 bits, or 128 bits we are using for each
	// resource, and each resource must be at least 64KB (65536 bits)
	for (int i = 0; i < frameBufferCount; ++i)
	{
		// create resource for cube 1
		HRESULT hr = device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // this heap will be used to upload the constant buffer data
			D3D12_HEAP_FLAG_NONE, // no flags
								  //&CD3DX12_RESOURCE_DESC::Buffer(1024 * 64), // size of the resource heap. Must be a multiple of 64KB for single-textures and constant buffers
			&CD3DX12_RESOURCE_DESC::Buffer(1024 * 64), // size of the resource heap. Must be a multiple of 64KB for single-textures and constant buffers
			D3D12_RESOURCE_STATE_GENERIC_READ, // will be data that is read from so we keep it in the generic read state
			nullptr, // we do not have use an optimized clear value for constant buffers
			IID_PPV_ARGS(&constantBufferUploadHeaps[i]));
		if (FAILED(hr))
		{
			Running = false;
			return false;
		}
		constantBufferUploadHeaps[i]->SetName(L"Constant Buffer Upload Resource Heap");

		ZeroMemory(&cbPerObject, sizeof(cbPerObject));

		CD3DX12_RANGE readRange(0, 0);	// We do not intend to read from this resource on the CPU. (so end is less than or equal to begin)

										// map the resource heap to get a gpu virtual address to the beginning of the heap
		hr = constantBufferUploadHeaps[i]->Map(0, &readRange, reinterpret_cast<void**>(&cbvGPUAddress[i]));

		// Because of the constant read alignment requirements, constant buffer views must be 256 bit aligned. Our buffers are smaller than 256 bits,
		// so we need to add spacing between the two buffers, so that the second buffer starts at 256 bits from the beginning of the resource heap.
		for (int j = 0; j < numObjModels; j++) {
			memcpy(cbvGPUAddress[i] + ConstantBufferPerObjectAlignedSize * j, &cbPerObject, sizeof(cbPerObject));
		}
	}


	//================================================================================
	//================================================================================
	//================================================================================
	// Intialize Vertex Buffer
	//================================================================================
	//================================================================================
	//================================================================================


	for (int i = 0; i < frameBufferCount; ++i)
	{

		// create upload heap. We will fill this with data for our text
		ID3D12Resource* vBufferUploadHeap;
		hr = gd3dApp->device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
			D3D12_HEAP_FLAG_NONE, // no flags
			&CD3DX12_RESOURCE_DESC::Buffer(((OnScreenText *)objModels[5]->getComponent("OnScreenText"))->maxNumTextCharacters * sizeof(TextVertex)), // resource description for a buffer
			D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
			nullptr,
			IID_PPV_ARGS(&((OnScreenText *)objModels[5]->getComponent("OnScreenText"))->textVertexBuffer[i]));
		if (FAILED(hr))
		{
			gd3dApp->Running = false;
			return false;
		}
		((OnScreenText *)objModels[5]->getComponent("OnScreenText"))->textVertexBuffer[i]->SetName(L"Text Vertex Buffer Upload Resource Heap");

		CD3DX12_RANGE readRange(0, 0);    // We do not intend to read from this resource on the CPU. (so end is less than or equal to begin)

		// map the resource heap to get a gpu virtual address to the beginning of the heap
		hr = ((OnScreenText *)objModels[5]->getComponent("OnScreenText"))->textVertexBuffer[i]->Map(0, &readRange, reinterpret_cast<void**>(&textVBGPUAddress[i]));
	}


	//-------------------------------------------------------------------------

	// Now we execute the command list to upload the initial assets (triangle data)
	commandList->Close();
	ID3D12CommandList* ppCommandLists[] = { commandList };
	commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	

	// increment the fence value now, otherwise the buffer might not be uploaded by the time we start drawing
	fenceValue[frameIndex]++;
	hr = commandQueue->Signal(fence[frameIndex], fenceValue[frameIndex]);
	if (FAILED(hr))
	{
		Running = false;
		return false;
	}

	// set the text vertex buffer view for each frame
	for (int i = 0; i < frameBufferCount; ++i)
	{
		((OnScreenText *)objModels[5]->getComponent("OnScreenText"))->textVertexBufferView[i].BufferLocation = ((OnScreenText *)objModels[5]->getComponent("OnScreenText"))->textVertexBuffer[i]->GetGPUVirtualAddress();
		((OnScreenText *)objModels[5]->getComponent("OnScreenText"))->textVertexBufferView[i].StrideInBytes = sizeof(TextVertex);
		((OnScreenText *)objModels[5]->getComponent("OnScreenText"))->textVertexBufferView[i].SizeInBytes = ((OnScreenText *)objModels[5]->getComponent("OnScreenText"))->maxNumTextCharacters * sizeof(TextVertex);
	}

	//================================================================================
	//================================================================================
	//================================================================================
	// Intialize Camera and View Related
	//================================================================================
	//================================================================================
	//================================================================================


	// Fill out the Viewport
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = Width;
	viewport.Height = Height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	// Fill out a scissor rect
	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = Width;
	scissorRect.bottom = Height;

	// build projection and view matrix
	XMMATRIX tmpMat = XMMatrixPerspectiveFovLH(45.0f*(3.14f / 180.0f), (float)Width / (float)Height, 0.1f, 1000.0f);
	XMStoreFloat4x4(&cameraProjMat, tmpMat);

	// (chai:20180706)
	XMMATRIX uiCameraTmpMat = XMMatrixPerspectiveFovLH(45.0f*(3.14f / 180.0f), (float)Width / (float)Height, 0.001f, 1000.0f);
	XMStoreFloat4x4(&uiCameraProjMat, uiCameraTmpMat);

	// set starting camera state
	cameraPosition = XMFLOAT4(CAMERA_DEFAULT_X, CAMERA_DEFAULT_Y, CAMERA_DEFAULT_Z, 0.0f);
	cameraTarget = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	cameraUp = XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f);

	// build view matrix
	XMVECTOR cPos = XMLoadFloat4(&cameraPosition);
	XMVECTOR cTarg = XMLoadFloat4(&cameraTarget);
	XMVECTOR cUp = XMLoadFloat4(&cameraUp);
	tmpMat = XMMatrixLookAtLH(cPos, cTarg, cUp);
	XMStoreFloat4x4(&cameraViewMat, tmpMat);

	return true;
}

void KjApp::OnResize()
{
	D3DApp::OnResize();
}

void KjApp::UpdateScene(float dt)
{
	double delta = timer.GetFrameDelta();
	Update(delta); // update the game logic
	
}

void KjApp::DrawScene()
{
	Render();
}

void KjApp::OnCreate(HWND hwnd) {
	// intialize menu bar
	if (initHwndsCount == 0) {
		HMENU hMenuBar = CreateMenu();
		HMENU hFile = CreateMenu();
		HMENU hOptions = CreateMenu();

		AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hFile, L"File");
		AppendMenu(hMenuBar, MF_POPUP, NULL, L"Edit");
		AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hOptions, L"Camera");

		AppendMenu(hFile, MF_STRING, 1, L"Exit");
		//AppendMenu(hFile, MF_STRING, 101, L"Open");
		AppendMenu(hOptions, MF_STRING, 2, L"+ X");
		AppendMenu(hOptions, MF_STRING, 3, L"- X");
		AppendMenu(hOptions, MF_STRING, 4, L"+ Y");
		AppendMenu(hOptions, MF_STRING, 5, L"- Y");
		AppendMenu(hOptions, MF_STRING, 6, L"+ Z");
		AppendMenu(hOptions, MF_STRING, 7, L"- Z");
		//AppendMenu(hOptions, MF_STRING, 8, L"Zoom In");
		//AppendMenu(hOptions, MF_STRING, 9, L"Zoom Out");

		SetMenu(hwnd, hMenuBar);
	}
	// intialize buttons and camera position text fields
	else if (initHwndsCount == 2) {
		button = CreateWindow(L"Button", L"Zoom In", WS_VISIBLE | WS_CHILD | WS_BORDER, 10, 10, 167, 20, hwnd, (HMENU)1000, NULL, NULL);
		button = CreateWindow(L"Button", L"Zoom Out", WS_VISIBLE | WS_CHILD | WS_BORDER, 10, 35, 167, 20, hwnd, (HMENU)1001, NULL, NULL);

		CreateWindow(L"Button", L"Camera Position", WS_VISIBLE | WS_CHILD | WS_BORDER | WS_DISABLED, 10, 75, 167, 20, hwnd, (HMENU)-1, NULL, NULL);
		cameraWindowXPos = CreateWindowEx(NULL, L"Edit", L"-1", WS_VISIBLE | WS_CHILD | WS_BORDER, 10, 100, 40, 20, hwnd, (HMENU)1002, NULL, NULL);
		cameraWindowYPos = CreateWindow(L"Edit", L"-1", WS_VISIBLE | WS_CHILD | WS_BORDER, 72, 100, 40, 20, hwnd, (HMENU)1003, NULL, NULL);
		cameraWindowZPos = CreateWindow(L"Edit", L"-1", WS_VISIBLE | WS_CHILD | WS_BORDER, 134, 100, 40, 20, hwnd, (HMENU)1004, NULL, NULL);

		CreateWindow(L"Button", L"Camera Target", WS_VISIBLE | WS_CHILD | WS_BORDER | WS_DISABLED, 10, 135, 167, 20, hwnd, (HMENU)-1, NULL, NULL);
		cameraWindowXTarget = CreateWindow(L"Edit", L"-1", WS_VISIBLE | WS_CHILD | WS_BORDER, 10, 160, 40, 20, hwnd, (HMENU)1005, NULL, NULL);
		cameraWindowYTarget = CreateWindow(L"Edit", L"-1", WS_VISIBLE | WS_CHILD | WS_BORDER, 72, 160, 40, 20, hwnd, (HMENU)1006, NULL, NULL);
		cameraWindowZTarget = CreateWindow(L"Edit", L"-1", WS_VISIBLE | WS_CHILD | WS_BORDER, 134, 160, 40, 20, hwnd, (HMENU)1007, NULL, NULL);
		button = CreateWindow(L"Button", L"Reset", WS_VISIBLE | WS_CHILD | WS_BORDER, 10, 200, 167, 20, hwnd, (HMENU)1008, NULL, NULL);

		objFilePath = CreateWindow(L"Edit", L"", WS_VISIBLE | WS_CHILD | WS_BORDER, 10, 234, 267, 20, hwnd, (HMENU)2000, NULL, NULL);
		objFilePathButton = CreateWindow(L"Button", L"Import Object", WS_VISIBLE | WS_CHILD | WS_BORDER, 10, 268, 167, 20, hwnd, (HMENU)2001, NULL, NULL);
	}
	// intialize console
	else if (initHwndsCount == 3) {
		console = CreateWindow(L"Edit", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_AUTOVSCROLL | ES_WANTRETURN | WS_HSCROLL | WS_VSCROLL | ES_MULTILINE, 0, 0, 974, 145, hwnd, (HMENU)1009, NULL, NULL);
		//outputConsole("hello\r\n");
		//outputConsole("hello\r\n");
		//outputConsole("hello\r\n");
		oldEditProc = (WNDPROC)SetWindowLongPtr(console, GWLP_WNDPROC, (LONG_PTR)subEditProc);
	}
	initHwndsCount++;
}

void KjApp::OnCommand(WPARAM wParam)
{
	switch (LOWORD(wParam)) {
		case 1:
			Running = false;
			DestroyWindow(hwnds[1]);
			break;
		case 2:
			cameraPosition.x = cameraPosition.x + 1;
			cameraTarget.x = cameraTarget.x + 1;
			break;
		case 3:
			cameraPosition.x = cameraPosition.x - 1;
			cameraTarget.x = cameraTarget.x - 1;
			break;
		case 4:
			cameraPosition.y = cameraPosition.y + 1;
			cameraTarget.y = cameraTarget.y + 1;
			break;
		case 5:
			cameraPosition.y = cameraPosition.y - 1;
			cameraTarget.y = cameraTarget.y - 1;
			break;
		case 6:
			cameraPosition.z = cameraPosition.z + 1;
			cameraTarget.z = cameraTarget.z + 1;
			break;
		case 7:
			cameraPosition.z = cameraPosition.z - 1;
			cameraTarget.z = cameraTarget.z - 1;
			break;

			//================================================================================
			//================================================================================
			//================================================================================
			// User Interface
			//================================================================================
			//================================================================================
			//================================================================================
		case 1000:
			curCameraDistanceFromViewObject--;
			break;
		case 1001:
			curCameraDistanceFromViewObject++;
			break;
		case 1002: {
			break;
		}

		case 1008:
			cameraTarget.x = 0.0f;
			cameraTarget.y = 0.0f;
			cameraTarget.z = 0.0f;

			xOnCameraPanningPlane = 0.0f;
			yOnCameraPanningPlane = 0.0f;

			curCameraDegreeOnXYPlane = 0.0f;
			curCameraDegreeOnXZPlane = 270.0f;

			curCameraDistanceFromViewObject = 10.0f;
			break;

		case 2000: {
			break;
		}
		case 2001: {
			/* Source code removed for privacy purposes */
			break;
		}
	}
}

void KjApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	/* Source code removed for privacy purposes */
}

void KjApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void KjApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	curCursorPosX = x;
	curCursorPosY = y;

	int xPosDifference = curCursorPosX - prevCursorPosX; // positive means moved to the right and negative means moved to the left
	int yPosDifference = curCursorPosY - prevCursorPosY; // positive means moved down and negative means moved up

	prevCursorPosX = curCursorPosX;
	prevCursorPosY = curCursorPosY;
	
	// chai:ver0049: panning
	if (btnState & MK_LBUTTON && btnState & MK_SHIFT) {

		//char buf[1024];
		//sprintf(buf, "%d %d", xPosDifference, yPosDifference);
		//msgBoxCstring(buf);

		if (xOnCameraPanningPlane == 0.0f) {
			xOnCameraPanningPlane = ((MOUSE_PANNING_HORIZONTAL_SPEED * xPosDifference / MOUSE_PANNING_HORIZONTAL_RESOLUTION) * MOUSE_PANNING_HORIZONTAL_DIRECTION);
		}
		if (yOnCameraPanningPlane == 0.0f) {
			yOnCameraPanningPlane = ((MOUSE_PANNING_VERTICAL_SPEED * yPosDifference / MOUSE_PANNING_VERTICAL_RESOLUTION) * MOUSE_PANNING_VERTICAL_DIRECTION);
		}
	}
	// chai:ver0048: 절대 아래를 위의 if보다 먼저 두면 안 되고, 두번째로, else를 꼭 붙여야 한다.
	else if (btnState & MK_LBUTTON) {
		//frames = 0;

		//char buf[1024];
		//sprintf(buf, "%d %d", xPosDifference, yPosDifference);
		//msgBoxCstring(buf);

		// move camera on sphere
		curCameraDegreeOnXZPlane += (MOUSE_ROTATION_HORIZONTAL_SPEED * xPosDifference / MOUSE_ROTATION_HORIZONTAL_RESOLUTION) * MOUSE_ROTATION_HORIZONTAL_DIRECTION; // left <-> right
		curCameraDegreeOnXYPlane += (MOUSE_ROTATION_VERTICAL_SPEED * yPosDifference / MOUSE_ROTATION_VERTICAL_RESOLUTION) * MOUSE_ROTATION_VERTICAL_DIRECTION; // up <-> down
	}
}

void KjApp::OnMouseWheel(WPARAM wParam) {
	//if (wParam & MK_SHIFT) {
	short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);


	if (zDelta < 0) { // zDelta == -120 이거나 zDelta == 120이거나..
		curCameraDistanceFromViewObject += 0.1f * (zDelta / 120.0f) * (zDelta / 120.0f); // scroll down or zoom out
	}
	else {
		if (curCameraDistanceFromViewObject - 0.1f * (zDelta / 120.0f) * (zDelta / 120.0f) >= 0.0f) {
			curCameraDistanceFromViewObject -= 0.1f * (zDelta / 120.0f) * (zDelta / 120.0f); // scroll up or zoom in
		}
	}
}

void KjApp::OnKeyDown(WPARAM keyState) {
	if (keyState == VK_ESCAPE) {
		if (MessageBox(0, L"Are you sure you want to exit?",
			L"Really?", MB_YESNO | MB_ICONQUESTION) == IDYES)
		{
			Running = false;
			DestroyWindow(hwnds[1]);
		}
	}
	if (keyState == VK_F1 && curCameraDistanceFromViewObject - 1.0f > 0) {
		curCameraDistanceFromViewObject--;
	}
	if (keyState == VK_F2) {
		curCameraDistanceFromViewObject++;
	}

	int xMovement = 0;
	int yMovement = 0;
	int zMovement = 0;

	// D: Positive X
	// A: Negative X
	// W: Positive Y
	// S: Negative Y
	// Q: Positive Z
	// E: Negative Z
	if (GetAsyncKeyState('D')) {
		xMovement++;
	}
	if (GetAsyncKeyState('A')) {
		xMovement--;
	}
	if (GetAsyncKeyState('W')) {
		yMovement++;
	}
	if (GetAsyncKeyState('S')) {
		yMovement--;
	}
	if (GetAsyncKeyState('E')) {
		zMovement++;
	}
	if (GetAsyncKeyState('Q')) {
		zMovement--;
	}

	if (objectSelected != -1) {
		Transform *curTransform = (Transform *)objModels.at(objectSelected)->getComponent("Transform");
		XMFLOAT4 curTransformPos;
		XMStoreFloat4(&curTransformPos, curTransform->pos); // store cube1's world matrix

		curTransform->pos = XMLoadFloat4(&XMFLOAT4(curTransformPos.x + xMovement, curTransformPos.y + yMovement, curTransformPos.z + zMovement, curTransformPos.w)); // set cube 1's position	
	}
}

//=============================================================================================================================
//=============================================================================================================================
//=============================================================================================================================
// Update()
//=============================================================================================================================
//=============================================================================================================================
//=============================================================================================================================

float tempZ = 0.0f;

XMMATRIX viewMatrix;

// Unity GameObject를 회전하거나 위치변환을 할 때, 아래처럼 하면 될 듯.
// 왜냐하면 GameObject transform 데이터안에 아래처럼 local 변환 matrix들을 가지고 있으므로..
// Source: https://gamedev.stackexchange.com/questions/152654/moving-camera-in-3d-directx12
void KjApp::UpdateCamera() {

	// Get Datas
	XMMATRIX viewMatrix = XMLoadFloat4x4(&cameraViewMat);

	//Move Up and bottom
	if (GetAsyncKeyState('Z')) {
		viewMatrix *= XMMatrixTranslation(0.0f, -0.0001f, 0.0f);
	}
	else if (GetAsyncKeyState('S')) {
		viewMatrix *= XMMatrixTranslation(0.0f, 0.0001f, 0.0f);
	}

	//Move Right and Left
	if (GetAsyncKeyState('Q')) {
		viewMatrix *= XMMatrixTranslation(0.0001f, 0.0f, 0.0f);
	}
	else if (GetAsyncKeyState('D')) {
		viewMatrix *= XMMatrixTranslation(-0.0001f, 0.0f, 0.0f);
	}

	//Move forward/Backward
	if (GetAsyncKeyState('P')) {
		viewMatrix *= XMMatrixTranslation(0.0f, 0.0f, -0.0001f);
	}
	else if (GetAsyncKeyState('M')) {
		viewMatrix *= XMMatrixTranslation(0.0f, 0.0f, 0.0001f);
	}

	//Rotate Y axis
	if (GetAsyncKeyState('A')) {
		viewMatrix *= XMMatrixRotationY(0.0001f);
	}
	else if (GetAsyncKeyState('E')) {
		viewMatrix *= XMMatrixRotationY(-0.0001f);
	}

	//Rotate X axis
	if (GetAsyncKeyState('R')) {
		viewMatrix *= XMMatrixRotationX(0.0001f);
	}
	else if (GetAsyncKeyState('T')) {
		viewMatrix *= XMMatrixRotationX(-0.0001f);
	}

	//Rotate Z axis
	if (GetAsyncKeyState('F')) {
		viewMatrix *= XMMatrixRotationZ(0.0001f);
	}
	else if (GetAsyncKeyState('G')) {
		viewMatrix *= XMMatrixRotationZ(-0.0001f);
	}

	//Update View Matrix of Camera
	XMStoreFloat4x4(&cameraViewMat, viewMatrix);
}

void KjApp::UpdateCameraOriginal() {
	// build projection and view matrix
	XMMATRIX tmpMat = XMMatrixPerspectiveFovLH(45.0f*(3.14f / 180.0f), (float)Width / (float)Height, 0.1f, 1000.0f);
	XMStoreFloat4x4(&cameraProjMat, tmpMat);

	// build view matrix
	XMVECTOR cPos = XMLoadFloat4(&cameraPosition);
	XMVECTOR cTarg = XMLoadFloat4(&cameraTarget);
	XMVECTOR cUp = XMLoadFloat4(&cameraUp);
	tmpMat = XMMatrixLookAtLH(cPos, cTarg, cUp);
	XMStoreFloat4x4(&cameraViewMat, tmpMat);
	viewMatrix = tmpMat;
}

void KjApp::UpdateCameraNewVer0001() {
	// build projection and view matrix
	XMMATRIX tmpMat = XMMatrixPerspectiveFovLH(45.0f*(3.14f / 180.0f), (float)Width / (float)Height, 0.1f, 1000.0f);
	XMStoreFloat4x4(&cameraProjMat, tmpMat);

	// (chai:20180706)
	XMMATRIX uiCameraTmpMat = XMMatrixPerspectiveFovLH(45.0f*(3.14f / 180.0f), (float)Width / (float)Height, 0.001f, 1000.0f);
	XMStoreFloat4x4(&uiCameraProjMat, uiCameraTmpMat);

	//================================================================================
	// 1. Camera Rotation
	//================================================================================

	// changes curCameraDegreeOnXYPlane from degree to radian
	float curCameraDegreeOnXYPlaneRadian = curCameraDegreeOnXYPlane * XM_PI / 180.0f;

	// get radius of smaller circle for XZ Plane (the x value after rotating across the XY Plane)
	float curRadiusOfCircleOnXZPlane = (1.0f)*cos(curCameraDegreeOnXYPlaneRadian) - (0.0f)*sin(curCameraDegreeOnXYPlaneRadian);
	// get camera Y position after rotating
	float curCameraPostionY = (1.0f)*sin(curCameraDegreeOnXYPlaneRadian) + (0.0f)*cos(curCameraDegreeOnXYPlaneRadian);

	// changes curCameraDegreeOnXZPlane from degree to radian
	float curCameraDegreeOnXZPlaneRadian = curCameraDegreeOnXZPlane * XM_PI / 180.0f;

	// get camera X and Z position after rotating around smaller circle
	float curCameraPostionX = (curRadiusOfCircleOnXZPlane)*cos(curCameraDegreeOnXZPlaneRadian) - (0.0f)*sin(curCameraDegreeOnXZPlaneRadian);
	float curCameraPostionZ = (curRadiusOfCircleOnXZPlane)*sin(curCameraDegreeOnXZPlaneRadian) + (0.0f)*cos(curCameraDegreeOnXZPlaneRadian);

	//================================================================================
	// 2. Camera Panning
	//================================================================================

	// get camera target position after panning
	float curCameraTargetRadiusOfCircleOnXZPlane = (0.0f)*cos(curCameraDegreeOnXYPlaneRadian) - (0.0f + yOnCameraPanningPlane)*sin(curCameraDegreeOnXYPlaneRadian);
	float curCameraTargetPostionY = (0.0f)*sin(curCameraDegreeOnXYPlaneRadian) + (0.0f + yOnCameraPanningPlane)*cos(curCameraDegreeOnXYPlaneRadian);

	float curCameraTargetPostionX = (curCameraTargetRadiusOfCircleOnXZPlane)*cos(curCameraDegreeOnXZPlaneRadian) - (0.0f + xOnCameraPanningPlane)*sin(curCameraDegreeOnXZPlaneRadian);
	float curCameraTargetPostionZ = (curCameraTargetRadiusOfCircleOnXZPlane)*sin(curCameraDegreeOnXZPlaneRadian) + (0.0f + xOnCameraPanningPlane)*cos(curCameraDegreeOnXZPlaneRadian);

	//================================================================================
	// 3. Camera Position
	//================================================================================

	// set camera position so it is relative to camera target
	curCameraPostionX = curCameraPostionX * curCameraDistanceFromViewObject + cameraTarget.x;
	curCameraPostionY = curCameraPostionY * curCameraDistanceFromViewObject + cameraTarget.y;
	curCameraPostionZ = curCameraPostionZ * curCameraDistanceFromViewObject + cameraTarget.z;

	// set the camera target position 
	cameraTarget.x = cameraTarget.x + curCameraTargetPostionX; // *curCameraDistanceFromViewObject;
	cameraTarget.y = cameraTarget.y + curCameraTargetPostionY; // *curCameraDistanceFromViewObject;
	cameraTarget.z = cameraTarget.z + curCameraTargetPostionZ; // *curCameraDistanceFromViewObject;
															   // reset panning values
	xOnCameraPanningPlane = 0.0f;
	yOnCameraPanningPlane = 0.0f;

	cameraPosition = XMFLOAT4(curCameraPostionX, curCameraPostionY, curCameraPostionZ, cameraPosition.w); // camera position
																										  // build view matrix
	XMVECTOR cPos = XMLoadFloat4(&cameraPosition);
	XMVECTOR cTarg = XMLoadFloat4(&cameraTarget);
	XMVECTOR cUp = XMLoadFloat4(&cameraUp);
	tmpMat = XMMatrixLookAtLH(cPos, cTarg, cUp);
	XMStoreFloat4x4(&cameraViewMat, tmpMat);
	viewMatrix = tmpMat;
}

void KjApp::UpdateCameraByMovingCameraAndCameraTarget() {
	/* Source code removed for privacy purposes */
}

void KjApp::Update(double delta) {
}

void KjApp::UpdatePipeline(int objIndex)
{
	HRESULT hr;

	// We have to wait for the gpu to finish with the command allocator before we reset it
	WaitForPreviousFrame();

	// we can only reset an allocator once the gpu is done with it
	// resetting an allocator frees the memory that the command list was stored in
	hr = commandAllocator[frameIndex]->Reset();
	if (FAILED(hr))
	{
		Running = false;
	}

	// reset the command list. by resetting the command list we are putting it into
	// a recording state so we can start recording commands into the command allocator.
	// the command allocator that we reference here may have multiple command lists
	// associated with it, but only one can be recording at any time. Make sure
	// that any other command lists associated to this command allocator are in
	// the closed state (not recording).
	// Here you will pass an initial pipeline state object as the second parameter,
	// but in this tutorial we are only clearing the rtv, and do not actually need
	// anything but an initial default pipeline, which is what we get by setting
	// the second parameter to NULL
	hr = commandList->Reset(commandAllocator[frameIndex], pipelineStateObject);
	if (FAILED(hr))
	{
		Running = false;
	}

	//=================================================================================
	 
	// here we start recording commands into the commandList (which all the commands will be stored in the commandAllocator)

	// transition the "frameIndex" render target from the present state to the render target state so the command list draws to it starting from here
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// here we again get the handle to our current render target view so we can set it as the render target in the output merger stage of the pipeline
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), frameIndex, rtvDescriptorSize);

	// Clear the render target by using the ClearRenderTargetView command
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	// get a handle to the depth/stencil buffer
	//CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(((Mesh *)(objModels[0]->components[2]))->dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	// set the render target for the output merger stage (the output of the pipeline)
	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	// clear the depth/stencil buffer
	commandList->ClearDepthStencilView(dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// set root signature
	commandList->SetGraphicsRootSignature(rootSignature); // set the root signature

														  // set the descriptor heap
														  // store vertex buffer in upload heap
	

	// transition the vertex buffer data from copy destination state to vertex buffer state
	//commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(vertexBuffers[2], D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
	//----------------------------------------------------------------------------

	commandList->RSSetViewports(1, &viewport); // set the viewports
	commandList->RSSetScissorRects(1, &scissorRect); // set the scissor rects

	
	//for (int i = 0; i < 2; i++) {
	for (objIndex = 0; objIndex < numObjModels; objIndex++) {
		commandList->SetGraphicsRootConstantBufferView(0, constantBufferUploadHeaps[frameIndex]->GetGPUVirtualAddress() + ConstantBufferPerObjectAlignedSize * objIndex);
		objModels[objIndex]->UpdatePipeline();
	}

	//----------------------------------------------------------------------------
	//----------------------------------------------------------------------------
	//----------------------------------------------------------------------------
	// void KjApp::Update(double delta)
	//----------------------------------------------------------------------------
	//----------------------------------------------------------------------------
	//----------------------------------------------------------------------------

	float prevCameraX = cameraPosition.x;
	float prevCameraY = cameraPosition.y;
	float prevCameraZ = cameraPosition.z;

	float prevCameraTargetX = cameraTarget.x;
	float prevCameraTargetY = cameraTarget.y;
	float prevCameraTargetZ = cameraTarget.z;

	// update camera
	UpdateCameraNewVer0001(); // mission: 꼭 원점이 아니더라도 target을 유지하면서 그 둘레를 camera가 움직일 수 있게 바꿀 것.

	// create translation matrix for grid from grid's position vector
	XMMATRIX cameraTranslationMat = XMMatrixTranslationFromVector(XMLoadFloat4(&cameraPosition));

	// create grid's world matrix by first rotating the cube, then positioning the rotated cube
	//XMMATRIX cameraPosWorldMat = cameraTranslationMat * tmpMat;
	XMMATRIX cameraPosWorldMat = cameraTranslationMat * viewMatrix;

	//	XMMATRIX cameraPosWorldMat = cameraTranslationMat;

	XMMATRIX viewMat = XMLoadFloat4x4(&cameraViewMat); // load view matrix
	XMMATRIX projMat = XMLoadFloat4x4(&cameraProjMat); // load projection matrix

	//----------------------------------------------------------------------------

	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------
	// now do grid's world matrix
	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------

	for (int i = 0; i < numObjModels; i++) {
		objModels[i]->Update();
		XMMATRIX wvpMat = ((Transform *)objModels[i]->getComponent("Transform"))->getWvpMat(viewMat, projMat); // create wvp matrix

		// store wvpMat into constant buffer
		XMMATRIX transposed = XMMatrixTranspose(wvpMat); // must transpose wvp matrix for the gpu
		XMStoreFloat4x4(&cbPerObject.wvpMat, transposed); // store transposed wvp matrix in constant buffer

		XMFLOAT4 bonePos[256];
		XMFLOAT4 boneRot[256];
		if (i == 4) { // only use bonePos and boneRot when the current object is bob (md5mesh)
			Animator *bobAnimator = ((Animator *)objModels[i]->getComponent("Animator"));
			Avatar *bobAvatar = bobAnimator->avatar;

			for (int j = 0; j < bobAvatar->numJoints; j++) {
				Bone *curBone = bobAvatar->bones[j];

				cbPerObject.bonePos[j] = XMFLOAT3toXMFLOAT4(curBone->pos);
				cbPerObject.boneRot[j] = curBone->rot;
			}
		}

		memcpy(cbvGPUAddress[frameIndex] + ConstantBufferPerObjectAlignedSize * i, &cbPerObject, sizeof(cbPerObject));
	}

	bool changed[6] = { 0 };
	float tempArray[6];

	if (EQUALS_FLOAT(prevCameraX, cameraPosition.x)) {
		tempArray[0] = cameraPosition.x;
		changed[0] = true;
	}
	if (EQUALS_FLOAT(prevCameraY, cameraPosition.y)) {
		tempArray[1] = cameraPosition.y;
		changed[1] = true;
	}
	if (EQUALS_FLOAT(prevCameraZ, cameraPosition.z)) {
		tempArray[2] = cameraPosition.z;
		changed[2] = true;
	}
	if (EQUALS_FLOAT(prevCameraTargetX, cameraTarget.x)) {
		tempArray[3] = cameraTarget.x;
		changed[3] = true;
	}
	if (EQUALS_FLOAT(prevCameraTargetY, cameraTarget.y)) {
		tempArray[4] = cameraTarget.y;
		changed[4] = true;
	}
	if (EQUALS_FLOAT(prevCameraTargetZ, cameraTarget.z)) {
		tempArray[5] = cameraTarget.z;
		changed[5] = true;
	}

	HWND windowArray[6];
	windowArray[0] = cameraWindowXPos;
	windowArray[1] = cameraWindowYPos;
	windowArray[2] = cameraWindowZPos;
	windowArray[3] = cameraWindowXTarget;
	windowArray[4] = cameraWindowYTarget;
	windowArray[5] = cameraWindowZTarget;

	//char text[256];
	//sprintf(text, "%f", tempArray[0]);
	//outputTextBox(cameraWindowXPos, "0");

	//outputTextBox(cameraWindowXPos, text);

	for (int i = 0; i < 6; i++) {
		if (changed[i]) {
			char text[256];
			sprintf(text, "%f", tempArray[i]);
			int len = GetWindowTextLength(windowArray[i]) + 1;
			std::vector<TCHAR> temp(len + strlen(text) + 1);

			WCHAR textInLP[256];
			int result = MultiByteToWideChar(CP_OEMCP, 0, text, -1, textInLP, strlen(text) + 1);

			_tcscat(temp.data(), textInLP);
			SetWindowText(windowArray[i], temp.data());
		}
	}
	//----------------------------------------------------------------------------
	//----------------------------------------------------------------------------
	//----------------------------------------------------------------------------
	// end of KjApp::Update(double delta)
	//----------------------------------------------------------------------------
	//----------------------------------------------------------------------------
	//----------------------------------------------------------------------------


	//-------------------------------------------------------------------------
	// On Screen Text
	//-------------------------------------------------------------------------
	//msgBoxCstring("On Screen Text Rendrer Started");

	OnScreenText *onScreenText = (OnScreenText *)(objModels[5]->getComponent("OnScreenText"));

	Font font = onScreenText->arialFont;
	std::wstring text = std::wstring(L"FPS: ") + std::to_wstring(gd3dApp->timer.fps);
	//std::wstring text = std::wstring(L"Hi");
	XMFLOAT2 pos = XMFLOAT2(0.02f, 0.01f);
	XMFLOAT2 scale = XMFLOAT2(2.0f, 2.0f);
	XMFLOAT2 padding = XMFLOAT2(0.5f, 0.0f);
	XMFLOAT4 color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	// clear the depth buffer so we can draw over everything
	commandList->ClearDepthStencilView(dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// set the text pipeline state object
	commandList->SetPipelineState(textPSO);

	// this way we only need 4 vertices per quad rather than 6 if we were to use a triangle list topology
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// set the text vertex buffer
	commandList->IASetVertexBuffers(0, 1, &onScreenText->textVertexBufferView[frameIndex]);

	// bind the text srv. We will assume the correct descriptor heap and table are currently bound and set
	commandList->SetGraphicsRootDescriptorTable(1, font.srvHandle);

	int numCharacters = 0;

	float topLeftScreenX = (pos.x * 2.0f) - 1.0f;
	float topLeftScreenY = ((1.0f - pos.y) * 2.0f) - 1.0f;

	float x = topLeftScreenX;
	float y = topLeftScreenY;

	float horrizontalPadding = (font.leftpadding + font.rightpadding) * padding.x;
	float verticalPadding = (font.toppadding + font.bottompadding) * padding.y;

	// cast the gpu virtual address to a textvertex, so we can directly store our vertices there
	TextVertex* textVert = (TextVertex*)textVBGPUAddress[frameIndex];
	//TextVertex* textVert = new TextVertex(1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);

	wchar_t lastChar = -1; // no last character to start with

	//msgBoxCstring("hello1");
	
	for (int i = 0; i < text.size(); ++i)
	{
		wchar_t c = text[i];

		FontChar* fc = font.GetChar(c);

		// character not in font char set
		if (fc == nullptr)
			continue;

		// end of string
		if (c == L'\0')
			break;

		// new line
		if (c == L'\n')
		{
			x = topLeftScreenX;
			y -= (font.lineHeight + verticalPadding) * scale.y;
			continue;
		}

		// don't overflow the buffer. In your app if this is true, you can implement a resize of your text vertex buffer
		if (numCharacters >= onScreenText->maxNumTextCharacters)
			break;

		float kerning = 0.0f;
		if (i > 0)
			kerning = font.GetKerning(lastChar, c);

		textVert[numCharacters] = TextVertex(color.x,
			color.y,
			color.z,
			color.w,
			fc->u,
			fc->v,
			fc->twidth,
			fc->theight,
			x + ((fc->xoffset + kerning) * scale.x),
			y - (fc->yoffset * scale.y),
			fc->width * scale.x,
			fc->height * scale.y);

		numCharacters++;

		// remove horrizontal padding and advance to next char position
		x += (fc->xadvance - horrizontalPadding) * scale.x;

		lastChar = c;
	}

	// we are going to have 4 vertices per character (trianglestrip to make quad), and each instance is one character
	commandList->DrawInstanced(4, numCharacters, 0, 0);

	//msgBoxCstring("On Screen Text Rendrer Done");
	//-------------------------------------------------------------------------
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	hr = commandList->Close();
	if (FAILED(hr))
	{
		Running = false;
	}

}

void KjApp::Render()
{
	HRESULT hr;

	//for (int i = 0; i < numObjModels; i++) {
	UpdatePipeline(0); // update the pipeline by sending commands to the commandqueue
					   // create an array of command lists (only one command list here)
	ID3D12CommandList* ppCommandLists[] = { commandList };

	// execute the array of command lists
	commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// this command goes in at the end of our command queue. we will know when our command queue 
	// has finished because the fence value will be set to "fenceValue" from the GPU since the command
	// queue is being executed on the GPU
	hr = commandQueue->Signal(fence[frameIndex], fenceValue[frameIndex]);
	if (FAILED(hr))
	{
		Running = false;
	}

	//}

	// present the current backbuffer
	hr = swapChain->Present(0, 0);
	if (FAILED(hr))
	{
		Running = false;
	}
}
