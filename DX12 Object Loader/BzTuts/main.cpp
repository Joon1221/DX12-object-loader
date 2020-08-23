//#define MOVE_BONE_TO_MOUSE_LEFT_CLICK_CODE_TURN_ON

#include "stdafx.h"

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

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Global Variables: General
//-----------------------------------------------------------------------------

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

// name of the window (not the title)
LPCTSTR WindowName = L"BzTutsApp";

// title of the window
LPCTSTR WindowTitle = L"Bz Window";

// width and height of the window
int Width = 800;
int Height = 600;

// is window full screen?
bool FullScreen = false;

// we will exit the program when this becomes false
bool Running = true;

// create a window
bool InitializeWindow(HINSTANCE hInstance,
	int ShowWnd,
	bool fullscreen);

// main application loop
void mainloop();

// callback function for windows messages
LRESULT CALLBACK WndProc(HWND hWnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam);

// direct3d stuff
const int frameBufferCount = 3; // number of buffers we want, 2 for double buffering, 3 for tripple buffering

ID3D12Device* device; // direct3d device

IDXGISwapChain3* swapChain; // swapchain used to switch between render targets

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

bool InitD3D(); // initializes direct3d 12

void Update(); // update the game logic

void UpdatePipeline(int objIndex); // update the direct3d pipeline (update command lists)

void Render(); // execute the command list

void Cleanup(); // release com ojects and clean up memory

void WaitForPreviousFrame(); // wait until gpu is finished with command list

ID3D12PipelineState* pipelineStateObject; // pso containing a pipeline state

ID3D12RootSignature* rootSignature; // root signature defines data shaders will access

D3D12_VIEWPORT viewport; // area that output from rasterizer will be stretched to.

D3D12_RECT scissorRect; // the area to draw in. pixels outside that area will not be drawn onto

vector <ID3D12Resource*> vertexBuffers; // a default buffer in GPU memory that we will load vertex data for our triangle into
vector <ID3D12Resource*> indexBuffers; // a default buffer in GPU memory that we will load index data for our triangle into

//ID3D12Resource* vertexBuffers; // a default buffer in GPU memory that we will load vertex data for our triangle into
//ID3D12Resource* indexBuffer; // a default buffer in GPU memory that we will load index data for our triangle into

vector <D3D12_VERTEX_BUFFER_VIEW> vertexBufferViews; // a structure containing a pointer to the vertex data in gpu memory
vector <D3D12_INDEX_BUFFER_VIEW> indexBufferViews; // a structure holding information about the index buffer

//D3D12_VERTEX_BUFFER_VIEW vertexBufferView; // a structure containing a pointer to the vertex data in gpu memory
//D3D12_INDEX_BUFFER_VIEW indexBufferView; // a structure holding information about the index buffer

ID3D12Resource* depthStencilBuffer; // This is the memory for our depth buffer. it will also be used for a stencil buffer in a later tutorial
//ID3D12DescriptorHeap* dsDescriptorHeap; // This is a heap for our depth/stencil buffer descriptor
vector <ID3D12DescriptorHeap*> dsDescriptorHeaps; // a default buffer in GPU memory that we will load index data for our triangle into

										// this is the structure of our constant buffer.
struct ConstantBufferPerObject {
	XMFLOAT4X4 wvpMat;
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

//XMFLOAT4X4 gridWorldMat; // grid's world matrix (transformation matrix)
//XMFLOAT4X4 gridRotMat; // this will keep track of our rotation for the grid
//XMFLOAT4 gridPosition; // grid's position in space

//XMFLOAT4X4 cube1WorldMat; // our first cubes world matrix (transformation matrix)
//XMFLOAT4X4 cube1RotMat; // this will keep track of our rotation for the first cube
//XMFLOAT4 cube1Position; // our first cubes position in space

XMFLOAT4X4 cube2WorldMat; // our first cubes world matrix (transformation matrix)
XMFLOAT4X4 cube2RotMat; // this will keep track of our rotation for the second cube
XMFLOAT4 cube2PositionOffset; // our second cube will rotate around the first cube, so this is the position offset from the first cube

vector<int> numObjIndices; // the number of indices to draw the cube

ID3D12Resource* textureBuffer; // the resource heap containing our texture
ID3D12Resource* textureBuffer1; // the resource heap containing our texture

int LoadImageDataFromFile(BYTE** imageData, D3D12_RESOURCE_DESC& resourceDescription, LPCWSTR filename, int &bytesPerRow);

DXGI_FORMAT GetDXGIFormatFromWICFormat(WICPixelFormatGUID& wicFormatGUID);
WICPixelFormatGUID GetConvertToWICFormat(WICPixelFormatGUID& wicFormatGUID);
int GetDXGIFormatBitsPerPixel(DXGI_FORMAT& dxgiFormat);

vector<ID3D12DescriptorHeap*> mainDescriptorHeaps;
//ID3D12DescriptorHeap* mainDescriptorHeap;

ID3D12Resource* textureBufferUploadHeap;

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

#include "obj_model.h"
#include "util.h"

char g_buffer[280 + 1];
int numObjModels = 0;
vector<ObjModel> objModels;
vector<string> objModelImagePaths;
//ObjModel *objModel2;
//ObjModel *objModel2;

//-----------------------------------------------------------------------------
// for mouse camera: ver0042 by kj
//-----------------------------------------------------------------------------
int frames = 0;

int curCursorPosX = 0;
int curCursorPosY = 0;
int prevCursorPosX = 0;
int prevCursorPosY = 0;

float curCameraDegreeOnXZPlane = 270.0f; // 카메라가 좌표계의 위에서 볼 때(Top View에서는 xz평면으로 보인다.), x가 1.0이고, z가 0.0인 unit vector가 (x:0, z:0)을 원점으로 시계반대방향으로 회전하는 각도.
float curCameraDegreeOnXYPlane = 0.0f;   // 카메라가 좌표계의 카메라의(카메라가 x축과 평행) 왼쪽 옆에서 볼 때(Left View에서는 xy평면으로 보인다.), x가 1.0이고, y가 0.0인 unit vector가 (x:0, y:0)을 원점으로 시계반대방향으로 회전하는 각도.

float xOnCameraPanningPlane = 0.0f; // chai:ver0049: 카메라가 패닝할 때, x축으로 움직이는 범위. 좌는 마이너스, 우는 플러스.
float yOnCameraPanningPlane = 0.0f; // chai:ver0049: 카메라가 패닝할 때, y축으로 움직이는 범위. 하는 마이너스, 상은 플러스.
//float curYOnCameraPanningPlane = 0.0f; // chai:ver0049: 카메라가 패닝할 때, y축으로 움직이는 범위. 하는 마이너스, 상은 플러스.

//float curCameraHeightFromRotationTarget = 2.0f;
//float curCameraRadiusFromRotationTarget = CAMERA_DEFAULT_DISTANCE_FROM_VIEW_OBJECT;
//float curCameraDegreeOnXZPlane = 270.0f; // camera move때 이미 위에서 만들면서 이것을 지움.

//-----------------------------------------------------------------------------
// for mouse camera: from ver0044
//-----------------------------------------------------------------------------
float curCameraDistanceFromViewObject = 10.0f; // ViewObject는 현재 origin. 나중에 바꿀수도 있게.. Blender처럼 3D Cursor나, 특정 object주변을 rotate할 수 있게. 

int objectSelected = -1;

short zDelta = 0;
int initHwndsCount = 0;

TCHAR *getTextBox(HWND textbox) {
	//if (textbox == NULL) {
	//	return L"hi";
	//}

	int len = GetWindowTextLength(textbox) + 1;
	std::vector<TCHAR> temp(len + 1);

	GetWindowText(textbox, temp.data(), temp.size());


	return temp.data();
/*
	TCHAR szWideString[256];
	char szString[256];
	size_t nNumCharConverted;
	wcstombs_s(&nNumCharConverted, szString, 256,
		szWideString, 256);
*/
	//return szString;
}

void setTextBox(HWND textbox, char *text) {

	int len = GetWindowTextLength(textbox) + 1;
	std::vector<TCHAR> temp(len + strlen(text) + 1);

	//GetWindowText(textbox, temp.data(), temp.size());

	WCHAR textInLP[256];
	int result = MultiByteToWideChar(CP_OEMCP, 0, text, -1, textInLP, strlen(text) + 1);

	_tcscat(temp.data(), textInLP);
	SetWindowText(textbox, temp.data());
}

void outputConsole(char *text) {

	int len = GetWindowTextLength(console) + 1;
	std::vector<TCHAR> temp(len + strlen(text) + 1);

	GetWindowText(console, temp.data(), temp.size());

	WCHAR textInLP[256];
	int result = MultiByteToWideChar(CP_OEMCP, 0, text, -1, textInLP, strlen(text) + 1);

	_tcscat(temp.data(), textInLP);
	SetWindowText(console, temp.data());
	//setTextBox(console, text);

	//SendMessageA(console, EM_SETSEL, -1, -1);
	//SendMessage(console, EM_SETSEL, 0, -1);
	//SendMessageA(console, EM_SETSEL, 0, 0);

	//SendMessage(console, EM_SCROLLCARET, -1, -1);
	//SendMessageA(console, EM_SCROLLCARET, 0, 0);
	//SendMessage(console, EM_SCROLLCARET, 0, -1);
	
	//	EM_SCROLLCARET

	SendMessage(console, EM_SETSEL, 0, -1);
	SetFocus(console);
	SendMessage(console, EM_SETSEL, -1, -1);
	SetFocus(console);
	SendMessage(console, EM_SCROLLCARET, -1, -1);
	SetFocus(console);
}

int WINAPI WinMain(HINSTANCE hInstance,    //Main windows function
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nShowCmd) {

	// create grid
	objModels.push_back(ObjModel());
	//objModelImagePaths.push_back("..\\..\\imp_exp\\axis.png");
	objModelImagePaths.push_back("..\\..\\imp_exp\\ash_uvgrid03.jpg");

	// create bone 
	objModels.push_back(ObjModel());
	//objModelImagePaths.push_back("..\\..\\imp_exp\\axis.png");
	objModelImagePaths.push_back("..\\..\\imp_exp\\ash_uvgrid03.jpg");
	//objModelImagePaths.push_back("..\\..\\imp_exp\\bone.png");

	// create small ui axis
	objModels.push_back(ObjModel());
	//objModelImagePaths.push_back("..\\..\\imp_exp\\ash_uvgrid03.jpg");
	objModelImagePaths.push_back("..\\..\\imp_exp\\axis.png");

	////create bone
	//objModels.push_back(ObjModel());
	////objModelImagePaths.push_back("..\\..\\imp_exp\\ash_uvgrid03.jpg");
	//objModelImagePaths.push_back("..\\..\\imp_exp\\bone.png");

	//GetWorkingDir();

	// create small ui axis
	//if (!objModels.at(0).loadObjFile("..\\..\\imp_exp\\monkey.obj")) {
	//if (!objModels.at(0).loadObjFile("..\\..\\imp_exp\\cube.obj")) {
	if (!objModels.at(0).loadObjFile("..\\..\\imp_exp\\grid.obj")) {
		MessageBox(NULL, L"CreateFile", L"Error", MB_OK | MB_ICONERROR);
		return 1;
	}
	//if (!objModels.at(1).loadObjFile("..\\..\\imp_exp\\cube.obj")) {
	if (!objModels.at(1).loadObjFile("..\\..\\imp_exp\\monkey.obj")) {
	//if (!objModels.at(1).loadBoneFile("..\\..\\imp_exp\\bone.txt")) {

		MessageBox(NULL, L"CreateFile", L"Error", MB_OK | MB_ICONERROR);
		return 1;
	}

	if (!objModels.at(2).loadObjFile("..\\..\\imp_exp\\axis.obj")) {
		MessageBox(NULL, L"CreateFile", L"Error", MB_OK | MB_ICONERROR);
		return 1;
	}

	//if (!objModels.at(3).loadBoneFile("..\\..\\imp_exp\\bone.txt")) {
	//	MessageBox(NULL, L"CreateFile", L"Error", MB_OK | MB_ICONERROR);
	//	return 1;
	//}

	// create the window
	if (!InitializeWindow(hInstance, nShowCmd, FullScreen))
	{
		MessageBox(0, L"Window Initialization - Failed",
			L"Error", MB_OK);
		return 1;
	}

	//setTextBox(cameraWindowZTarget, "ghello1");

	// initialize direct3d
	if (!InitD3D())
	{
		MessageBox(0, L"Failed to initialize direct3d 12",
			L"Error", MB_OK);
		Cleanup();
		return 1;
	}

	// start the main loop
	mainloop();

	// we want to wait for the gpu to finish executing the command list before we start releasing everything
	WaitForPreviousFrame();

	// close the fence event
	CloseHandle(fenceEvent);

	// clean up everything
	Cleanup();

	return 0;
}

int POS_WINDOW[4][2] = {
//	{CW_USEDEFAULT, CW_USEDEFAULT},
	{0, 0},
	{0, 0}, // DirectX
	{784, 0}, // Tool
	{0, 799} // Console
};

int SIZE_WINDOW[4][2] = {
	{1000, 1040},
	{785, 800 }, // DirectX
	{200, 800 }, // Tool
	{990, 180 } // Console
};

HINSTANCE ghInstance;
XMMATRIX rayWorldMatrix;

// create and show the window
bool InitializeWindow(HINSTANCE hInstance,
	int ShowWnd,
	bool fullscreen)

{
	ghInstance = hInstance;
	if (fullscreen)
	{
		HMONITOR hmon = MonitorFromWindow(hwnds[0],
			MONITOR_DEFAULTTONEAREST);
		MONITORINFO mi = { sizeof(mi) };
		GetMonitorInfo(hmon, &mi);

		Width = mi.rcMonitor.right - mi.rcMonitor.left;
		Height = mi.rcMonitor.bottom - mi.rcMonitor.top;
	}

	WNDCLASSEX wc;

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = NULL;
	wc.cbWndExtra = NULL;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = WindowName;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, L"Error registering class",
			L"Error", MB_OK | MB_ICONERROR);
		return false;
	}

	for (int i = 0; i < 4; i++) {
		hwnds[i] = CreateWindowEx(NULL,
			WindowName,
			WindowTitle,
			WS_OVERLAPPEDWINDOW,
			//CW_USEDEFAULT, CW_USEDEFAULT,
			POS_WINDOW[i][0], POS_WINDOW[i][1],
			//Width, Height,
			SIZE_WINDOW[i][0], SIZE_WINDOW[i][1],
			NULL,
			NULL,
			hInstance,
			NULL);

		if (!hwnds[i])
		{
			MessageBox(NULL, L"Error creating window",
				L"Error", MB_OK | MB_ICONERROR);
			return false;
		}
	
		if (fullscreen)
		{
			SetWindowLong(hwnds[i], GWL_STYLE, 0);
		}

		ShowWindow(hwnds[i], ShowWnd);
		UpdateWindow(hwnds[i]);
	}
	//setTextBox(cameraWindowZTarget, "ghello1");

	SetParent(hwnds[1], hwnds[0]);
	SetParent(hwnds[2], hwnds[0]);
	SetParent(hwnds[3], hwnds[0]);
	
	//cameraWindowZTarget = NULL;
	//hwnds[3] = (HWND)1;

	//char buf[1024];
	//sprintf(buf, "hwnds[0] = %d", hwnds[0]);
	//msgBoxCstring(buf);
	//sprintf(buf, "hwnds[1] = %d", hwnds[1]);
	//msgBoxCstring(buf);
	//sprintf(buf, "hwnds[2] = %d", hwnds[2]);
	//msgBoxCstring(buf);
	//sprintf(buf, "hwnds[3] = %d", hwnds[3]);
	//msgBoxCstring(buf);
	//sprintf(buf, "cameraWindowZTarget = %d", cameraWindowZTarget);
	//msgBoxCstring(buf);

	return true;
}

void mainloop() {
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	while (Running)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			// run game code
			Update(); // update the game logic
			Render(); // execute the command queue (rendering the scene is the result of the gpu executing the command lists)
		}
	}
}

WNDPROC oldEditProc;

LRESULT CALLBACK subEditProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_RETURN:
			//Do your stuff
			outputConsole("world\r\n");
			break;  //or return 0; if you don't want to pass it further to def proc
					//If not your key, skip to default:
		}
	default:
		return CallWindowProc(oldEditProc, wnd, msg, wParam, lParam);
	}
	return 0;
}

void outputConsoleXMFLOAT4X4(string msg, XMFLOAT4X4 &m) {
	char tempBuf[1000];
	sprintf(tempBuf, "%s\r\n", msg.c_str());
	outputConsole(tempBuf);
	for (int i = 0; i < 4; i++) {
		char tempBuf[1000];
		sprintf(tempBuf, "%8.2f %8.2f %8.2f %8.2f\r\n", m.m[i][0], m.m[i][1], m.m[i][2], m.m[i][3]);
		outputConsole(tempBuf);
	}
}

void outputConsoleXMMATRIX(string msg, XMMATRIX &m) {
	XMFLOAT4X4 tempXMFLOAT4X4;
	XMStoreFloat4x4(&tempXMFLOAT4X4, m);
	outputConsoleXMFLOAT4X4(msg, tempXMFLOAT4X4);
}

