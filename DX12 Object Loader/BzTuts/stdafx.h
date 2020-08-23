#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN    // Exclude rarely-used stuff from Windows headers.
#endif

#include <iostream>
#include <fstream>
#include <streambuf>
#include <string>
#include <vector>
#include <tchar.h>
#include <stdio.h>
#include <strsafe.h>

#include <Winsock2.h>
#include <windows.h>
#include <wincodec.h>

#include <windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include "d3dx12.h"
#include <string>

// this will only call release if an object exists (prevents exceptions calling release on non existant objects)
#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = 0; } }

using namespace std;
using namespace DirectX; // we will be using the directxmath library
