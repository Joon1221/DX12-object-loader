//***************************************************************************************
// d3dApp.h by Frank Luna (C) 2011 All Rights Reserved.
//
// Simple Direct3D demo application class.  
// Make sure you link: d3d11.lib d3dx11d.lib D3DCompiler.lib D3DX11EffectsD.lib 
//                     dxerr.lib dxgi.lib dxguid.lib.
// Link d3dx11.lib and D3DX11Effects.lib for release mode builds instead
//   of d3dx11d.lib and D3DX11EffectsD.lib.
//***************************************************************************************

#ifndef D3DAPP_H
#define D3DAPP_H

#include "stdafx.h"
#include "game_object.h"
#include "util.h"

class D3DApp
{
public:
	D3DApp(HINSTANCE hInstance);
	virtual ~D3DApp();

	HINSTANCE AppInst()const;
	HWND      MainWnd()const;
	float     AspectRatio()const;

	int Run();

	// Framework methods.  Derived client class overrides these methods to 
	// implement specific application requirements.

	virtual bool Init();
	virtual void OnResize();
	virtual void UpdateScene(float dt) = 0;
	virtual void DrawScene() = 0;
	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	// Convenience overrides for handling mouse/keyboard input.
	virtual void OnCreate(HWND hwnd) { }
	virtual void OnCommand(WPARAM wParam) { }

	virtual void OnMouseDown(WPARAM btnState, int x, int y) { }
	virtual void OnMouseUp(WPARAM btnState, int x, int y) { }
	virtual void OnMouseMove(WPARAM btnState, int x, int y) { }
	virtual void OnMouseWheel(WPARAM wParam) { } // by chai
	virtual void OnKeyDown(WPARAM keyState) { }  // by chai

protected:
	bool InitMainWindow();

	//-------------------------------------------------------------------------
	// DX11
	//-------------------------------------------------------------------------
	//bool InitDirect3D();

	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------
	// DX12
	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------
	bool InitD3D(); // initializes direct3d 12
	void CreateCommandQueue();				// chai: dx12
	void CreateCommandAllocator();			// chai: dx12
	void CreateSwapChain();					// chai: dx12
	void CreateRtvAndDsvDescriptorHeaps();	// chai: dx12
	void CreateFenceAndFenceEvent();		// chai: dx12

	void FlushCommandQueue();				// chai: dx12

	void CalculateFrameStats();

	virtual void Cleanup(); // release com ojects and clean up memory
	void WaitForPreviousFrame(); // wait until gpu is finished with command list

	DXGI_FORMAT GetDXGIFormatFromWICFormat(WICPixelFormatGUID& wicFormatGUID);
	WICPixelFormatGUID GetConvertToWICFormat(WICPixelFormatGUID& wicFormatGUID);
	int GetDXGIFormatBitsPerPixel(DXGI_FORMAT& dxgiFormat);
	
	void outputConsoleXMFLOAT4X4(string msg, XMFLOAT4X4 &m);
	void outputConsoleXMMATRIX(string msg, XMMATRIX &m);
	void outputConsoleXMFLOAT4(string msg, XMFLOAT4 &tempXMFLOAT4);
	void outputConsoleXMVECTOR_4(string msg, XMVECTOR &v);

public:
	WNDPROC oldEditProc;
	void outputConsole(char *text);

	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------
	// Windows General
	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------
	HINSTANCE mhAppInst;
	HWND      mhMainWnd;
	bool      mAppPaused;
	bool      mMinimized;
	bool      mMaximized;
	bool      mResizing;
	UINT      m4xMsaaQuality;

	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------
	// DX12
	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------
	static const int frameBufferCount = 3; // number of buffers we want, 2 for double buffering, 3 for tripple buffering

	IDXGIFactory4* dxgiFactory;

	ID3D12Device* device; // direct3d device
	IDXGISwapChain3* swapChain; // swapchain used to switch between render targets
	DXGI_SAMPLE_DESC sampleDesc;
	ID3D12CommandQueue* commandQueue; // container for command lists
	ID3D12DescriptorHeap* rtvDescriptorHeap; // a descriptor heap to hold resources like the render targets
	ID3D12Resource* renderTargets[frameBufferCount]; // number of render targets equal to buffer count
	ID3D12CommandAllocator* commandAllocator[frameBufferCount]; // we want enough allocators for each buffer * number of threads (we only have one thread)
	ID3D12GraphicsCommandList* commandList; // a command list we can record commands into, then execute them to render the frame
	ID3D12Fence* fence[frameBufferCount];    // an object that is locked while our command list is being executed by the gpu. We need as many 
											 //as we have allocators (more if we want to know when the gpu is finished with an asset)
	HANDLE fenceEvent; // a handle to an event when our fence is unlocked by the gpu
	UINT64 fenceValue[frameBufferCount]; // this value is incremented each frame. each fence will have its own value