void outputConsoleXMFLOAT4(string msg, XMFLOAT4 &tempXMFLOAT4) {
	char tempBuf1[1000];
	sprintf(tempBuf1, "%s: %8.2f %8.2f %8.2f %8.2f\r\n", msg.c_str(), tempXMFLOAT4.x, tempXMFLOAT4.y, tempXMFLOAT4.z, tempXMFLOAT4.w);
	outputConsole(tempBuf1);
}

void outputConsoleXMVECTOR_4(string msg, XMVECTOR &v) {
	XMFLOAT4 tempXMFloat4;
	XMStoreFloat4(&tempXMFloat4, v);
	outputConsoleXMFLOAT4(msg, tempXMFloat4);
}

//================================================================================
//================================================================================
// Source: http://geomalgorithms.com/a06-_intersect-2.html
//     - checks if ray intersects with triangle in 3D space 
//================================================================================
//================================================================================
/*

#define SMALL_NUM   0.00000001 // anything that avoids division overflow
// dot product (3D) which allows vector operations in arguments
#define dot(u,v)   ((u).x * (v).x + (u).y * (v).y + (u).z * (v).z)

// intersect3D_RayTriangle(): find the 3D intersection of a ray with a triangle
//    Input:  a ray R, and a triangle T
//    Output: *I = intersection point (when it exists)
//    Return: -1 = triangle is degenerate (a segment or point)
//             0 =  disjoint (no intersect)
//             1 =  intersect in unique point I1
//             2 =  are in the same plane

int intersect3D_RayTriangle(XMFLOAT3 R, XMFLOAT3 t1, XMFLOAT3 t2, XMFLOAT3 t3, XMFLOAT3 I) {
	XMFLOAT3    u, v, n;              // triangle vectors
	XMFLOAT3    dir, w0, w;           // ray vectors
	float     r, a, b;              // params to calc ray-plane intersect

									// get triangle edge vectors and plane normal
	u = T.V1 - T.V0;
	v = T.V2 - T.V0;
	n = u * v;              // cross product

	dir = R.P1 - R.P0;              // ray direction vector
	w0 = R.P0 - T.V0;
	a = -dot(n, w0);
	b = dot(n, dir);
	if (fabs(b) < SMALL_NUM) {     // ray is  parallel to triangle plane
		if (a == 0)                 // ray lies in triangle plane
			return 2;
		else return 0;              // ray disjoint from plane
	}

	// get intersect point of ray with triangle plane
	r = a / b;
	if (r < 0.0)                    // ray goes away from triangle
		return 0;                   // => no intersect
									// for a segment, also test if (r > 1.0) => no intersect

	*I = R.P0 + r * dir;            // intersect point of ray and plane

									// is I inside T?
	float    uu, uv, vv, wu, wv, D;
	uu = dot(u, u);
	uv = dot(u, v);
	vv = dot(v, v);
	w = *I - T.V0;
	wu = dot(w, u);
	wv = dot(w, v);
	D = uv * uv - uu * vv;

	// get and test parametric coords
	float s, t;
	s = (uv * wv - vv * wu) / D;
	if (s < 0.0 || s > 1.0)         // I is outside T
		return 0;
	t = (uv * wu - uu * wv) / D;
	if (t < 0.0 || (s + t) > 1.0)  // I is outside T
		return 0;

	return 1;                       // I is in T
}

*/

LRESULT CALLBACK WndProc(HWND hwnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam) {
	switch (msg) {
	case WM_CREATE: {

		if (initHwndsCount == 0) {
			HMENU hMenuBar = CreateMenu();
			HMENU hFile = CreateMenu();
			HMENU hOptions = CreateMenu();

			AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hFile, L"File");
			AppendMenu(hMenuBar, MF_POPUP, NULL, L"Edit");
			AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hOptions, L"Camera");

			AppendMenu(hFile, MF_STRING, 1, L"Exit");
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
		}
		else if (initHwndsCount == 3) {
			console = CreateWindow(L"Edit", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_AUTOVSCROLL | ES_WANTRETURN | WS_HSCROLL | WS_VSCROLL | ES_MULTILINE, 0, 0, 974, 145, hwnd, (HMENU)1009, NULL, NULL);
			outputConsole("hello\r\n");
			outputConsole("hello\r\n");
			outputConsole("hello\r\n");
			oldEditProc = (WNDPROC)SetWindowLongPtr(console, GWLP_WNDPROC, (LONG_PTR)subEditProc);
			//HWND hInput = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "",
			//	WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL,
			//	0, 0, 100, 100, hwnd, (HMENU)IDC_MAIN_INPUT, GetModuleHandle(NULL), NULL);
			//
			//oldEditProc = (WNDPROC)SetWindowLongPtr(hInput, GWLP_WNDPROC, (LONG_PTR)subEditProc);
		}
		initHwndsCount++;
	}
		break;
	case WM_COMMAND: {
		switch (LOWORD(wParam)) {
		case 1:
			Running = false;
			DestroyWindow(hwnd);
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
			//setTextBox(cameraWindowXPos, getTextBox(cameraWindowXPos));
			//GetWindowText(textbox, temp.data(), temp.size());


			//TCHAR *szWideString = getTextBox(cameraWindowXPos);
			//char szString[256];
			//size_t nNumCharConverted;
			//wcstombs_s(&nNumCharConverted, szString, 256, szWideString, 256);

			//size_t len = strlen(szString);
			//WCHAR unistring[280 + 1];
			//int result = MultiByteToWideChar(CP_OEMCP, 0, szString, -1, unistring, len + 1);


			//SetWindowText(console, unistring);
			//outputConsole(szString);

			//TCHAR *szWideString = getTextBox(cameraWindowXPos);
			//char szString[256];
			//size_t nNumCharConverted;
			//wcstombs_s(&nNumCharConverted, szString, 256, szWideString, 256);

			//cameraPosition.x = atof(szString);
			//outputConsole(szString);

			//switch (HIWORD(wParam))

			//{

			//	case EN_CHANGE:

			//	{

			//		outputConsole("a");

			//	}

			//}


			break;
		}

		//case 1003:
		//	break;
		//case 1004:
		//	break;
		//case 1005:
		//	break;
		//case 1006:
		//	break;
		//case 1007:
		//	break;
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
		}
	}
	break;

	//-----------------------------------------------------------------------------
	// for mouse camera: ver0042 by kj
	//-----------------------------------------------------------------------------
	case WM_MOUSEMOVE: {
		if (hwnd != hwnds[1]) {
			break;

		}
			curCursorPosX = LOWORD(lParam);
			curCursorPosY = HIWORD(lParam);

			int xPosDifference = curCursorPosX - prevCursorPosX; // positive means moved to the right and negative means moved to the left
			int yPosDifference = curCursorPosY - prevCursorPosY; // positive means moved down and negative means moved up

			prevCursorPosX = curCursorPosX;
			prevCursorPosY = curCursorPosY;

			// chai:ver0049: rotating
			//if (wParam & MK_LBUTTON && frames >= 2) {

			//else {
			//	frames++;
			//}

			// chai:ver0049: panning
			if (wParam & MK_LBUTTON  && wParam & MK_SHIFT) {

				//char buf[1024];
				//sprintf(buf, "%d %d", xPosDifference, yPosDifference);
				//msgBoxCstring(buf);

				// move camera on sphere
				//curCameraDegreeOnXZPlane += (MOUSE_ROTATION_HORIZONTAL_SPEED * xPosDifference / MOUSE_ROTATION_HORIZONTAL_RESOLUTION) * MOUSE_ROTATION_HORIZONTAL_DIRECTION; // left <-> right
				//curCameraDegreeOnXYPlane += (MOUSE_ROTATION_VERTICAL_SPEED * yPosDifference / MOUSE_ROTATION_VERTICAL_RESOLUTION) * MOUSE_ROTATION_VERTICAL_DIRECTION; // up <-> down
				
				//xOnCameraPanningPlane += (MOUSE_PANNING_HORIZONTAL_SPEED * xPosDifference / MOUSE_PANNING_HORIZONTAL_RESOLUTION) * MOUSE_PANNING_HORIZONTAL_DIRECTION;
				
				if (xOnCameraPanningPlane == 0.0f) {
					xOnCameraPanningPlane = ((MOUSE_PANNING_HORIZONTAL_SPEED * xPosDifference / MOUSE_PANNING_HORIZONTAL_RESOLUTION) * MOUSE_PANNING_HORIZONTAL_DIRECTION);
				}
				if (yOnCameraPanningPlane == 0.0f) {
					yOnCameraPanningPlane = ((MOUSE_PANNING_VERTICAL_SPEED * yPosDifference / MOUSE_PANNING_VERTICAL_RESOLUTION) * MOUSE_PANNING_VERTICAL_DIRECTION);
					//if (((MOUSE_PANNING_VERTICAL_SPEED * yPosDifference / MOUSE_PANNING_VERTICAL_RESOLUTION) * MOUSE_PANNING_VERTICAL_DIRECTION) < 0.0f) {
					//	yOnCameraPanningPlane = -0.01f;
					//}
					//else {
					//	yOnCameraPanningPlane = 0.01f;
					//}
				}
			}
			// chai:ver0048: 절대 아래를 위의 if보다 먼저 두면 안 되고, 두번째로, else를 꼭 붙여야 한다.
			else if (wParam & MK_LBUTTON) {
				//frames = 0;

				//char buf[1024];
				//sprintf(buf, "%d %d", xPosDifference, yPosDifference);
				//msgBoxCstring(buf);

				// move camera on sphere
				curCameraDegreeOnXZPlane += (MOUSE_ROTATION_HORIZONTAL_SPEED * xPosDifference / MOUSE_ROTATION_HORIZONTAL_RESOLUTION) * MOUSE_ROTATION_HORIZONTAL_DIRECTION; // left <-> right
				curCameraDegreeOnXYPlane += (MOUSE_ROTATION_VERTICAL_SPEED * yPosDifference / MOUSE_ROTATION_VERTICAL_RESOLUTION) * MOUSE_ROTATION_VERTICAL_DIRECTION; // up <-> down
			}
		}
		break;
	case WM_MOUSEWHEEL: {

			if (hwnd != hwnds[1]) {
				break;
			}
			//if (wParam & MK_SHIFT) {
			zDelta = GET_WHEEL_DELTA_WPARAM(wParam);

			//if (zDelta > 120 || zDelta < -120) {
			//	char buf[1024];
			//	sprintf(buf, "WM_MOUSEHWHEEL: zDelta = %d", zDelta);
			//	msgBoxCstring(buf);
			//}

			if (zDelta < 0) { // zDelta == -120 이거나 zDelta == 120이거나..
				curCameraDistanceFromViewObject += 0.1f * (zDelta / 120.0f) * (zDelta / 120.0f); // scroll down or zoom out
			}
			else {
				if (curCameraDistanceFromViewObject - 0.1f * (zDelta / 120.0f) * (zDelta / 120.0f) >= 0.0f) {
					curCameraDistanceFromViewObject -= 0.1f * (zDelta / 120.0f) * (zDelta / 120.0f); // scroll up or zoom in
				}
			}

			// 이렇게 아래처럼 해도 된다.
			//((short)GET_WHEEL_DELTA_WPARAM(wParam) < 0) ? curCameraDistanceFromViewObject-- : curCameraDistanceFromViewObject++;
		
			//}

			//DWORD x = HIWORD(wParam);
			//	if (x>0)
			//	{
			//		curCameraDistanceFromViewObject--;
			//	}
			//	else {
			//		curCameraDistanceFromViewObject++;
			//	}
		}
		break;
	//case WM_MOUSEHWHEEL: { // horizontal: thumb scroll wheel of logitech MX Master
	//		//if (wParam & MK_SHIFT) {
	//		zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
	//		char buf[1024];
	//		sprintf(buf, "WM_MOUSEHWHEEL: zDelta = %d", zDelta);
	//		msgBoxCstring(buf);

	//		//((short)GET_WHEEL_DELTA_WPARAM(wParam) < 0) ? curCameraDistanceFromViewObject-- : curCameraDistanceFromViewObject++;

	//		//}

	//		//DWORD x = HIWORD(wParam);
	//		//	if (x>0)
	//		//	{
	//		//		curCameraDistanceFromViewObject--;
	//		//	}
	//		//	else {
	//		//		curCameraDistanceFromViewObject++;
	//		//	}
	//	}
	//	break;
	case WM_LBUTTONDOWN: {
		// mouse picking / ray
		// [ KJ_SEASON_TWO_SECOND_PART_SECTION_1:BLENDER_TOOL_AND_3D_API / Work Log: 201808141700 ] 
		// source: http://antongerdelan.net/opengl/raycasting.html
		if (hwnd != hwnds[1]) {
			break;
		}

		curCursorPosX = LOWORD(lParam);
		curCursorPosX += 15;
		curCursorPosY = HIWORD(lParam);
		curCursorPosY += 15;

		char buf[1024];
		//sprintf(buf, "%d %d\r\n", curCursorPosX, curCursorPosY);
		//outputConsole(buf);
		//msgBoxCstring(buf);


		XMMATRIX viewMat = XMLoadFloat4x4(&cameraViewMat); // load view matrix
		XMMATRIX viewMatInversed = XMMatrixInverse(&XMMatrixDeterminant(viewMat), viewMat);

		//outputConsoleXMMATRIX(string("viewMat: "), viewMat);
		//outputConsoleXMMATRIX(string("viewMatInversed: "), viewMatInversed);

		//---------------------------------------------------------------------------
		// 맞는지 보기 위해, 아무 matrix든 viewMat에 곱해서, 다시 viewMatInversed를 곱하면 원본이 나와야 한다.
		//---------------------------------------------------------------------------
		//float TEMP_MATRIX[4][4] = {
		//	{ 00, 01, 02, 03 },
		//	{ 10, 11, 12, 13 },
		//	{ 20, 21, 22, 23 },
		//	{ 30, 31, 32, 33 },
		//};
		//XMFLOAT4X4 temp4x4Matrix;
		//memcpy(temp4x4Matrix.m, TEMP_MATRIX, sizeof(float)*4*4);
		//outputConsoleXMFLOAT4X4(string("temp4x4Matrix: "), temp4x4Matrix);
		//XMMATRIX tempXMMatrix = XMLoadFloat4x4(&temp4x4Matrix);
		//outputConsoleXMMATRIX(string("tempXMMatrix: "), tempXMMatrix);

		////XMMATRIX newTempXMMatrix = XMMatrixMultiply(tempXMMatrix, viewMat);
		//XMMATRIX newTempXMMatrix = tempXMMatrix * viewMat;
		//outputConsoleXMMATRIX(string("newTempXMMatrix: "), newTempXMMatrix);

		////XMMATRIX backTempXMMatrix = XMMatrixMultiply(newTempXMMatrix, viewMatInversed);
		//XMMATRIX backTempXMMatrix = newTempXMMatrix * viewMatInversed;
		//outputConsoleXMMATRIX(string("backTempXMMatrix: "), backTempXMMatrix);

		//exit(1);
		//---------------------------------------------------------------------------


		XMMATRIX projMat = XMLoadFloat4x4(&cameraProjMat); // load projection matrix
		XMMATRIX projMatInversed = XMMatrixInverse(&XMMatrixDeterminant(projMat), projMat);
		//outputConsoleXMMATRIX(string("projMat: "), projMat);
		//outputConsoleXMMATRIX(string("projMatInversed: "), projMatInversed);

		XMMATRIX cameraTranslationMat = XMMatrixTranslationFromVector(XMLoadFloat4(&cameraPosition));
		XMMATRIX cameraTranslationMatInversed = XMMatrixInverse(&XMMatrixDeterminant(cameraTranslationMat), cameraTranslationMat);
		//outputConsoleXMMATRIX(string("cameraTranslationMat: "), cameraTranslationMat);
		//outputConsoleXMMATRIX(string("cameraTranslationMatInversed: "), cameraTranslationMatInversed);

		XMMATRIX cameraPosWorldMat = cameraTranslationMat * viewMat;
		XMMATRIX cameraPosWorldMatInversed = XMMatrixInverse(&XMMatrixDeterminant(cameraPosWorldMat), cameraPosWorldMat);
		//outputConsoleXMMATRIX(string("cameraPosWorldMat: "), cameraPosWorldMat);
		//outputConsoleXMMATRIX(string("cameraPosWorldMatInversed: "), cameraPosWorldMatInversed);

		float screenWidth = SIZE_WINDOW[1][0];
		float screenHeight = SIZE_WINDOW[1][1];

		XMFLOAT4X4 P;
		XMStoreFloat4x4(&P, projMat);
		float rayX = (+2.0f*curCursorPosX / screenWidth - 1.0f) / P(0, 0); // 프로젝션은 어차피 inverse matrix만들어서, 곱하는 과정없이, 어차피 원본 proj matrix (0, 0)위치의 값을 나누면 inverse가 된다. y는 (1, 1)위치로 나누면 된다.
		float rayY = (-2.0f*curCursorPosY / screenHeight + 1.0f) / P(1, 1);
		float rayZ = 1.0f;
		//vec3 ray_nds = vec3(x, y, z);

		// Render화면 child window를 frame에 꽉차게 하고 찍어야
		// 정상적으로 -0.5, -0.5까지나온다. bottom right도 마찬가지.
		// 암튼 proj mat을 적용후, -0.5에서 0.5의 범위가 된다.
		//char buf1[1024];
		//sprintf(buf1, "rayX = %8.2f, rayY = %8.2f\r\n", rayX, rayY);
		//outputConsole(buf1);

		// Ray definition in view space.
		XMVECTOR rayOrigin = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		XMVECTOR rayDirVec = XMVectorSet(rayX, rayY, rayZ, 0.0f);

		rayOrigin = XMVector3TransformCoord(rayOrigin, cameraPosWorldMatInversed);
		rayDirVec = XMVector3TransformNormal(rayDirVec, cameraPosWorldMatInversed);
		//outputConsoleXMFLOAT4(string("cameraPosition: "), cameraPosition);
		//outputConsoleXMVECTOR_4(string("rayOrigin: "), rayOrigin);
		//outputConsoleXMVECTOR_4(string("before normalize rayDir: "), rayDir);

		// Make the ray direction unit length for the intersection tests.
		rayDirVec = XMVector3Normalize(rayDirVec);
		//outputConsoleXMVECTOR_4(string("after normalize rayDir: "), rayDir);

		XMFLOAT4 rayDirFloat4;

		XMStoreFloat4(&rayDirFloat4, rayDirVec);
		//rayDirFloat4.x *= 4;
		//rayDirFloat4.y *= 4;
		//rayDirFloat4.z *= 4;
		//rayDirFloat4.w *= 4;

		//rayDirFloat4.x += cameraPosition.x;
		//rayDirFloat4.y += cameraPosition.y;
		//rayDirFloat4.z += cameraPosition.z;
		//rayDirFloat4.w += cameraPosition.w;

#ifdef MOVE_BONE_TO_MOUSE_LEFT_CLICK_CODE_TURN_ON
		cube1Position.x = rayDirFloat4.x + cameraPosition.x;
		cube1Position.y = rayDirFloat4.y + cameraPosition.y;
		cube1Position.z = rayDirFloat4.z + cameraPosition.z;
		cube1Position.w = rayDirFloat4.w + cameraPosition.w;
#endif
		//outputConsoleXMFLOAT4(string("cube1Position: "), cube1Position);

		// check if ray collides with any object


		//char tempBuf[256];
		//sprintf(tempBuf, "numObjModels: %d\r\n", numObjModels);
		//outputConsole(tempBuf);

		//float shortestDist = 100000.0f;
		//int shortestIndexI = -1;
		//int shortestIndexJ = -1;

		//for (int i = 0; i < numObjModels; i++) {
		//	//sprintf(tempBuf, "i: %d\r\n", i);
		//	//outputConsole(tempBuf);

		//	ObjModel *curObjModel = &objModels.at(i);

		//	//char tempBuf[256];
		//	//sprintf(tempBuf, "curObjMode->triFaces.size(): %d \r\n", curObjModel->triFaces.size());
		//	//outputConsole(tempBuf);

		//	////char tempBuf[256];
		//	//sprintf(tempBuf, "curObjModel->triFaces[0].indices[0]: %d \r\n", curObjModel->triFaces[0].indices[0]);
		//	//outputConsole(tempBuf);

		//	////char tempBuf[256];
		//	//sprintf(tempBuf, "curObjModel->finalVertices.at(0).pos.x: %f \r\n", curObjModel->finalVertices.at(0).pos.x);
		//	//outputConsole(tempBuf);

		//	for (int j = 0; j < curObjModel->triFaces.size(); j++) {

		//		XMFLOAT3 *triVertex1 = &(curObjModel->finalVertices.at(curObjModel->triFaces[j].indices[0]).pos);
		//		XMFLOAT3 *triVertex2 = &(curObjModel->finalVertices.at(curObjModel->triFaces[j].indices[1]).pos);
		//		XMFLOAT3 *triVertex3 = &(curObjModel->finalVertices.at(curObjModel->triFaces[j].indices[2]).pos);

		//		//char tempBuf1[256];
		//		//char tempBuf2[256];
		//		//char tempBuf3[256];
		//		//sprintf(tempBuf1, "TriVertex1: | x = %f , y = %f, z = %f | \r\n", triVertex1->x, triVertex1->y, triVertex1->z);
		//		//sprintf(tempBuf2, "TriVertex2: | x = %f , y = %f, z = %f | \r\n", triVertex2->x, triVertex2->y, triVertex2->z);
		//		//sprintf(tempBuf3, "TriVertex3: | x = %f , y = %f, z = %f | \r\n", triVertex3->x, triVertex3->y, triVertex3->z);
		//		//outputConsole(tempBuf1);
		//		//outputConsole(tempBuf2);
		//		//outputConsole(tempBuf3);

		//		XMFLOAT3 rayDir;
		//		rayDir.x = rayDirFloat4.x;
		//		rayDir.y = rayDirFloat4.y;
		//		rayDir.z = rayDirFloat4.z;

		//		XMFLOAT3 rayPos;
		//		rayPos.x = cameraPosition.x;
		//		rayPos.y = cameraPosition.y;
		//		rayPos.z = cameraPosition.z;

		//		XMFLOAT3 intersectionPoint;


		//		float averageX = (triVertex1->x + triVertex2->x + triVertex3->x) / 3;
		//		float averageY = (triVertex1->y + triVertex2->y + triVertex3->y) / 3;
		//		float averageZ = (triVertex1->z + triVertex2->z + triVertex3->z) / 3;

		//		float curDist = sqrt(pow(rayPos.x - averageX, 2) + pow(rayPos.y - averageY, 2) + pow(rayPos.z - averageZ, 2));

		//		if (intersect3DTriangleWithRay(rayPos, rayDir, *triVertex1, *triVertex2, *triVertex3, &intersectionPoint) && curDist < shortestDist) {
		//			shortestDist = curDist;
		//			shortestIndexI = i;
		//			shortestIndexJ = j;

		//			char tempBuf[256];
		//			sprintf(tempBuf, "Shorter Intersection Point Found At: (%4.2f , %4.2f , %4.2f) \r\n", intersectionPoint.x, intersectionPoint.y, intersectionPoint.z);
		//			//outputConsole(tempBuf);


		//			//curObjModel->finalVertices.at(curObjModel->triFaces[j].indices[0]).texCoord.x = 0;
		//			//curObjModel->finalVertices.at(curObjModel->triFaces[j].indices[0]).texCoord.y = 0;
		//			//curObjModel->finalVertices.at(curObjModel->triFaces[j].indices[1]).texCoord.x = 200;
		//			//curObjModel->finalVertices.at(curObjModel->triFaces[j].indices[1]).texCoord.y = 200;
		//			//curObjModel->finalVertices.at(curObjModel->triFaces[j].indices[2]).texCoord.x = -100;
		//			//curObjModel->finalVertices.at(curObjModel->triFaces[j].indices[2]).texCoord.y = -12;
		//			//outputConsole("triangle intersected\r\n");
		//		}
		//	}
		//}
		//
		//if (shortestIndexI != -1 && shortestIndexJ != -1) {
		//	ObjModel *curObjModel = &objModels.at(shortestIndexI);

		//	XMFLOAT3 *triVertex1 = &(curObjModel->finalVertices.at(curObjModel->triFaces[shortestIndexJ].indices[0]).pos);
		//	XMFLOAT3 *triVertex2 = &(curObjModel->finalVertices.at(curObjModel->triFaces[shortestIndexJ].indices[1]).pos);
		//	XMFLOAT3 *triVertex3 = &(curObjModel->finalVertices.at(curObjModel->triFaces[shortestIndexJ].indices[2]).pos);

		//	//char tempBuf1[256];
		//	//char tempBuf2[256];
		//	//char tempBuf3[256];
		//	//sprintf(tempBuf1, "TriVertex1: | x = %f , y = %f, z = %f | \r\n", triVertex1->x, triVertex1->y, triVertex1->z);
		//	//sprintf(tempBuf2, "TriVertex2: | x = %f , y = %f, z = %f | \r\n", triVertex2->x, triVertex2->y, triVertex2->z);
		//	//sprintf(tempBuf3, "TriVertex3: | x = %f , y = %f, z = %f | \r\n", triVertex3->x, triVertex3->y, triVertex3->z);
		//	//outputConsole(tempBuf1);
		//	//outputConsole(tempBuf2);
		//	//outputConsole(tempBuf3);

		//	//outputConsole("----------------------------------------\r\n");


		//	//cube1Position.x = (triVertex1->x + triVertex2->x + triVertex3->x) / 3;
		//	//cube1Position.y = (triVertex1->y + triVertex2->y + triVertex3->y) / 3;
		//	//cube1Position.z = (triVertex1->z + triVertex2->z + triVertex3->z) / 3;
		//	XMFLOAT4 tempFloat((triVertex1->x + triVertex2->x + triVertex3->x) / 3, (triVertex1->y + triVertex2->y + triVertex3->y) / 3, (triVertex1->z + triVertex2->z + triVertex3->z) / 3, 0);

		//	//outputConsoleXMFLOAT4("1", objModels.at(0).getPos());
		//	objModels.at(1).setPos(tempFloat);
		//	//objModels.at(0).setPos(XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)); // set cube 1's position
		//	//outputConsoleXMFLOAT4("2", objModels.at(0).pos);	
		//}

		XMFLOAT3 P1(3, 1, -8);
		XMFLOAT3 P2(4, 4, 1);
		XMFLOAT3 P3(-5, 7, 2);

		//--------------------------------------------------------------------------------
		//--------------------------------------------------------------------------------
		// example from https://www.geogebra.org/3d/smzfybwa 
		//--------------------------------------------------------------------------------
		//--------------------------------------------------------------------------------
		////when ray AB does not intersect with triangle: intersection is point C
		////XMFLOAT3 rayPos(1, 2, -4); // point A
		////XMFLOAT3 rayDir(4, 4, 11); // distance between point A and point B: (B.x - A.x, B.y - A.y, B.z - A.z) 

		////when ray AB2 intersects with triangle: intersection is point C2
		//XMFLOAT3 rayPos(1, 2, -4); // point A
		//XMFLOAT3 rayDir(15, -12, 17); // distance between point A and point B2: (B2.x - A.x, B2.y - A.y, B2.z - A.z) 

		////XMFLOAT3 rayPos(1, 2, -4); // point A
		////XMFLOAT3 rayDir(3, -13.36, 10.62); // distance between point A and point B2: (B2.x - A.x, B2.y - A.y, B2.z - A.z) 
		////XMFLOAT3 rayDir(-6.84, -5.12, 10.62); // distance between point A and point B2: (B2.x - A.x, B2.y - A.y, B2.z - A.z) 


		//// triangle DEF
		//XMFLOAT3 P1(1, 0, 3); // point D
		//XMFLOAT3 P2(10, 0, 0); // point E
		//XMFLOAT3 P3(0, -5, 0); // point F


		//XMFLOAT3 intersectionPoint;
		//if (intersect3DTriangleWithRay(rayPos, rayDir, P1, P2, P3, &intersectionPoint)) {
		//	outputConsole("triangle intersected");
		//}
		//--------------------------------------------------------------------------------


		//---------------------------------------------------------------------------
		// 맞는지 보기 위해, 아무 matrix든 cameraPosWorldMat에 곱해서, 다시 cameraPosWorldMatInversed를 곱하면 원본이 나와야 한다.
		//---------------------------------------------------------------------------
		//float TEMP_MATRIX[4][4] = {
		//	{ 00, 01, 02, 03 },
		//	{ 10, 11, 12, 13 },
		//	{ 20, 21, 22, 23 },
		//	{ 30, 31, 32, 33 },
		//};
		//XMFLOAT4X4 temp4x4Matrix;
		//memcpy(temp4x4Matrix.m, TEMP_MATRIX, sizeof(float) * 4 * 4);
		//outputConsoleXMFLOAT4X4(string("temp4x4Matrix: "), temp4x4Matrix);
		//XMMATRIX tempXMMatrix = XMLoadFloat4x4(&temp4x4Matrix);
		//outputConsoleXMMATRIX(string("tempXMMatrix: "), tempXMMatrix);

		////XMMATRIX newTempXMMatrix = XMMatrixMultiply(tempXMMatrix, cameraPosWorldMat);
		//XMMATRIX newTempXMMatrix = tempXMMatrix * cameraPosWorldMat;
		//outputConsoleXMMATRIX(string("newTempXMMatrix: "), newTempXMMatrix);

		////XMMATRIX backTempXMMatrix = XMMatrixMultiply(newTempXMMatrix, cameraPosWorldMatInversed);
		//XMMATRIX backTempXMMatrix = newTempXMMatrix * cameraPosWorldMatInversed;
		//outputConsoleXMMATRIX(string("backTempXMMatrix: "), backTempXMMatrix);

		//XMMATRIX rayEyeMatrix = projMatInversed * rayClipMatrix;
		//outputConsoleXMMATRIX(string("rayEyeMatrix: "), rayEyeMatrix);
		//---------------------------------------------------------------------------

		//rayWorldMatrix = viewMatInversed * rayEyeMatrix;

		// create translation matrix for grid from grid's position vector
		//XMMATRIX translationMat = XMMatrixTranslationFromVector(XMLoadFloat4(&mouseClick));

		// update constant buffer for grid
		// create the wvp matrix and store in constant buffer
		//XMMATRIX wvpMat = projMatInversed * viewMatInversed; // create wvp matrix
		//XMMATRIX transposed = XMMatrixTranspose(rayWorldMatrix); // must transpose wvp matrix for the gpu
		//XMStoreFloat4x4(&cbPerObject.wvpMat, transposed); // store transposed wvp matrix in constant buffer

		//memcpy(cbvGPUAddress[frameIndex] + ConstantBufferPerObjectAlignedSize, &cbPerObject, sizeof(cbPerObject));
		}
		//exit(1);
		break;
	case WM_KEYDOWN: {
		//outputConsole("a");

			if (wParam == VK_ESCAPE) {
				if (MessageBox(0, L"Are you sure you want to exit?",
					L"Really?", MB_YESNO | MB_ICONQUESTION) == IDYES)
				{
					Running = false;
					DestroyWindow(hwnd);
				}
			}
			if (wParam == VK_F1 && curCameraDistanceFromViewObject - 1.0f > 0) {
				curCameraDistanceFromViewObject--;
			}
			if (wParam == VK_F2) {
				curCameraDistanceFromViewObject++;
			}

			int xMovement = 0;
			int yMovement = 0;

			if (wParam == VK_UP) {
				yMovement++;
			}
			if (wParam == VK_DOWN) {
				yMovement--;
			}
			if (wParam == VK_LEFT) {
				xMovement--;
			}
			if (wParam == VK_RIGHT) {
				xMovement++;
			}
			if (objectSelected == -1) {
				//gridPosition = XMFLOAT4(gridPosition.x + xMovement, gridPosition.y + yMovement, gridPosition.z, gridPosition.w); // set cube 1's position
				//XMVECTOR posVec = XMLoadFloat4(&gridPosition); // create xmvector for cube1's position

				//XMMATRIX tmpMat = XMMatrixTranslationFromVector(posVec); // create translation matrix from cube1's position vector
				//XMStoreFloat4x4(&gridRotMat, XMMatrixIdentity()); // initialize cube1's rotation matrix to identity matrix
				//XMStoreFloat4x4(&gridWorldMat, tmpMat); // store cube1's world matrix
			}
			if (objectSelected == 1) {

				//cube1Position = XMFLOAT4(cube1Position.x + xMovement, cube1Position.y + yMovement, cube1Position.z, cube1Position.w); // set cube 1's position
				//XMVECTOR posVec = XMLoadFloat4(&cube1Position); // create xmvector for cube1's position

				//XMMATRIX tmpMat = XMMatrixTranslationFromVector(posVec); // create translation matrix from cube1's position vector
				//XMStoreFloat4x4(&cube1WorldMat, tmpMat); // store cube1's world matrix
			}
			return 0;
		}
	case WM_DESTROY: // x button on top right corner of window was pressed
		Running = false;
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd,
		msg,
		wParam,
		lParam);
}

bool initObject(int objIndex);

BYTE* imageData;