	int frameIndex; // current rtv we are on

	int rtvDescriptorSize; // size of the rtv descriptor on the device (all front and back buffers will be the same size)
						   // function declarations

	int LoadImageDataFromFile(BYTE** imageData, D3D12_RESOURCE_DESC& resourceDescription, LPCWSTR filename, int &bytesPerRow);

	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------
	// On Screen Text
	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------
	struct Timer
	{
		double timerFrequency = 0.0;
		long long lastFrameTime = 0;
		long long lastSecond = 0;
		double frameDelta = 0;
		int fps = 0;

		Timer()
		{
			LARGE_INTEGER li;
			QueryPerformanceFrequency(&li);

			// seconds
			//timerFrequency = double(li.QuadPart);

			// milliseconds
			timerFrequency = double(li.QuadPart) / 1000.0;

			// microseconds
			//timerFrequency = double(li.QuadPart) / 1000000.0;

			QueryPerformanceCounter(&li);
			lastFrameTime = li.QuadPart;
		}

		// Call this once per frame
		double GetFrameDelta()
		{
			LARGE_INTEGER li;
			QueryPerformanceCounter(&li);
			frameDelta = double(li.QuadPart - lastFrameTime) / timerFrequency;
			if (frameDelta > 0)
				fps = 1000 / frameDelta;
			lastFrameTime = li.QuadPart;
			return frameDelta;
		}
	};

	// create an instance of timer
	Timer timer;


	//-------------------------------------------------------------------------
	//-----------------------------------------------------------------------------
	// (kj::main.cpp) Global Variables: General
	//-----------------------------------------------------------------------------
	//-------------------------------------------------------------------------

	// Handle to the window
	HWND hwnds[4];
	HWND button = NULL;
	HWND console = NULL;

	HWND cameraWindowXPos = NULL;
	HWND cameraWindowXTarget = NULL;
	HWND cameraWindowYPos = NULL;
	HWND cameraWindowYTarget = NULL;
	HWND cameraWindowZPos = NULL;
	HWND cameraWindowZTarget = NULL;
	HWND objFilePath = NULL;
	HWND objFilePathButton = NULL;

	// name of the window (not the title)
	LPCTSTR WindowName = L"Object Loader Application";

	// title of the window
	LPCTSTR WindowTitle = L"Object Loader";

	// width and height of the window
	int Width = 800;
	int Height = 600;

	// is window full screen?
	bool fullscreen = false;

	// we will exit the program when this becomes false
	bool Running = true;

	// position of the different windows
	int POS_WINDOW[4][2] = {
		{ 0, 0 }, // Main Screen
		{ 0, 0 }, // DirectX
		{ 784, 0 }, // Tool
		{ 0, 799 } // Console
	};

	int SIZE_WINDOW[4][2] = {
		{ 1000, 1040 }, // Main Screen
		{ 785, 800 }, // DirectX
		{ 200, 800 }, // Tool
		{ 990, 180 } // Console
	};

	//-------------------------------------------------------------------------
	// DX11
	//-------------------------------------------------------------------------
	//GameTimer mTimer;

	//ID3D11Device* md3dDevice;
	//ID3D11DeviceContext* md3dImmediateContext;
	//IDXGISwapChain* mSwapChain;
	//ID3D11Texture2D* mDepthStencilBuffer;
	//ID3D11RenderTargetView* mRenderTargetView;
	//ID3D11DepthStencilView* mDepthStencilView;
	//D3D11_VIEWPORT mScreenViewport;

	//-------------------------------------------------------------------------
	// Windows General
	//-------------------------------------------------------------------------
	// Derived class should set these in derived constructor to customize starting values.
	std::wstring mMainWndCaption;
	//D3D_DRIVER_TYPE md3dDriverType;
	int mClientWidth;
	int mClientHeight;
	bool mEnable4xMsaa;
};

#endif // D3DAPP_H