bool InitD3D()
{
	HRESULT hr;

	// -- Create the Device -- //

	IDXGIFactory4* dxgiFactory;
	hr = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));
	if (FAILED(hr))
	{
		return false;
	}

	IDXGIAdapter1* adapter; // adapters are the graphics card (this includes the embedded graphics on the motherboard)

	int adapterIndex = 0; // we'll start looking for directx 12  compatible graphics devices starting at index 0

	bool adapterFound = false; // set this to true when a good one was found

							   // find first hardware gpu that supports d3d 12
	while (dxgiFactory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			// we dont want a software device
			continue;
		}

		// we want a device that is compatible with direct3d 12 (feature level 11 or higher)
		hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr);
		if (SUCCEEDED(hr))
		{
			adapterFound = true;
			break;
		}

		adapterIndex++;
	}

	if (!adapterFound)
	{
		Running = false;
		return false;
	}

	// Create the device
	hr = D3D12CreateDevice(
		adapter,
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&device)
	);
	if (FAILED(hr))
	{
		Running = false;
		return false;
	}

	// -- Create a direct command queue -- //

	D3D12_COMMAND_QUEUE_DESC cqDesc = {};
	cqDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	cqDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // direct means the gpu can directly execute this command queue

	hr = device->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&commandQueue)); // create the command queue
	if (FAILED(hr))
	{
		Running = false;
		return false;
	}

	// -- Create the Swap Chain (double/tripple buffering) -- //

	DXGI_MODE_DESC backBufferDesc = {}; // this is to describe our display mode
	backBufferDesc.Width = Width; // buffer width
	backBufferDesc.Height = Height; // buffer height
	backBufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // format of the buffer (rgba 32 bits, 8 bits for each chanel)

														// describe our multi-sampling. We are not multi-sampling, so we set the count to 1 (we need at least one sample of course)
	DXGI_SAMPLE_DESC sampleDesc = {};
	sampleDesc.Count = 1; // multisample count (no multisampling, so we just put 1, since we still need 1 sample)

						  // Describe and create the swap chain.
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount = frameBufferCount; // number of buffers we have
	swapChainDesc.BufferDesc = backBufferDesc; // our back buffer description
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // this says the pipeline will render to this swap chain
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // dxgi will discard the buffer (data) after we call present
	swapChainDesc.OutputWindow = hwnds[1]; // handle to our window
	swapChainDesc.SampleDesc = sampleDesc; // our multi-sampling description
	swapChainDesc.Windowed = !FullScreen; // set to true, then if in fullscreen must call SetFullScreenState with true for full screen to get uncapped fps

	IDXGISwapChain* tempSwapChain;

	dxgiFactory->CreateSwapChain(
		commandQueue, // the queue will be flushed once the swap chain is created
		&swapChainDesc, // give it the swap chain description we created above
		&tempSwapChain // store the created swap chain in a temp IDXGISwapChain interface
	);

	swapChain = static_cast<IDXGISwapChain3*>(tempSwapChain);

	frameIndex = swapChain->GetCurrentBackBufferIndex();

	// -- Create the Back Buffers (render target views) Descriptor Heap -- //

	// describe an rtv descriptor heap and create
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = frameBufferCount; // number of descriptors for this heap.
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // this heap is a render target view heap

													   // This heap will not be directly referenced by the shaders (not shader visible), as this will store the output from the pipeline
													   // otherwise we would set the heap's flag to D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	hr = device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvDescriptorHeap));
	if (FAILED(hr))
	{
		Running = false;
		return false;
	}

	// get the size of a descriptor in this heap (this is a rtv heap, so only rtv descriptors should be stored in it.
	// descriptor sizes may vary from device to device, which is why there is no set size and we must ask the 
	// device to give us the size. we will use this size to increment a descriptor handle offset
	rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// get a handle to the first descriptor in the descriptor heap. a handle is basically a pointer,
	// but we cannot literally use it like a c++ pointer.
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	// Create a RTV for each buffer (double buffering is two buffers, tripple buffering is 3).
	for (int i = 0; i < frameBufferCount; i++)
	{
		// first we get the n'th buffer in the swap chain and store it in the n'th
		// position of our ID3D12Resource array
		hr = swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i]));
		if (FAILED(hr))
		{
			Running = false;
			return false;
		}

		// the we "create" a render target view which binds the swap chain buffer (ID3D12Resource[n]) to the rtv handle
		device->CreateRenderTargetView(renderTargets[i], nullptr, rtvHandle);

		// we increment the rtv handle by the rtv descriptor size we got above
		rtvHandle.Offset(1, rtvDescriptorSize);
	}

	// -- Create the Command Allocators -- //

	for (int i = 0; i < frameBufferCount; i++)
	{
		hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator[i]));
		if (FAILED(hr))
		{
			Running = false;
			return false;
		}
	}

	// -- Create a Command List -- //

	// create the command list with the first allocator
	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator[frameIndex], NULL, IID_PPV_ARGS(&commandList));
	if (FAILED(hr))
	{
		Running = false;
		return false;
	}

	// -- Create a Fence & Fence Event -- //

	// create the fences
	for (int i = 0; i < frameBufferCount; i++)
	{
		hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence[i]));
		if (FAILED(hr))
		{
			Running = false;
			return false;
		}
		fenceValue[i] = 0; // set the initial fence value to 0
	}

	// create a handle to a fence event
	fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (fenceEvent == nullptr)
	{
		Running = false;
		return false;
	}

	// create root signature

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
	hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &errorBuff);
	if (FAILED(hr))
	{
		OutputDebugStringA((char*)errorBuff->GetBufferPointer());
		return false;
	}

	hr = device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	if (FAILED(hr))
	{
		return false;
	}

	// create vertex and pixel shaders

	// when debugging, we can compile the shader files at runtime.
	// but for release versions, we can compile the hlsl shaders
	// with fxc.exe to create .cso files, which contain the shader
	// bytecode. We can load the .cso files at runtime to get the
	// shader bytecode, which of course is faster than compiling
	// them at runtime

	// compile vertex shader
	ID3DBlob* vertexShader; // d3d blob for holding vertex shader bytecode
	hr = D3DCompileFromFile(L"VertexShader.hlsl",
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
		return false;
	}

	// fill out a shader bytecode structure, which is basically just a pointer
	// to the shader bytecode and the size of the shader bytecode
	D3D12_SHADER_BYTECODE vertexShaderBytecode = {};
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
		return false;
	}

	// fill out shader bytecode structure for pixel shader
	D3D12_SHADER_BYTECODE pixelShaderBytecode = {};
	pixelShaderBytecode.BytecodeLength = pixelShader->GetBufferSize();
	pixelShaderBytecode.pShaderBytecode = pixelShader->GetBufferPointer();

	// create input layout

	// The input layout is used by the Input Assembler so that it knows
	// how to read the vertex data bound to it.

	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	// fill out an input layout description structure
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = {};

	// we can get the number of elements in an array by "sizeof(array) / sizeof(arrayElementType)"
	inputLayoutDesc.NumElements = sizeof(inputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC);
	inputLayoutDesc.pInputElementDescs = inputLayout;

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
	hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineStateObject));
	if (FAILED(hr))
	{
		Running = false;
		return false;
	}

	//================================================================================
	//================================================================================
	//================================================================================
	// calling init object 
	//================================================================================
	//================================================================================
	//================================================================================

	if (!initObject(0)) {
		return false;
	}

	//msgBoxCstring("First Object Initialized");

	if (!initObject(1)) {
		return false;
	}

	//msgBoxCstring("Second Object Initialized");

	if (!initObject(2)) {
		return false;
	}

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
		hr = device->CreateCommittedResource(
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
			memcpy(cbvGPUAddress[i] + ConstantBufferPerObjectAlignedSize*j, &cbPerObject, sizeof(cbPerObject)); 
		}
	}

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

	//----------------------------------------------------------------------------
	// set starting objects position
	//----------------------------------------------------------------------------
	
	// grid
	for (int i = 0; i < 2; i++) {
		objModels.at(i).setPos(XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)); // set cube 1's position
		XMVECTOR posVec = XMLoadFloat4(&objModels.at(0).pos); // create xmvector for cube1's position

		tmpMat = XMMatrixTranslationFromVector(posVec); // create translation matrix from cube1's position vector
		XMStoreFloat4x4(&objModels.at(i).rotMat, XMMatrixIdentity()); // initialize cube1's rotation matrix to identity matrix
		XMStoreFloat4x4(&objModels.at(i).worldMat, tmpMat); // store cube1's world matrix
	}
	// first cube
	//cube1Position = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f); // set cube 1's position
	//posVec = XMLoadFloat4(&cube1Position); // create xmvector for cube1's position

	//tmpMat = XMMatrixTranslationFromVector(posVec); // create translation matrix from cube1's position vector
	//XMStoreFloat4x4(&cube1RotMat, XMMatrixIdentity()); // initialize cube1's rotation matrix to identity matrix
	//XMStoreFloat4x4(&cube1WorldMat, tmpMat); // store cube1's world matrix

	// second cube
//	cube2PositionOffset = XMFLOAT4(-0.5f, -0.6f, 1.0f, 0.0f);
	//cube2PositionOffset = XMFLOAT4(0.0f, 0.0f, 10.0f, 0.0f);
	//posVec = XMLoadFloat4(&cube2PositionOffset) + XMLoadFloat4(&cube1Position); // create xmvector for cube2's position
	//																			// we are rotating around cube1 here, so add cube2's position to cube1
//	XMVECTOR posVec = XMLoadFloat4(&cube2PositionOffset) + XMLoadFloat4(&cameraPosition); // create xmvector for cube2's position
																				// we are rotating around cube1 here, so add cube2's position to cube1

//	tmpMat = XMMatrixTranslationFromVector(posVec); // create translation matrix from cube2's position offset vector
//	XMStoreFloat4x4(&cube2RotMat, XMMatrixIdentity()); // initialize cube2's rotation matrix to identity matrix
//	XMStoreFloat4x4(&cube2WorldMat, tmpMat); // store cube2's world matrix

	return true;
}

bool initObject(int objIndex) {

	HRESULT hr;

	// vertex buffer and index buffer are only used inside of this function so it is not necessary to turn into a vector; it is done as a safety measure
	//ID3D12Resource *vertexBuffer; // a default buffer in GPU memory that we will load vertex data for our triangle into
	//vertexBuffers.push_back(vertexBuffer);
	vertexBuffers.push_back(NULL);

	//ID3D12Resource *indexBuffer; // a default buffer in GPU memory that we will load vertex data for our triangle into
	//indexBuffers.push_back(indexBuffer);
	indexBuffers.push_back(NULL);

	ID3D12DescriptorHeap *mainDescriptorHeap;
	mainDescriptorHeaps.push_back(mainDescriptorHeap);

	//D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	//vertexBufferViews.push_back(vertexBufferView);
	vertexBufferViews.push_back(D3D12_VERTEX_BUFFER_VIEW());

	//D3D12_INDEX_BUFFER_VIEW indexBufferView;
	//indexBufferViews.push_back(indexBufferView);
	indexBufferViews.push_back(D3D12_INDEX_BUFFER_VIEW());

	dsDescriptorHeaps.push_back(NULL);


	Vertex *vList = &(objModels.at(objIndex).finalVertices[0]);
	int vBufferSize = sizeof(Vertex) * objModels.at(objIndex).finalVertices.size();


	//sprintf(g_buffer, "main.cpp::initObject(): objModel.finalVertices.size(): %d", objModels.at(objIndex).finalVertices.size());
	//msgBoxCstring(g_buffer);

	//sprintf(g_buffer, "main.cpp::initObject(): vBufferSize: %d", vBufferSize);
	//msgBoxCstring(g_buffer);

	// create default heap
	// default heap is memory on the GPU. Only the GPU has access to this memory
	// To get data into this heap, we will have to upload the data using
	// an upload heap

	hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // a default heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&CD3DX12_RESOURCE_DESC::Buffer(vBufferSize), // resource description for a buffer
		D3D12_RESOURCE_STATE_COPY_DEST, // we will start this heap in the copy destination state since we will copy data
										// from the upload heap to this heap
		nullptr, // optimized clear value must be null for this type of resource. used for render targets and depth/stencil buffers
		IID_PPV_ARGS(&vertexBuffers[objIndex]));
	if (FAILED(hr))
	{
		Running = false;
		return false;
	}

	// we can give resource heaps a name so when we debug with the graphics debugger we know what resource we are looking at
	vertexBuffers[objIndex]->SetName(L"Vertex Buffer Resource Heap");

	// create upload heap
	// upload heaps are used to upload data to the GPU. CPU can write to it, GPU can read from it
	// We will upload the vertex buffer using this heap to the default heap
	ID3D12Resource* vBufferUploadHeap;
	hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&CD3DX12_RESOURCE_DESC::Buffer(vBufferSize), // resource description for a buffer
		D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
		nullptr,
		IID_PPV_ARGS(&vBufferUploadHeap));
	if (FAILED(hr))
	{
		Running = false;
		return false;
	}
	vBufferUploadHeap->SetName(L"Vertex Buffer Upload Resource Heap");

	// store vertex buffer in upload heap
	D3D12_SUBRESOURCE_DATA vertexData = {};
	vertexData.pData = reinterpret_cast<BYTE*>(vList); // pointer to our vertex array
	vertexData.RowPitch = vBufferSize; // size of all our triangle vertex data
	vertexData.SlicePitch = vBufferSize; // also the size of our triangle vertex data

	// we are now creating a command with the command list to copy the data from
	// the upload heap to the default heap
	UpdateSubresources(commandList, vertexBuffers[objIndex], vBufferUploadHeap, 0, 0, 1, &vertexData);

	// transition the vertex buffer data from copy destination state to vertex buffer state
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(vertexBuffers[objIndex], D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

	char cStrBuf1[2048];
	sprintf(cStrBuf1, "main.cpp::initObject(): quadFaces.size(): %d", objModels.at(objIndex).quadFaces.size());
	//msgBoxCstring(cStrBuf1);

	for (int i = 0; i < objModels.at(objIndex).quadFaces.size(); i++) {
	//for (int i = 0; i < 437; i++) {
			QuadFace curQuadFace = objModels.at(objIndex).quadFaces[i];
		if (curQuadFace.indices[3] == -1) { // tri face
			objModels.at(objIndex).triFaces.push_back(TriFace(curQuadFace.indices[0], curQuadFace.indices[1], curQuadFace.indices[2]));
		}
		else {  // quad face
			objModels.at(objIndex).triFaces.push_back(TriFace(curQuadFace.indices[3], curQuadFace.indices[0], curQuadFace.indices[1]));
			objModels.at(objIndex).triFaces.push_back(TriFace(curQuadFace.indices[2], curQuadFace.indices[3], curQuadFace.indices[1]));
		}
	}


	//ObjModel *curObjModel = &objModels.at(0);
	//for (int t = 0; t < 100; t++) {
	//	curObjModel->finalVertices.at(curObjModel->triFaces[t].indices[0]).texCoord.x = 0;
	//	curObjModel->finalVertices.at(curObjModel->triFaces[t].indices[0]).texCoord.y = 0;
	//	curObjModel->finalVertices.at(curObjModel->triFaces[t].indices[1]).texCoord.x = 0;
	//	curObjModel->finalVertices.at(curObjModel->triFaces[t].indices[1]).texCoord.y = 0;
	//	curObjModel->finalVertices.at(curObjModel->triFaces[t].indices[2]).texCoord.x = 0;
	//	curObjModel->finalVertices.at(curObjModel->triFaces[t].indices[2]).texCoord.y = 0;
	//}

	DWORD *iList = (DWORD *)&(objModels.at(objIndex).triFaces[0]);
	int iBufferSize = sizeof(TriFace) * objModels.at(objIndex).triFaces.size();
	numObjIndices.push_back(objModels.at(objIndex).triFaces.size() * 3);

	//sprintf(g_buffer, "main.cpp::initObject(): triFaces.size(): %d", objModels.at(objIndex).triFaces.size());
	//msgBoxCstring(g_buffer);
	//sprintf(g_buffer, "main.cpp::initObject(): iBufferSize: %d", iBufferSize);
	//msgBoxCstring(g_buffer);
	//sprintf(g_buffer, "main.cpp::initObject(): numCubeIndices: %d", numObjIndices);
	//msgBoxCstring(g_buffer);

	// create default heap to hold index buffer
	hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // a default heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&CD3DX12_RESOURCE_DESC::Buffer(iBufferSize), // resource description for a buffer
		D3D12_RESOURCE_STATE_COPY_DEST, // start in the copy destination state
		nullptr, // optimized clear value must be null for this type of resource
		IID_PPV_ARGS(&indexBuffers[objIndex]));
	if (FAILED(hr))
	{
		Running = false;
		return false;
	}

	// we can give resource heaps a name so when we debug with the graphics debugger we know what resource we are looking at
	vertexBuffers[objIndex]->SetName(L"Index Buffer Resource Heap");

	// create upload heap to upload index buffer
	ID3D12Resource* iBufferUploadHeap;
	hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&CD3DX12_RESOURCE_DESC::Buffer(iBufferSize), // resource description for a buffer
		D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
		nullptr,
		IID_PPV_ARGS(&iBufferUploadHeap));
	if (FAILED(hr))
	{
		Running = false;
		return false;
	}
	vBufferUploadHeap->SetName(L"Index Buffer Upload Resource Heap");

	// store vertex buffer in upload heap
	D3D12_SUBRESOURCE_DATA indexData = {};
	indexData.pData = reinterpret_cast<BYTE*>(iList); // pointer to our index array
	indexData.RowPitch = iBufferSize; // size of all our index buffer
	indexData.SlicePitch = iBufferSize; // also the size of our index buffer

	// we are now creating a command with the command list to copy the data from
	// the upload heap to the default heap
	UpdateSubresources(commandList, indexBuffers[objIndex], iBufferUploadHeap, 0, 0, 1, &indexData);

	// transition the vertex buffer data from copy destination state to vertex buffer state
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(indexBuffers[objIndex], D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

	// Create the depth/stencil buffer

	// create a depth stencil descriptor heap so we can get a pointer to the depth stencil buffer
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	hr = device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsDescriptorHeaps[objIndex]));
	if (FAILED(hr))
	{
		Running = false;
		return false;
	}

	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
	depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
	depthOptimizedClearValue.DepthStencil.Stencil = 0;

	hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, Width, Height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthOptimizedClearValue,
		IID_PPV_ARGS(&depthStencilBuffer)
		);
	if (FAILED(hr))
	{
		Running = false;
		return false;
	}
	dsDescriptorHeaps[objIndex]->SetName(L"Depth/Stencil Resource Heap");

	device->CreateDepthStencilView(depthStencilBuffer, &depthStencilDesc, dsDescriptorHeaps[objIndex]->GetCPUDescriptorHandleForHeapStart());

	// load the image, create a texture resource and descriptor heap

	// create the descriptor heap that will store our srv
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = 1;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	hr = device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mainDescriptorHeaps[objIndex]));
	if (FAILED(hr))
	{
		Running = false;
	}

	// Load the image from file
	D3D12_RESOURCE_DESC textureDesc;
	int imageBytesPerRow;
	//int imageSize = LoadImageDataFromFile(&imageData, textureDesc, L"imp_exp.png", imageBytesPerRow);
	//int imageSize = LoadImageDataFromFile(&imageData, textureDesc, L"..\\..\\imp_exp\\imp_exp.png", imageBytesPerRow);
	//int imageSize = LoadImageDataFromFile(&imageData, textureDesc, L"..\\..\\imp_exp\\ash_uvgrid03.jpg", imageBytesPerRow);


	size_t len = strlen(objModelImagePaths.at(objIndex).c_str());
	WCHAR imagePath[280 + 1];
	int result = MultiByteToWideChar(CP_OEMCP, 0, objModelImagePaths.at(objIndex).c_str(), -1, imagePath, len + 1);

	//char cStrBuf1[2048];
	//sprintf(cStrBuf1, "main.cpp::initObject(): initObject(): objModelImagePaths[%d] = |%s|", objIndex, objModelImagePaths.at(objIndex).c_str());
	//msgBoxCstring(cStrBuf1);

	int imageSize = LoadImageDataFromFile(&imageData, textureDesc, imagePath, imageBytesPerRow);
	//int imageSize = LoadImageDataFromFile(&imageData, textureDesc, L"..\\..\\imp_exp\\character_creation.png", imageBytesPerRow);
	
	// make sure we have data
	if(imageSize <= 0)
	{
		Running = false;
		return false;
	}

	// create a default heap where the upload heap will copy its contents into (contents being the texture)
	hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // a default heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&textureDesc, // the description of our texture
		D3D12_RESOURCE_STATE_COPY_DEST, // We will copy the texture from the upload heap to here, so we start it out in a copy dest state
		nullptr, // used for render targets and depth/stencil buffers
		IID_PPV_ARGS(&textureBuffer));
	if (FAILED(hr))
	{
		Running = false;
		return false;
	}
	textureBuffer->SetName(L"Texture Buffer Resource Heap");

	UINT64 textureUploadBufferSize;
	// this function gets the size an upload buffer needs to be to upload a texture to the gpu.
	// each row must be 256 byte aligned except for the last row, which can just be the size in bytes of the row
	// eg. textureUploadBufferSize = ((((width * numBytesPerPixel) + 255) & ~255) * (height - 1)) + (width * numBytesPerPixel);
	//textureUploadBufferSize = (((imageBytesPerRow + 255) & ~255) * (textureDesc.Height - 1)) + imageBytesPerRow;
	device->GetCopyableFootprints(&textureDesc, 0, 1, 0, nullptr, nullptr, nullptr, &textureUploadBufferSize);

	// now we create an upload heap to upload our texture to the GPU
	hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&CD3DX12_RESOURCE_DESC::Buffer(textureUploadBufferSize), // resource description for a buffer (storing the image data in this heap just to copy to the default heap)
		D3D12_RESOURCE_STATE_GENERIC_READ, // We will copy the contents from this heap to the default heap above
		nullptr,
		IID_PPV_ARGS(&textureBufferUploadHeap));
	if (FAILED(hr))
	{
		Running = false;
		return false;
	}
	textureBufferUploadHeap->SetName(L"Texture Buffer Upload Resource Heap");

	// store vertex buffer in upload heap
	D3D12_SUBRESOURCE_DATA textureData = {};
	textureData.pData = &imageData[0]; // pointer to our image data
	textureData.RowPitch = imageBytesPerRow; // size of all our triangle vertex data
	textureData.SlicePitch = imageBytesPerRow * textureDesc.Height; // also the size of our triangle vertex data

	// Now we copy the upload buffer contents to the default heap
	UpdateSubresources(commandList, textureBuffer, textureBufferUploadHeap, 0, 0, 1, &textureData);

	// transition the texture default heap to a pixel shader resource (we will be sampling from this heap in the pixel shader to get the color of pixels)
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(textureBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

	// now we create a shader resource view (descriptor that points to the texture and describes it)
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	device->CreateShaderResourceView(textureBuffer, &srvDesc, mainDescriptorHeaps[objIndex]->GetCPUDescriptorHandleForHeapStart());

	// we are done with image data now that we've uploaded it to the gpu, so free it up
	delete imageData;

	// create a vertex buffer view for the triangle. We get the GPU memory address to the vertex pointer using the GetGPUVirtualAddress() method
	vertexBufferViews[objIndex].BufferLocation = vertexBuffers[objIndex]->GetGPUVirtualAddress();
	vertexBufferViews[objIndex].StrideInBytes = sizeof(Vertex);
	vertexBufferViews[objIndex].SizeInBytes = vBufferSize;

	// create a index buffer view for the triangle. We get the GPU memory address to the vertex pointer using the GetGPUVirtualAddress() method
	indexBufferViews[objIndex].BufferLocation = indexBuffers[objIndex]->GetGPUVirtualAddress();
	indexBufferViews[objIndex].Format = DXGI_FORMAT_R32_UINT; // 32-bit unsigned integer (this is what a dword is, double word, a word is 2 bytes)
	indexBufferViews[objIndex].SizeInBytes = iBufferSize;

	numObjModels++;

	//char cStrBuf1[2048];
	sprintf(cStrBuf1, "main.cpp::initObject(): initObject(): end!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
	//msgBoxCstring(cStrBuf1);

	return true;
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
void UpdateCamera() {

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

// 로테이션이나 패닝없을 때의 원본 버젼: 카메라 위치와 target위치에 의해 이상한 현상없이 카메라가 target을 바라보는 버젼.
void UpdateCameraOriginal() {
	// build projection and view matrix
	XMMATRIX tmpMat = XMMatrixPerspectiveFovLH(45.0f*(3.14f / 180.0f), (float)Width / (float)Height, 0.1f, 1000.0f);
	XMStoreFloat4x4(&cameraProjMat, tmpMat);

	// line 1155에서는 이렇게 초기에 셋팅되어 있음.
	//cameraPosition = XMFLOAT4(CAMERA_DEFAULT_X, CAMERA_DEFAULT_Y, CAMERA_DEFAULT_Z, 0.0f);
	//cameraTarget = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	//cameraUp = XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f);

	// set starting camera state
	//cameraPosition = XMFLOAT4(0.0f, 2.0f, -8.0f, 0.0f);
	//cameraTarget = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	//cameraUp = XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f);

	// build view matrix
	XMVECTOR cPos = XMLoadFloat4(&cameraPosition);
	XMVECTOR cTarg = XMLoadFloat4(&cameraTarget);
	XMVECTOR cUp = XMLoadFloat4(&cameraUp);
	tmpMat = XMMatrixLookAtLH(cPos, cTarg, cUp);
	XMStoreFloat4x4(&cameraViewMat, tmpMat);
	viewMatrix = tmpMat;
}

// 카메라 상하 좌우 패닝이 제대로 되는 버젼..
// 앨거리듬:
//     1. Camera Position은 Camera Center로부터 상대적인 rotation만 적용.
//     2. Camera Target에다가 panning을 적용.
//     3. 그 Camera Target 위치에다가 상대적인 Camera Position 변환값을 더하면 최종 Camera Position값이 된다.
void UpdateCameraNewVer0001() {
	// build projection and view matrix
	XMMATRIX tmpMat = XMMatrixPerspectiveFovLH(45.0f*(3.14f / 180.0f), (float)Width / (float)Height, 0.1f, 1000.0f);
	XMStoreFloat4x4(&cameraProjMat, tmpMat);

	// (chai:20180706)
	XMMATRIX uiCameraTmpMat = XMMatrixPerspectiveFovLH(45.0f*(3.14f / 180.0f), (float)Width / (float)Height, 0.001f, 1000.0f);
	XMStoreFloat4x4(&uiCameraProjMat, uiCameraTmpMat);

	//================================================================================
	// 1. Camera Position은 Camera Center로부터 상대적인 rotation만 적용.
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
	// 2. Camera Target에다가 panning을 적용.
	//================================================================================

	// get camera target position after panning
	float curCameraTargetRadiusOfCircleOnXZPlane = (0.0f)*cos(curCameraDegreeOnXYPlaneRadian) - (0.0f + yOnCameraPanningPlane)*sin(curCameraDegreeOnXYPlaneRadian);
	float curCameraTargetPostionY = (0.0f)*sin(curCameraDegreeOnXYPlaneRadian) + (0.0f + yOnCameraPanningPlane)*cos(curCameraDegreeOnXYPlaneRadian);

	float curCameraTargetPostionX = (curCameraTargetRadiusOfCircleOnXZPlane)*cos(curCameraDegreeOnXZPlaneRadian) - (0.0f + xOnCameraPanningPlane)*sin(curCameraDegreeOnXZPlaneRadian);
	float curCameraTargetPostionZ = (curCameraTargetRadiusOfCircleOnXZPlane)*sin(curCameraDegreeOnXZPlaneRadian) + (0.0f + xOnCameraPanningPlane)*cos(curCameraDegreeOnXZPlaneRadian);

	//================================================================================
	// 3. 그 Camera Target 위치에다가 상대적인 Camera Position 변환값을 더하면 최종 Camera Position값이 된다.
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


// chai+kj original
void UpdateCameraByMovingCameraAndCameraTarget() {
	//====================================================================================================
	//====================================================================================================
	// Set Camera
	//====================================================================================================
	//====================================================================================================

	// build projection and view matrix
	XMMATRIX tmpMat = XMMatrixPerspectiveFovLH(45.0f*(3.14f / 180.0f), (float)Width / (float)Height, 0.1f, 1000.0f);
	XMStoreFloat4x4(&cameraProjMat, tmpMat);

	// (chai:20180706)
	XMMATRIX uiCameraTmpMat = XMMatrixPerspectiveFovLH(45.0f*(3.14f / 180.0f), (float)Width / (float)Height, 0.001f, 1000.0f);
	XMStoreFloat4x4(&uiCameraProjMat, uiCameraTmpMat);

	// set starting camera state
	//cameraPosition = XMFLOAT4(cameraPosition.x, cameraPosition.y, cameraPosition.z, cameraPosition.w); // camera position
	//cameraTarget = XMFLOAT4(cameraTarget.x, cameraTarget.y, cameraTarget.z, cameraTarget.w);           // camera look at vector
	//cameraUp = XMFLOAT4(cameraUp.x, cameraUp.y, cameraUp.z, cameraUp.w);

	//-------------------------------------------------------------------------
	// Source: http://egloos.zum.com/scripter/v/1618411
	//-------------------------------------------------------------------------
	/*
	cx, cy가 회전중심점
	px, py가 원점
	rx, ry가 돌아간 점.

	private function init():void
	{
	//특정 오브젝트 sp 의 원래 좌표
	sp.x = 100
	sp.y = 100

	var ob : Object = transformation(0, 0, sp.x, sp.y, 30 * Math.PI / 180)
	trace(ob.x, ob.y)

	sp.x = ob.x
	sp.y = ob.y
	}

	private function transformation(cx:Number, cy : Number,
	px : Number, py : Number,
	rad : Number) :Object
	{

	var rx : Number = (px - cx)*Math.cos(rad) - (py - cy)*Math.sin(rad) + cx; <-- 이것 가져다 씀.
	var ry : Number = (px - cx)*Math.sin(rad) + (py - cy)*Math.cos(rad) + cy; <-- 이것 가져다 씀.
	return { x:rx , y : ry }
	}
	*/
	
	//-------------------------------------------------------------------------
	// 계산 앨거리듬:
	// 1. 일단 xy평면(left side에서 바라보는 듯한)에서 x를 1.0(unit)으로 놓고, y는 0으로 놓은 다음, 원을 따라서 각도로 카메라를 들어올린다고 생각하고 계산한다.
	//    이때, xz평면에서, 이제 카메라가 좌우로 돌아간 것을 표현하려면, radius가 더 이상 1이 아니고 cos(높이 각도)가 되어 더 줄어든다.
	// 2. 다시 위에서 바라보는 xz평면에서, x를 줄어든 위에서 나온 cos(높이 각도)으로 놓고, z는 0로 놓은 다음, 다시 좌우로 돌아간 각도로 회전공식을 적용하면 된다.
	// 3. 마지막으로 카메라가 원점인 0,0,0을 바라보게 하면 된다.
	//-------------------------------------------------------------------------
	cameraTarget.x = 0.0f;
	cameraTarget.y = 0.0f;
	cameraTarget.z = 0.0f;

	//float xOnCameraPanningPlane = 0.0f; // chai:ver0049: 카메라가 패닝할 때, x축으로 움직이는 범위. 좌는 마이너스, 우는 플러스.
	//cameraTarget.z = -5.0f; // 만약 (x:0, z:5)를 중심으로 돌려보고 싶을 때..(현재는 x:0, y:0, z:0) 카메라 시작 위치는(x:0, y:2.0, z:-10.0), xz plane에서 270도에서 시작해서 반 시계 방향으로 돈다. 270에서 0.001씩 증가.
	float curCameraDegreeOnXYPlaneRadian = curCameraDegreeOnXYPlane * XM_PI / 180.0f;
	float curRadiusOfCircleOnXZPlane = (1.0f - cameraTarget.x)*cos(curCameraDegreeOnXYPlaneRadian) - (0.0f - cameraTarget.y)*sin(curCameraDegreeOnXYPlaneRadian) + cameraTarget.x;
	float curCameraPostionY = (1.0f - cameraTarget.x)*sin(curCameraDegreeOnXYPlaneRadian) + (0.0f - cameraTarget.y)*cos(curCameraDegreeOnXYPlaneRadian) + cameraTarget.y;

	float curCameraDegreeOnXZPlaneRadian = curCameraDegreeOnXZPlane * XM_PI / 180.0f;
	////float curCameraPostionX = (curRadiusOfCircleOnXZPlane - cameraTarget.x)*cos(curCameraDegreeOnXZPlaneRadian) - (0.0f - cameraTarget.z)*sin(curCameraDegreeOnXZPlaneRadian) + cameraTarget.x;
	////float curCameraPostionZ = (curRadiusOfCircleOnXZPlane - cameraTarget.x)*sin(curCameraDegreeOnXZPlaneRadian) + (0.0f - cameraTarget.z)*cos(curCameraDegreeOnXZPlaneRadian) + cameraTarget.z;
	float curCameraPostionX = (curRadiusOfCircleOnXZPlane - cameraTarget.x)*cos(curCameraDegreeOnXZPlaneRadian) - ((0.0f + xOnCameraPanningPlane) - cameraTarget.z)*sin(curCameraDegreeOnXZPlaneRadian) + cameraTarget.x;
	float curCameraPostionZ = (curRadiusOfCircleOnXZPlane - cameraTarget.x)*sin(curCameraDegreeOnXZPlaneRadian) + ((0.0f + xOnCameraPanningPlane) - cameraTarget.z)*cos(curCameraDegreeOnXZPlaneRadian) + cameraTarget.z;

	////tempZ += 0.001f;
	////xOnCameraPanningPlane += 0.0001f;
	////if (xOnCameraPanningPlane > (2.0f / curCameraDistanceFromViewObject)) {
	////	xOnCameraPanningPlane = 2.0f;
	////}

	//curCameraPostionX *= curCameraDistanceFromViewObject;
	//curCameraPostionY *= curCameraDistanceFromViewObject;
	//curCameraPostionZ *= curCameraDistanceFromViewObject;

	//float curCameraTargetPostionX = (0.0f - cameraTarget.x)*cos(curCameraDegreeOnXZPlaneRadian) - ((0.0f + xOnCameraPanningPlane) - cameraTarget.z)*sin(curCameraDegreeOnXZPlaneRadian) + cameraTarget.x;
	//float curCameraTargetPostionZ = (0.0f - cameraTarget.x)*sin(curCameraDegreeOnXZPlaneRadian) + ((0.0f + xOnCameraPanningPlane) - cameraTarget.z)*cos(curCameraDegreeOnXZPlaneRadian) + cameraTarget.z;
	////cameraTarget.x = curCameraTargetPostionX * curCameraDistanceFromViewObject;
	////cameraTarget.z = curCameraTargetPostionZ * curCameraDistanceFromViewObject;
	//cameraTarget.x = curCameraTargetPostionX * 0.5f;
	//cameraTarget.z = curCameraTargetPostionZ * 0.5f;

	//cameraPosition = XMFLOAT4(curCameraPostionX, curCameraPostionY, curCameraPostionZ, cameraPosition.w); // camera position
	//cameraTarget = XMFLOAT4(cameraTarget.x, cameraTarget.y, cameraTarget.z, cameraTarget.w); // camera look at vector
	//cameraUp = XMFLOAT4(cameraUp.x, cameraUp.y, cameraUp.z, cameraUp.w); // camera up(rotation)

	//float curCameraDegreeOnXYPlaneRadian = curCameraDegreeOnXYPlane * XM_PI / 180.0f;
	//float curRadiusOfCircleOnXZPlane = (1.0f)*cos(curCameraDegreeOnXYPlaneRadian) - (0.0f)*sin(curCameraDegreeOnXYPlaneRadian);
	//float curCameraPostionY = (1.0f)*sin(curCameraDegreeOnXYPlaneRadian) + (0.0f)*cos(curCameraDegreeOnXYPlaneRadian);

	//float curCameraDegreeOnXZPlaneRadian = curCameraDegreeOnXZPlane * XM_PI / 180.0f;
	////float curCameraPostionX = (curRadiusOfCircleOnXZPlane)*cos(curCameraDegreeOnXZPlaneRadian) - (0.0f)*sin(curCameraDegreeOnXZPlaneRadian);
	////float curCameraPostionZ = (curRadiusOfCircleOnXZPlane)*sin(curCameraDegreeOnXZPlaneRadian) + (0.0f)*cos(curCameraDegreeOnXZPlaneRadian);
	////float curCameraPostionX = (curRadiusOfCircleOnXZPlane)*cos(curCameraDegreeOnXZPlaneRadian) - ((0.0f + xOnCameraPanningPlane))*sin(curCameraDegreeOnXZPlaneRadian);
	////float curCameraPostionZ = (curRadiusOfCircleOnXZPlane)*sin(curCameraDegreeOnXZPlaneRadian) + ((0.0f + xOnCameraPanningPlane))*cos(curCameraDegreeOnXZPlaneRadian);
	//float curCameraTargetPostionX = (0.0f - cameraTarget.x)*cos(curCameraDegreeOnXZPlaneRadian) - ((0.0f + xOnCameraPanningPlane) - cameraTarget.z)*sin(curCameraDegreeOnXZPlaneRadian) + cameraTarget.x;
	//float curCameraTargetPostionZ = (0.0f - cameraTarget.x)*sin(curCameraDegreeOnXZPlaneRadian) + ((0.0f + xOnCameraPanningPlane) - cameraTarget.z)*cos(curCameraDegreeOnXZPlaneRadian) + cameraTarget.z;

	//tempZ += 0.001f;
	//xOnCameraPanningPlane += 0.0001f;
	//if (xOnCameraPanningPlane > (2.0f / curCameraDistanceFromViewObject)) {
	//	xOnCameraPanningPlane = 2.0f;
	//}

	curCameraPostionX *= curCameraDistanceFromViewObject;
	curCameraPostionY *= curCameraDistanceFromViewObject;
	curCameraPostionZ *= curCameraDistanceFromViewObject;

	float curCameraTargetPostionX = (0.0f - cameraTarget.x)*cos(curCameraDegreeOnXZPlaneRadian) - ((0.0f + xOnCameraPanningPlane) - cameraTarget.z)*sin(curCameraDegreeOnXZPlaneRadian) + cameraTarget.x;
	float curCameraTargetPostionZ = (0.0f - cameraTarget.x)*sin(curCameraDegreeOnXZPlaneRadian) + ((0.0f + xOnCameraPanningPlane) - cameraTarget.z)*cos(curCameraDegreeOnXZPlaneRadian) + cameraTarget.z;
	////cameraTarget.x = curCameraTargetPostionX * curCameraDistanceFromViewObject;
	////cameraTarget.z = curCameraTargetPostionZ * curCameraDistanceFromViewObject;
	//cameraTarget.x = curCameraTargetPostionX * 0.1f;
	//cameraTarget.z = curCameraTargetPostionZ * 0.1f;
	//cameraTarget.x += cameraPosition.x - curCameraPostionX;
	//cameraTarget.z += cameraPosition.z - curCameraPostionZ;
	//float curCameraTargetPostionX = (0.0f)*cos(curCameraDegreeOnXZPlaneRadian) - ((0.0f + xOnCameraPanningPlane))*sin(curCameraDegreeOnXZPlaneRadian);
	//float curCameraTargetPostionZ = (0.0f)*sin(curCameraDegreeOnXZPlaneRadian) + ((0.0f + xOnCameraPanningPlane))*cos(curCameraDegreeOnXZPlaneRadian);
	//cameraTarget.x = curCameraTargetPostionX * curCameraDistanceFromViewObject;
	//cameraTarget.z = curCameraTargetPostionZ * curCameraDistanceFromViewObject;

	cameraPosition = XMFLOAT4(curCameraPostionX, curCameraPostionY, curCameraPostionZ, cameraPosition.w); // camera position
	cameraTarget = XMFLOAT4(cameraTarget.x, cameraTarget.y, cameraTarget.z, cameraTarget.w); // camera look at vector
	cameraUp = XMFLOAT4(cameraUp.x, cameraUp.y, cameraUp.z, cameraUp.w); // camera up(rotation)

	// rotate camera
	//curCameraDegreeOnXZPlane += 0.003f; // camera move때 이미 위에서 만들면서 이것을 지움.
	//-------------------------------------------------------------------------

	// build view matrix
	XMVECTOR cPos = XMLoadFloat4(&cameraPosition);
	XMVECTOR cTarg = XMLoadFloat4(&cameraTarget);
	XMVECTOR cUp = XMLoadFloat4(&cameraUp);
	tmpMat = XMMatrixLookAtLH(cPos, cTarg, cUp);
	XMStoreFloat4x4(&cameraViewMat, tmpMat);
	viewMatrix = tmpMat;
}

void Update() {
	float prevCameraX = cameraPosition.x;
	float prevCameraY = cameraPosition.y;
	float prevCameraZ = cameraPosition.z;

	float prevCameraTargetX = cameraTarget.x;
	float prevCameraTargetY = cameraTarget.y;
	float prevCameraTargetZ = cameraTarget.z;

//	UpdateCamera(); // works: 문제는 그냥 카메라가 초기 셋팅된 상태에서 cameraViewMat만 움직임. 실제 카메라나 원점은 움직이지 않음. 그리고 특정 target을 중심으로 돌지 못함.
//  UpdateCameraOriginal(); // works: 여기에서는 target을 조정하면 look at이 제대로 동작하였음.
	UpdateCameraNewVer0001(); // mission: 꼭 원점이 아니더라도 target을 유지하면서 그 둘레를 camera가 움직일 수 있게 바꿀 것.
//  UpdateCameraByMovingCameraAndCameraTarget();
/*
	//====================================================================================================
	//====================================================================================================
	// Set Camera
	//====================================================================================================
	//====================================================================================================

	// build projection and view matrix
	XMMATRIX tmpMat = XMMatrixPerspectiveFovLH(45.0f*(3.14f / 180.0f), (float)Width / (float)Height, 0.1f, 1000.0f);
	XMStoreFloat4x4(&cameraProjMat, tmpMat);

	// (chai:20180706)
	XMMATRIX uiCameraTmpMat = XMMatrixPerspectiveFovLH(45.0f*(3.14f / 180.0f), (float)Width / (float)Height, 0.001f, 1000.0f);
	XMStoreFloat4x4(&uiCameraProjMat, uiCameraTmpMat);

	// set starting camera state
	//cameraPosition = XMFLOAT4(cameraPosition.x, cameraPosition.y, cameraPosition.z, cameraPosition.w); // camera position
	//cameraTarget = XMFLOAT4(cameraTarget.x, cameraTarget.y, cameraTarget.z, cameraTarget.w);           // camera look at vector
	//cameraUp = XMFLOAT4(cameraUp.x, cameraUp.y, cameraUp.z, cameraUp.w);
*/
	//-------------------------------------------------------------------------
	// Source: http://egloos.zum.com/scripter/v/1618411
	//-------------------------------------------------------------------------
    /*
	cx, cy가 회전중심점
	px, py가 원점
	rx, ry가 돌아간 점.

	private function init():void
	{
		//특정 오브젝트 sp 의 원래 좌표
		sp.x = 100
			sp.y = 100

			var ob : Object = transformation(0, 0, sp.x, sp.y, 30 * Math.PI / 180)
			trace(ob.x, ob.y)

			sp.x = ob.x
			sp.y = ob.y
	}

	private function transformation(cx:Number, cy : Number,
		px : Number, py : Number,
		rad : Number) :Object
	{

		var rx : Number = (px - cx)*Math.cos(rad) - (py - cy)*Math.sin(rad) + cx; <-- 이것 가져다 씀.
		var ry : Number = (px - cx)*Math.sin(rad) + (py - cy)*Math.cos(rad) + cy; <-- 이것 가져다 씀.
		return { x:rx , y : ry }
	}
	*/
/*
	//-------------------------------------------------------------------------
	// 계산 앨거리듬:
	// 1. 일단 xy평면(left side에서 바라보는 듯한)에서 x를 1.0(unit)으로 놓고, y는 0으로 놓은 다음, 원을 따라서 각도로 카메라를 들어올린다고 생각하고 계산한다.
	//    이때, xz평면에서, 이제 카메라가 좌우로 돌아간 것을 표현하려면, radius가 더 이상 1이 아니고 cos(높이 각도)가 되어 더 줄어든다.
	// 2. 다시 위에서 바라보는 xz평면에서, x를 줄어든 위에서 나온 cos(높이 각도)으로 놓고, z는 0로 놓은 다음, 다시 좌우로 돌아간 각도로 회전공식을 적용하면 된다.
	// 3. 마지막으로 카메라가 원점인 0,0,0을 바라보게 하면 된다.
	//-------------------------------------------------------------------------
	cameraTarget.x = 5.0f;
	cameraTarget.y = 0.0f;
	cameraTarget.z = 0.0f;
	//float xOnCameraPanningPlane = 0.0f; // chai:ver0049: 카메라가 패닝할 때, x축으로 움직이는 범위. 좌는 마이너스, 우는 플러스.
	//cameraTarget.z = -5.0f; // 만약 (x:0, z:5)를 중심으로 돌려보고 싶을 때..(현재는 x:0, y:0, z:0) 카메라 시작 위치는(x:0, y:2.0, z:-10.0), xz plane에서 270도에서 시작해서 반 시계 방향으로 돈다. 270에서 0.001씩 증가.
	float curCameraDegreeOnXYPlaneRadian = curCameraDegreeOnXYPlane * XM_PI / 180.0f;
	float curRadiusOfCircleOnXZPlane = (1.0f - cameraTarget.x)*cos(curCameraDegreeOnXYPlaneRadian) - (0.0f - cameraTarget.y)*sin(curCameraDegreeOnXYPlaneRadian) + cameraTarget.x;
	float curCameraPostionY = (1.0f - cameraTarget.x)*sin(curCameraDegreeOnXYPlaneRadian) + (0.0f - cameraTarget.y)*cos(curCameraDegreeOnXYPlaneRadian) + cameraTarget.y;

	float curCameraDegreeOnXZPlaneRadian = curCameraDegreeOnXZPlane * XM_PI / 180.0f;
	////float curCameraPostionX = (curRadiusOfCircleOnXZPlane - cameraTarget.x)*cos(curCameraDegreeOnXZPlaneRadian) - (0.0f - cameraTarget.z)*sin(curCameraDegreeOnXZPlaneRadian) + cameraTarget.x;
	////float curCameraPostionZ = (curRadiusOfCircleOnXZPlane - cameraTarget.x)*sin(curCameraDegreeOnXZPlaneRadian) + (0.0f - cameraTarget.z)*cos(curCameraDegreeOnXZPlaneRadian) + cameraTarget.z;
	float curCameraPostionX = (curRadiusOfCircleOnXZPlane - cameraTarget.x)*cos(curCameraDegreeOnXZPlaneRadian) - ((0.0f + xOnCameraPanningPlane) - cameraTarget.z)*sin(curCameraDegreeOnXZPlaneRadian) + cameraTarget.x;
	float curCameraPostionZ = (curRadiusOfCircleOnXZPlane - cameraTarget.x)*sin(curCameraDegreeOnXZPlaneRadian) + ((0.0f + xOnCameraPanningPlane) - cameraTarget.z)*cos(curCameraDegreeOnXZPlaneRadian) + cameraTarget.z;

	////tempZ += 0.001f;
	////xOnCameraPanningPlane += 0.0001f;
	////if (xOnCameraPanningPlane > (2.0f / curCameraDistanceFromViewObject)) {
	////	xOnCameraPanningPlane = 2.0f;
	////}

	//curCameraPostionX *= curCameraDistanceFromViewObject;
	//curCameraPostionY *= curCameraDistanceFromViewObject;
	//curCameraPostionZ *= curCameraDistanceFromViewObject;

	//float curCameraTargetPostionX = (0.0f - cameraTarget.x)*cos(curCameraDegreeOnXZPlaneRadian) - ((0.0f + xOnCameraPanningPlane) - cameraTarget.z)*sin(curCameraDegreeOnXZPlaneRadian) + cameraTarget.x;
	//float curCameraTargetPostionZ = (0.0f - cameraTarget.x)*sin(curCameraDegreeOnXZPlaneRadian) + ((0.0f + xOnCameraPanningPlane) - cameraTarget.z)*cos(curCameraDegreeOnXZPlaneRadian) + cameraTarget.z;
	////cameraTarget.x = curCameraTargetPostionX * curCameraDistanceFromViewObject;
	////cameraTarget.z = curCameraTargetPostionZ * curCameraDistanceFromViewObject;
	//cameraTarget.x = curCameraTargetPostionX * 0.5f;
	//cameraTarget.z = curCameraTargetPostionZ * 0.5f;

	//cameraPosition = XMFLOAT4(curCameraPostionX, curCameraPostionY, curCameraPostionZ, cameraPosition.w); // camera position
	//cameraTarget = XMFLOAT4(cameraTarget.x, cameraTarget.y, cameraTarget.z, cameraTarget.w); // camera look at vector
	//cameraUp = XMFLOAT4(cameraUp.x, cameraUp.y, cameraUp.z, cameraUp.w); // camera up(rotation)
	
	//float curCameraDegreeOnXYPlaneRadian = curCameraDegreeOnXYPlane * XM_PI / 180.0f;
	//float curRadiusOfCircleOnXZPlane = (1.0f)*cos(curCameraDegreeOnXYPlaneRadian) - (0.0f)*sin(curCameraDegreeOnXYPlaneRadian);
	//float curCameraPostionY = (1.0f)*sin(curCameraDegreeOnXYPlaneRadian) + (0.0f)*cos(curCameraDegreeOnXYPlaneRadian);

	//float curCameraDegreeOnXZPlaneRadian = curCameraDegreeOnXZPlane * XM_PI / 180.0f;
	////float curCameraPostionX = (curRadiusOfCircleOnXZPlane)*cos(curCameraDegreeOnXZPlaneRadian) - (0.0f)*sin(curCameraDegreeOnXZPlaneRadian);
	////float curCameraPostionZ = (curRadiusOfCircleOnXZPlane)*sin(curCameraDegreeOnXZPlaneRadian) + (0.0f)*cos(curCameraDegreeOnXZPlaneRadian);
	////float curCameraPostionX = (curRadiusOfCircleOnXZPlane)*cos(curCameraDegreeOnXZPlaneRadian) - ((0.0f + xOnCameraPanningPlane))*sin(curCameraDegreeOnXZPlaneRadian);
	////float curCameraPostionZ = (curRadiusOfCircleOnXZPlane)*sin(curCameraDegreeOnXZPlaneRadian) + ((0.0f + xOnCameraPanningPlane))*cos(curCameraDegreeOnXZPlaneRadian);
	//float curCameraTargetPostionX = (0.0f - cameraTarget.x)*cos(curCameraDegreeOnXZPlaneRadian) - ((0.0f + xOnCameraPanningPlane) - cameraTarget.z)*sin(curCameraDegreeOnXZPlaneRadian) + cameraTarget.x;
	//float curCameraTargetPostionZ = (0.0f - cameraTarget.x)*sin(curCameraDegreeOnXZPlaneRadian) + ((0.0f + xOnCameraPanningPlane) - cameraTarget.z)*cos(curCameraDegreeOnXZPlaneRadian) + cameraTarget.z;

	//tempZ += 0.001f;
	//xOnCameraPanningPlane += 0.0001f;
	//if (xOnCameraPanningPlane > (2.0f / curCameraDistanceFromViewObject)) {
	//	xOnCameraPanningPlane = 2.0f;
	//}

	curCameraPostionX *= curCameraDistanceFromViewObject;
	curCameraPostionY *= curCameraDistanceFromViewObject;
	curCameraPostionZ *= curCameraDistanceFromViewObject;

	float curCameraTargetPostionX = (0.0f - cameraTarget.x)*cos(curCameraDegreeOnXZPlaneRadian) - ((0.0f + xOnCameraPanningPlane) - cameraTarget.z)*sin(curCameraDegreeOnXZPlaneRadian) + cameraTarget.x;
	float curCameraTargetPostionZ = (0.0f - cameraTarget.x)*sin(curCameraDegreeOnXZPlaneRadian) + ((0.0f + xOnCameraPanningPlane) - cameraTarget.z)*cos(curCameraDegreeOnXZPlaneRadian) + cameraTarget.z;
	////cameraTarget.x = curCameraTargetPostionX * curCameraDistanceFromViewObject;
	////cameraTarget.z = curCameraTargetPostionZ * curCameraDistanceFromViewObject;
	//cameraTarget.x = curCameraTargetPostionX * 0.1f;
	//cameraTarget.z = curCameraTargetPostionZ * 0.1f;
	//cameraTarget.x += cameraPosition.x - curCameraPostionX;
	//cameraTarget.z += cameraPosition.z - curCameraPostionZ;
	//float curCameraTargetPostionX = (0.0f)*cos(curCameraDegreeOnXZPlaneRadian) - ((0.0f + xOnCameraPanningPlane))*sin(curCameraDegreeOnXZPlaneRadian);
	//float curCameraTargetPostionZ = (0.0f)*sin(curCameraDegreeOnXZPlaneRadian) + ((0.0f + xOnCameraPanningPlane))*cos(curCameraDegreeOnXZPlaneRadian);
	//cameraTarget.x = curCameraTargetPostionX * curCameraDistanceFromViewObject;
	//cameraTarget.z = curCameraTargetPostionZ * curCameraDistanceFromViewObject;

	cameraPosition = XMFLOAT4(curCameraPostionX, curCameraPostionY, curCameraPostionZ, cameraPosition.w); // camera position
	cameraTarget = XMFLOAT4(cameraTarget.x, cameraTarget.y, cameraTarget.z, cameraTarget.w); // camera look at vector
	cameraUp = XMFLOAT4(cameraUp.x, cameraUp.y, cameraUp.z, cameraUp.w); // camera up(rotation)

	// rotate camera
	//curCameraDegreeOnXZPlane += 0.003f; // camera move때 이미 위에서 만들면서 이것을 지움.
	//-------------------------------------------------------------------------

	// build view matrix
	XMVECTOR cPos = XMLoadFloat4(&cameraPosition);
	XMVECTOR cTarg = XMLoadFloat4(&cameraTarget);
	XMVECTOR cUp = XMLoadFloat4(&cameraUp);
	tmpMat = XMMatrixLookAtLH(cPos, cTarg, cUp);
	XMStoreFloat4x4(&cameraViewMat, tmpMat);
*/
	//----------------------------------------------------------------------------
	// create camera's world matrix for ui objects(tiny axis, ui panel, ...)
	//----------------------------------------------------------------------------
	//XMMATRIX rotXMat = XMMatrixRotationX(0.001f);
	//XMMATRIX rotYMat = XMMatrixRotationY(0.000f);
	//XMMATRIX rotZMat = XMMatrixRotationZ(0.000f);

	//// add rotation to grid's rotation matrix and store it
	////rotMat = XMLoadFloat4x4(&gridRotMat) * rotXMat * rotYMat * rotZMat;
	//XMMATRIX rotMat = rotXMat * rotYMat * rotZMat;
	//XMStoreFloat4x4(&gridRotMat, rotMat);

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
	objModels.at(2).pos = XMFLOAT4(-0.04f, -0.03f, 0.1f, 0.0f);
	//objModels.at(3).worldMat = cameraTranslationMat;

	for (int i = 0; i < 3; i++) {
		//XMMATRIX rotXMat = XMMatrixRotationX(0.000f);
		//XMMATRIX rotYMat = XMMatrixRotationY(0.000f);
		//XMMATRIX rotZMat = XMMatrixRotationZ(0.000f);

		//// add rotation to grid's rotation matrix and store it
		//XMMATRIX rotMat = rotXMat * rotYMat * rotZMat;

		//// create translation matrix for grid from grid's position vector

		//XMMATRIX translationMat = XMMatrixTranslationFromVector(XMLoadFloat4(&objModels.at(0).pos));

		//// create grid's world matrix by first rotating the cube, then positioning the rotated cube
		//XMMATRIX worldMat = rotMat * translationMat;

		// store grid's world matrix

		objModels.at(i).updateMatrices();

		// update constant buffer for grid
		// create the wvp matrix and store in constant buffer
		XMMATRIX wvpMat = XMLoadFloat4x4(&objModels.at(i).worldMat) * viewMat * projMat; // create wvp matrix
		XMMATRIX transposed = XMMatrixTranspose(wvpMat); // must transpose wvp matrix for the gpu
		XMStoreFloat4x4(&cbPerObject.wvpMat, transposed); // store transposed wvp matrix in constant buffer

		memcpy(cbvGPUAddress[frameIndex] + ConstantBufferPerObjectAlignedSize*i, &cbPerObject, sizeof(cbPerObject));
		//????????????????????????????????????????????????????????????????????????
		//memcpy((void *)(vertexBufferViews[i].BufferLocation), &(objModels.at(i).finalVertices[0]), sizeof(Vertex) * objModels.at(i).finalVertices.size());
	}
	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------
	// now do cube1's world matrix
	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------
	
	// create rotation matrices
	//rotXMat = XMMatrixRotationX(0.0001f);
	//rotYMat = XMMatrixRotationY(0.0002f);
	////rotZMat = XMMatrixRotationZ(0.0003f);
	//rotXMat = XMMatrixRotationX(0.0000f);
	//rotYMat = XMMatrixRotationY(0.0000f);
	//rotZMat = XMMatrixRotationZ(0.0000f);

	//// add rotation to cube1's rotation matrix and store it
	//rotMat = XMLoadFloat4x4(&cube1RotMat) * rotXMat * rotYMat * rotZMat;
	//XMStoreFloat4x4(&cube1RotMat, rotMat);

	//// create translation matrix for cube 1 from cube 1's position vector
	//translationMat = XMMatrixTranslationFromVector(XMLoadFloat4(&cube1Position));

	//// create cube1's world matrix by first rotating the cube, then positioning the rotated cube
	//worldMat = rotMat * translationMat;

	//// store cube1's world matrix
	//XMStoreFloat4x4(&cube1WorldMat, worldMat);

	//// update constant buffer for cube1
	//// create the wvp matrix and store in constant buffer

	//// create the wvp matrix and store in constant buffer
	//wvpMat = XMLoadFloat4x4(&cube1WorldMat) * viewMat * projMat; // create wvp matrix
	//transposed = XMMatrixTranspose(wvpMat); // must transpose wvp matrix for the gpu
	////transposed = XMMatrixTranspose(rayWorldMatrix); // must transpose wvp matrix for the gpu
	//
	//XMStoreFloat4x4(&cbPerObject.wvpMat, transposed); // store transposed wvp matrix in constant buffer

	//// copy our ConstantBuffer instance to the mapped constant buffer resource
	//memcpy(cbvGPUAddress[frameIndex] + ConstantBufferPerObjectAlignedSize, &cbPerObject, sizeof(cbPerObject));

	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------
	// now do ui axis model's world matrix(Small UI Axis)
	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------

	// create rotation matrices for ui axis model
	XMMATRIX uiAxisRotXMat = XMMatrixRotationX(0.0000f);
	XMMATRIX uiAxisRotYMat = XMMatrixRotationY(0.0000f);
	XMMATRIX uiAxisRotZMat = XMMatrixRotationZ(0.0000f);

	// add rotation to ui axis model's rotation matrix and store it
	XMMATRIX uiAxisRotMat = uiAxisRotZMat * (XMLoadFloat4x4(&cube2RotMat) * (uiAxisRotXMat * uiAxisRotYMat));
	XMStoreFloat4x4(&cube2RotMat, uiAxisRotMat);

	//-----------------------------------------------------------------------------------------------------
	// 일단 동작하는 코드
	//-----------------------------------------------------------------------------------------------------
	// BzTut11에서 Text PSO처럼, 아래만 한다고 되는 것이 아니라, 화면 ui용을 따로 만들고, 그것을 꺼야 한다.
	//commandList->ClearDepthStencilView(dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	//commandList->ClearDepthStencilView(dsDescriptorHeaps[2]->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// x, y는 UI Panel을 2D로 봤을때의 위치이고, z는 카메라로부터 UI Panel이 떨어진 거리이다.
	cube2PositionOffset = XMFLOAT4(-0.04f, -0.03f, 0.1f, 0.0f); // 화면 좌하단(Blender처럼)
	//cube2PositionOffset = XMFLOAT4(0.04f, 0.03f, 0.1f, 0.0f); // 화면 우상단(Unity처럼)
	//cube2PositionOffset = XMFLOAT4(0.0f, 0.0f, 1.0f, 0.0f); // 화면 좌하단(Blender처럼)

	XMMATRIX uiAxisTranslationOffsetMat = XMMatrixTranslationFromVector(XMLoadFloat4(&cube2PositionOffset));

	XMMATRIX uiAxisScaleMat = XMMatrixScaling(0.01f, 0.01f, 0.01f);
	//XMMATRIX uiAxisScaleMat = XMMatrixScaling(1.0f, 1.0f, 1.0f);

	// 아래의 순서를 잘 지킬 것.
	//	worldMat = scaleMat * cameraTranslationMat * viewMat * translationOffsetMat;

	// 1. scale조정부터 한다: uiAxisScaleMat
	// 2. uiAxis를 camera space안으로 가져온 후: cameraTranslationMat * viewMat
	// 3. camera space의 중앙을 원점으로 하여 이동: uiAxisTranslationOffsetMat
	//    * 이때 회전은 하지 않았으므로, original world space에서 향했던 방향은 그대로 유지.
	XMMATRIX uiAxisWorldMat = uiAxisScaleMat * cameraTranslationMat * viewMat * uiAxisTranslationOffsetMat;

	//// store cube2's world matrix
	XMStoreFloat4x4(&cube2WorldMat, uiAxisWorldMat);

	XMMATRIX uiProjMat = XMLoadFloat4x4(&uiCameraProjMat); // load projection matrix

	//	wvpMat = XMLoadFloat4x4(&cube2WorldMat) * uiProjMat; // create wvp matrix
	XMMATRIX uiAxisWvpMat = XMLoadFloat4x4(&cube2WorldMat) * uiProjMat; // create wvp matrix

	//-----------------------------------------------------------------------------------------------------
	// 원본 cube2가 cube1 근처를 도는 코드는 아래처럼 되어 있었음.
	//-----------------------------------------------------------------------------------------------------
	//cube2PositionOffset = XMFLOAT4(-0.04f, -0.03f, 0.1f, 0.0f); // 화면 좌하단(Blender처럼)

	//XMMATRIX uiAxisTranslationOffsetMat = XMMatrixTranslationFromVector(XMLoadFloat4(&cube2PositionOffset));

	//XMMATRIX uiAxisScaleMat = XMMatrixScaling(1.0f, 1.0f, 1.0f);

	//XMMATRIX uiAxisWorldMat = uiAxisScaleMat * uiAxisTranslationOffsetMat * viewMat * cameraTranslationMat;
	////// store cube2's world matrix
	//XMStoreFloat4x4(&cube2WorldMat, uiAxisWorldMat);

	//XMMATRIX uiProjMat = XMLoadFloat4x4(&uiCameraProjMat); // load projection matrix

	//XMMATRIX uiAxisEvpMat = XMLoadFloat4x4(&cube2WorldMat) * viewMat * uiProjMat; // create wvp matrix
	//-----------------------------------------------------------------------------------------------------

	XMMATRIX uiAxisTransposed = XMMatrixTranspose(uiAxisWvpMat); // must transpose wvp matrix for the gpu
	XMStoreFloat4x4(&cbPerObject.wvpMat, uiAxisTransposed); // store transposed wvp matrix in constant buffer

															// copy our ConstantBuffer instance to the mapped constant buffer resource
	memcpy(cbvGPUAddress[frameIndex] + ConstantBufferPerObjectAlignedSize * 2, &cbPerObject, sizeof(cbPerObject));

//	//-------------------------------------------------------------------------
//	//-------------------------------------------------------------------------
//	//-------------------------------------------------------------------------
//	// now do ui axis model's world matrix(Small UI Axis)
//	//-------------------------------------------------------------------------
//	//-------------------------------------------------------------------------
//	//-------------------------------------------------------------------------
//
//	// create rotation matrices for ui axis model
//	rotXMat = XMMatrixRotationX(0.0000f);
//	rotYMat = XMMatrixRotationY(0.0000f);
//	rotZMat = XMMatrixRotationZ(0.0000f);
//
//	// add rotation to ui axis model's rotation matrix and store it
//	rotMat = rotZMat * (XMLoadFloat4x4(&cube2RotMat) * (rotXMat * rotYMat));
//	XMStoreFloat4x4(&cube2RotMat, rotMat);
//
//	// x, y는 UI Panel을 2D로 봤을때의 위치이고, z는 카메라로부터 UI Panel이 떨어진 거리이다.
//	//cube2PositionOffset = XMFLOAT4(-0.04f, -0.03f, 0.1f, 0.0f); // 화면 좌하단(Blender처럼)
//	//cube2PositionOffset = XMFLOAT4(0.04f, 0.03f, 0.1f, 0.0f); // 화면 우상단(Unity처럼)
//	cube2PositionOffset = XMFLOAT4(0.0f, 0.0f, 1.0f, 0.0f); // 화면 좌하단(Blender처럼)
//
//	XMMATRIX translationOffsetMat = XMMatrixTranslationFromVector(XMLoadFloat4(&cube2PositionOffset));
//
////	XMMATRIX scaleMat = XMMatrixScaling(0.01f, 0.01f, 0.01f);
//	XMMATRIX scaleMat = XMMatrixScaling(1.0f, 1.0f, 1.0f);
//
//	// 아래의 순서를 잘 지킬 것.
////	worldMat = scaleMat * cameraTranslationMat * viewMat * translationOffsetMat;
//	worldMat = scaleMat * cameraTranslationMat * viewMat * translationOffsetMat;
//
//	//// store cube2's world matrix
//	XMStoreFloat4x4(&cube2WorldMat, worldMat);
//
//	XMMATRIX uiProjMat = XMLoadFloat4x4(&uiCameraProjMat); // load projection matrix
//
////	wvpMat = XMLoadFloat4x4(&cube2WorldMat) * uiProjMat; // create wvp matrix
//	wvpMat = XMLoadFloat4x4(&cube2WorldMat) * uiProjMat; // create wvp matrix
//
//	transposed = XMMatrixTranspose(wvpMat); // must transpose wvp matrix for the gpu
//	XMStoreFloat4x4(&cbPerObject.wvpMat, transposed); // store transposed wvp matrix in constant buffer
//
//	// copy our ConstantBuffer instance to the mapped constant buffer resource
//	memcpy(cbvGPUAddress[frameIndex] + ConstantBufferPerObjectAlignedSize, &cbPerObject, sizeof(cbPerObject));

	//if (prevCameraX == cameraPosition.x && prevCameraY == cameraPosition.y && prevCameraZ == cameraPosition.z && prevCameraTargetX == cameraTarget.x && prevCameraTargetY == cameraTarget.y && prevCameraTargetZ == cameraTarget.z) {
	//	return;
	//}

	bool changed[6] = { 0 };
	float tempArray[6];

	//if (!(prevCameraX - 0.0001f <= cameraPosition.x && prevCameraX + 0.001f >= cameraPosition.x)) {
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
	//tempArray[0] = cameraPosition.x;
	//tempArray[1] = cameraPosition.y;
	//tempArray[2] = cameraPosition.z;
	//tempArray[3] = cameraTarget.x;
	//tempArray[4] = cameraTarget.y;
	//tempArray[5] = cameraTarget.z;

	//???????????????????????????????????????????? 나중에 global로 만들고, text box들이 처음 만들어진 직후, 한번만 넣을 것.
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
}

void UpdatePipeline(int objIndex)
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

	// here we start recording commands into the commandList (which all the commands will be stored in the commandAllocator)

	// transition the "frameIndex" render target from the present state to the render target state so the command list draws to it starting from here
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// here we again get the handle to our current render target view so we can set it as the render target in the output merger stage of the pipeline
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), frameIndex, rtvDescriptorSize);

	// Clear the render target by using the ClearRenderTargetView command
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	// get a handle to the depth/stencil buffer
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(dsDescriptorHeaps[objIndex]->GetCPUDescriptorHandleForHeapStart());

	// set the render target for the output merger stage (the output of the pipeline)
	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	// clear the depth/stencil buffer
	commandList->ClearDepthStencilView(dsDescriptorHeaps[objIndex]->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// set root signature
	commandList->SetGraphicsRootSignature(rootSignature); // set the root signature

														  // set the descriptor heap
														  // store vertex buffer in upload heap

	//----------------------------------------------------------------------------
	// Update Vertex Buffer
	//----------------------------------------------------------------------------
	objModels.at(2).finalVertices[0].pos.x += 0.0001;

	Vertex *vList = &(objModels.at(2).finalVertices[0]);
	int vBufferSize = sizeof(Vertex) * objModels.at(2).finalVertices.size();

	// create upload heap
	// upload heaps are used to upload data to the GPU. CPU can write to it, GPU can read from it
	// We will upload the vertex buffer using this heap to the default heap
	ID3D12Resource* vBufferUploadHeap;
	hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&CD3DX12_RESOURCE_DESC::Buffer(vBufferSize), // resource description for a buffer
		D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
		nullptr,
		IID_PPV_ARGS(&vBufferUploadHeap));
	if (FAILED(hr))
	{
		Running = false;
		return;
	}
	vBufferUploadHeap->SetName(L"Vertex Buffer Upload Resource Heap");

	D3D12_SUBRESOURCE_DATA vertexData = {};
	vertexData.pData = reinterpret_cast<BYTE*>(vList); // pointer to our vertex array
	vertexData.RowPitch = vBufferSize; // size of all our triangle vertex data
	vertexData.SlicePitch = vBufferSize; // also the size of our triangle vertex data

										 // we are now creating a command with the command list to copy the data from
										 // the upload heap to the default heap
	UpdateSubresources(commandList, vertexBuffers[2], vBufferUploadHeap, 0, 0, 1, &vertexData);

	// transition the vertex buffer data from copy destination state to vertex buffer state
	//commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(vertexBuffers[2], D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
	//----------------------------------------------------------------------------

	//for (int i = 0; i < 2; i++) {
	for (objIndex = 0; objIndex < 3; objIndex++) {
		ID3D12DescriptorHeap* descriptorHeaps[] = { mainDescriptorHeaps[objIndex] };
		commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

		// set the descriptor table to the descriptor heap (parameter 1, as constant buffer root descriptor is parameter index 0)
		commandList->SetGraphicsRootDescriptorTable(1, mainDescriptorHeaps[objIndex]->GetGPUDescriptorHandleForHeapStart());

		// set cube1's constant buffer
		commandList->SetGraphicsRootConstantBufferView(0, constantBufferUploadHeaps[frameIndex]->GetGPUVirtualAddress() + ConstantBufferPerObjectAlignedSize * objIndex);
		commandList->RSSetViewports(1, &viewport); // set the viewports
		commandList->RSSetScissorRects(1, &scissorRect); // set the scissor rects
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // set the primitive topology
		commandList->IASetVertexBuffers(0, 1, &vertexBufferViews[objIndex]); // set the vertex buffer (using the vertex buffer view)
		commandList->IASetIndexBuffer(&indexBufferViews[objIndex]);

		// first cube


		// draw first cube
		commandList->DrawIndexedInstanced(numObjIndices[objIndex], 1, 0, 0, 0);

		// second cube

		// set cube2's constant buffer. You can see we are adding the size of ConstantBufferPerObject to the constant buffer
		// resource heaps address. This is because cube1's constant buffer is stored at the beginning of the resource heap, while
		// cube2's constant buffer data is stored after (256 bits from the start of the heap).
		//commandList->SetGraphicsRootConstantBufferView(0, constantBufferUploadHeaps[frameIndex]->GetGPUVirtualAddress() + ConstantBufferPerObjectAlignedSize);

		// draw second cube
		//commandList->DrawIndexedInstanced(numObjIndices[objIndex], 1, 0, 0, 0);
	}

	// transition the "frameIndex" render target from the render target state to the present state. If the debug layer is enabled, you will receive a
	// warning if present is called on the render target when it's not in the present state
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	hr = commandList->Close();
	if (FAILED(hr))
	{
		Running = false;
	}
}

void Render()
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

void Cleanup()
{
	// wait for the gpu to finish all frames
	for (int i = 0; i < frameBufferCount; ++i)
	{
		frameIndex = i;
		WaitForPreviousFrame();
	}

	// get swapchain out of full screen before exiting
	BOOL fs = false;
	if (swapChain->GetFullscreenState(&fs, NULL))
		swapChain->SetFullscreenState(false, NULL);

	SAFE_RELEASE(device);
	SAFE_RELEASE(swapChain);
	SAFE_RELEASE(commandQueue);
	SAFE_RELEASE(rtvDescriptorHeap);
	SAFE_RELEASE(commandList);

	for (int i = 0; i < frameBufferCount; ++i)
	{
		SAFE_RELEASE(renderTargets[i]);
		SAFE_RELEASE(commandAllocator[i]);
		SAFE_RELEASE(fence[i]);
	};

	SAFE_RELEASE(pipelineStateObject);
	SAFE_RELEASE(rootSignature);

	for (int i = 0; i < numObjModels; i++) {
		SAFE_RELEASE(vertexBuffers.at(i));
	}

	for (int i = 0; i < numObjModels; i++) {
		SAFE_RELEASE(indexBuffers.at(i));
	}
	SAFE_RELEASE(depthStencilBuffer);
	for (int i = 0; i < numObjModels; i++) {
		SAFE_RELEASE(dsDescriptorHeaps[i]);
	}

	for (int i = 0; i < frameBufferCount; ++i)
	{
		SAFE_RELEASE(constantBufferUploadHeaps[i]);
	};
}

void WaitForPreviousFrame()
{
	HRESULT hr;

	// swap the current rtv buffer index so we draw on the correct buffer
	frameIndex = swapChain->GetCurrentBackBufferIndex();

	// if the current fence value is still less than "fenceValue", then we know the GPU has not finished executing
	// the command queue since it has not reached the "commandQueue->Signal(fence, fenceValue)" command
	if (fence[frameIndex]->GetCompletedValue() < fenceValue[frameIndex])
	{
		// we have the fence create an event which is signaled once the fence's current value is "fenceValue"
		hr = fence[frameIndex]->SetEventOnCompletion(fenceValue[frameIndex], fenceEvent);
		if (FAILED(hr))
		{
			Running = false;
		}

		// We will wait until the fence has triggered the event that it's current value has reached "fenceValue". once it's value
		// has reached "fenceValue", we know the command queue has finished executing
		WaitForSingleObject(fenceEvent, INFINITE);
	}

	// increment fenceValue for next frame
	fenceValue[frameIndex]++;
}

// get the dxgi format equivilent of a wic format
DXGI_FORMAT GetDXGIFormatFromWICFormat(WICPixelFormatGUID& wicFormatGUID)
{
	if (wicFormatGUID == GUID_WICPixelFormat128bppRGBAFloat) return DXGI_FORMAT_R32G32B32A32_FLOAT;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppRGBAHalf) return DXGI_FORMAT_R16G16B16A16_FLOAT;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppRGBA) return DXGI_FORMAT_R16G16B16A16_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppRGBA) return DXGI_FORMAT_R8G8B8A8_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppBGRA) return DXGI_FORMAT_B8G8R8A8_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppBGR) return DXGI_FORMAT_B8G8R8X8_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppRGBA1010102XR) return DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM;

	else if (wicFormatGUID == GUID_WICPixelFormat32bppRGBA1010102) return DXGI_FORMAT_R10G10B10A2_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat16bppBGRA5551) return DXGI_FORMAT_B5G5R5A1_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat16bppBGR565) return DXGI_FORMAT_B5G6R5_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppGrayFloat) return DXGI_FORMAT_R32_FLOAT;
	else if (wicFormatGUID == GUID_WICPixelFormat16bppGrayHalf) return DXGI_FORMAT_R16_FLOAT;
	else if (wicFormatGUID == GUID_WICPixelFormat16bppGray) return DXGI_FORMAT_R16_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat8bppGray) return DXGI_FORMAT_R8_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat8bppAlpha) return DXGI_FORMAT_A8_UNORM;

	else return DXGI_FORMAT_UNKNOWN;
}

// get a dxgi compatible wic format from another wic format
WICPixelFormatGUID GetConvertToWICFormat(WICPixelFormatGUID& wicFormatGUID)
{
	if (wicFormatGUID == GUID_WICPixelFormatBlackWhite) return GUID_WICPixelFormat8bppGray;
	else if (wicFormatGUID == GUID_WICPixelFormat1bppIndexed) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat2bppIndexed) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat4bppIndexed) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat8bppIndexed) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat2bppGray) return GUID_WICPixelFormat8bppGray;
	else if (wicFormatGUID == GUID_WICPixelFormat4bppGray) return GUID_WICPixelFormat8bppGray;
	else if (wicFormatGUID == GUID_WICPixelFormat16bppGrayFixedPoint) return GUID_WICPixelFormat16bppGrayHalf;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppGrayFixedPoint) return GUID_WICPixelFormat32bppGrayFloat;
	else if (wicFormatGUID == GUID_WICPixelFormat16bppBGR555) return GUID_WICPixelFormat16bppBGRA5551;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppBGR101010) return GUID_WICPixelFormat32bppRGBA1010102;
	else if (wicFormatGUID == GUID_WICPixelFormat24bppBGR) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat24bppRGB) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppPBGRA) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppPRGBA) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat48bppRGB) return GUID_WICPixelFormat64bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat48bppBGR) return GUID_WICPixelFormat64bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppBGRA) return GUID_WICPixelFormat64bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppPRGBA) return GUID_WICPixelFormat64bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppPBGRA) return GUID_WICPixelFormat64bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat48bppRGBFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf;
	else if (wicFormatGUID == GUID_WICPixelFormat48bppBGRFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppRGBAFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppBGRAFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppRGBFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppRGBHalf) return GUID_WICPixelFormat64bppRGBAHalf;
	else if (wicFormatGUID == GUID_WICPixelFormat48bppRGBHalf) return GUID_WICPixelFormat64bppRGBAHalf;
	else if (wicFormatGUID == GUID_WICPixelFormat128bppPRGBAFloat) return GUID_WICPixelFormat128bppRGBAFloat;
	else if (wicFormatGUID == GUID_WICPixelFormat128bppRGBFloat) return GUID_WICPixelFormat128bppRGBAFloat;
	else if (wicFormatGUID == GUID_WICPixelFormat128bppRGBAFixedPoint) return GUID_WICPixelFormat128bppRGBAFloat;
	else if (wicFormatGUID == GUID_WICPixelFormat128bppRGBFixedPoint) return GUID_WICPixelFormat128bppRGBAFloat;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppRGBE) return GUID_WICPixelFormat128bppRGBAFloat;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppCMYK) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppCMYK) return GUID_WICPixelFormat64bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat40bppCMYKAlpha) return GUID_WICPixelFormat64bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat80bppCMYKAlpha) return GUID_WICPixelFormat64bppRGBA;

#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8) || defined(_WIN7_PLATFORM_UPDATE)
	else if (wicFormatGUID == GUID_WICPixelFormat32bppRGB) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppRGB) return GUID_WICPixelFormat64bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppPRGBAHalf) return GUID_WICPixelFormat64bppRGBAHalf;
#endif

	else return GUID_WICPixelFormatDontCare;
}

// get the number of bits per pixel for a dxgi format
int GetDXGIFormatBitsPerPixel(DXGI_FORMAT& dxgiFormat)
{
	if (dxgiFormat == DXGI_FORMAT_R32G32B32A32_FLOAT) return 128;
	else if (dxgiFormat == DXGI_FORMAT_R16G16B16A16_FLOAT) return 64;
	else if (dxgiFormat == DXGI_FORMAT_R16G16B16A16_UNORM) return 64;
	else if (dxgiFormat == DXGI_FORMAT_R8G8B8A8_UNORM) return 32;
	else if (dxgiFormat == DXGI_FORMAT_B8G8R8A8_UNORM) return 32;
	else if (dxgiFormat == DXGI_FORMAT_B8G8R8X8_UNORM) return 32;
	else if (dxgiFormat == DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM) return 32;

	else if (dxgiFormat == DXGI_FORMAT_R10G10B10A2_UNORM) return 32;
	else if (dxgiFormat == DXGI_FORMAT_B5G5R5A1_UNORM) return 16;
	else if (dxgiFormat == DXGI_FORMAT_B5G6R5_UNORM) return 16;
	else if (dxgiFormat == DXGI_FORMAT_R32_FLOAT) return 32;
	else if (dxgiFormat == DXGI_FORMAT_R16_FLOAT) return 16;
	else if (dxgiFormat == DXGI_FORMAT_R16_UNORM) return 16;
	else if (dxgiFormat == DXGI_FORMAT_R8_UNORM) return 8;
	else if (dxgiFormat == DXGI_FORMAT_A8_UNORM) return 8;
}

// load and decode image from file
int LoadImageDataFromFile(BYTE** imageData, D3D12_RESOURCE_DESC& resourceDescription, LPCWSTR filename, int &bytesPerRow)
{
	HRESULT hr;

	// we only need one instance of the imaging factory to create decoders and frames
	static IWICImagingFactory *wicFactory = NULL;

	// reset decoder, frame, and converter, since these will be different for each image we load
	IWICBitmapDecoder *wicDecoder = NULL;
	IWICBitmapFrameDecode *wicFrame = NULL;
	IWICFormatConverter *wicConverter = NULL;

	bool imageConverted = false;

	if (wicFactory == NULL)
	{
		// Initialize the COM library
		CoInitialize(NULL);

		// create the WIC factory
		hr = CoCreateInstance(
			CLSID_WICImagingFactory,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&wicFactory)
			);
		if (FAILED(hr)) return 0;

		//========================================================================================================================
		// from here
		//========================================================================================================================

		// hr = wicFactory->CreateFormatConverter(&wicConverter);
		//if (FAILED(hr)) return 0;
	}
	
	//========================================================================================================================
	// to here
	//========================================================================================================================

	hr = wicFactory->CreateFormatConverter(&wicConverter);
	if (FAILED(hr)) return 0;


	// load a decoder for the image
	hr = wicFactory->CreateDecoderFromFilename(
		filename,                        // Image we want to load in
		NULL,                            // This is a vendor ID, we do not prefer a specific one so set to null
		GENERIC_READ,                    // We want to read from this file
		WICDecodeMetadataCacheOnLoad,    // We will cache the metadata right away, rather than when needed, which might be unknown
		&wicDecoder                      // the wic decoder to be created
		);
	if (FAILED(hr)) return 0;

	// get image from decoder (this will decode the "frame")
	hr = wicDecoder->GetFrame(0, &wicFrame);
	if (FAILED(hr)) return 0;

	// get wic pixel format of image
	WICPixelFormatGUID pixelFormat;
	hr = wicFrame->GetPixelFormat(&pixelFormat);
	if (FAILED(hr)) return 0;

	// get size of image
	UINT textureWidth, textureHeight;
	hr = wicFrame->GetSize(&textureWidth, &textureHeight);
	if (FAILED(hr)) return 0;

	// we are not handling sRGB types in this tutorial, so if you need that support, you'll have to figure
	// out how to implement the support yourself

	// convert wic pixel format to dxgi pixel format
	DXGI_FORMAT dxgiFormat = GetDXGIFormatFromWICFormat(pixelFormat);

	// if the format of the image is not a supported dxgi format, try to convert it
	if (dxgiFormat == DXGI_FORMAT_UNKNOWN)
	{
		// get a dxgi compatible wic format from the current image format
		WICPixelFormatGUID convertToPixelFormat = GetConvertToWICFormat(pixelFormat);

		// return if no dxgi compatible format was found
		if (convertToPixelFormat == GUID_WICPixelFormatDontCare) return 0;

		// set the dxgi format
		dxgiFormat = GetDXGIFormatFromWICFormat(convertToPixelFormat);

		// make sure we can convert to the dxgi compatible format
		BOOL canConvert = FALSE;
		hr = wicConverter->CanConvert(pixelFormat, convertToPixelFormat, &canConvert);
		if (FAILED(hr) || !canConvert) return 0;

		// do the conversion (wicConverter will contain the converted image)
		hr = wicConverter->Initialize(wicFrame, convertToPixelFormat, WICBitmapDitherTypeErrorDiffusion, 0, 0, WICBitmapPaletteTypeCustom);
		if (FAILED(hr)) return 0;

		// this is so we know to get the image data from the wicConverter (otherwise we will get from wicFrame)
		imageConverted = true;
	}

	int bitsPerPixel = GetDXGIFormatBitsPerPixel(dxgiFormat); // number of bits per pixel
	bytesPerRow = (textureWidth * bitsPerPixel) / 8; // number of bytes in each row of the image data
	int imageSize = bytesPerRow * textureHeight; // total image size in bytes

	// allocate enough memory for the raw image data, and set imageData to point to that memory
	*imageData = (BYTE*)malloc(imageSize);

	// copy (decoded) raw image data into the newly allocated memory (imageData)
	if (imageConverted)
	{
		// if image format needed to be converted, the wic converter will contain the converted image
		hr = wicConverter->CopyPixels(0, bytesPerRow, imageSize, *imageData);
		if (FAILED(hr)) return 0;
	}
	else
	{
		// no need to convert, just copy data from the wic frame
		hr = wicFrame->CopyPixels(0, bytesPerRow, imageSize, *imageData);
		if (FAILED(hr)) return 0;
	}

	// now describe the texture with the information we have obtained from the image
	resourceDescription = {};
	resourceDescription.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDescription.Alignment = 0; // may be 0, 4KB, 64KB, or 4MB. 0 will let runtime decide between 64KB and 4MB (4MB for multi-sampled textures)
	resourceDescription.Width = textureWidth; // width of the texture
	resourceDescription.Height = textureHeight; // height of the texture
	resourceDescription.DepthOrArraySize = 1; // if 3d image, depth of 3d image. Otherwise an array of 1D or 2D textures (we only have one image, so we set 1)
	resourceDescription.MipLevels = 1; // Number of mipmaps. We are not generating mipmaps for this texture, so we have only one level
	resourceDescription.Format = dxgiFormat; // This is the dxgi format of the image (format of the pixels)
	resourceDescription.SampleDesc.Count = 1; // This is the number of samples per pixel, we just want 1 sample
	resourceDescription.SampleDesc.Quality = 0; // The quality level of the samples. Higher is better quality, but worse performance
	resourceDescription.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN; // The arrangement of the pixels. Setting to unknown lets the driver choose the most efficient one
	resourceDescription.Flags = D3D12_RESOURCE_FLAG_NONE; // no flags

	// return the size of the image. remember to delete the image once your done with it (in this tutorial once its uploaded to the gpu)
	return imageSize;
}