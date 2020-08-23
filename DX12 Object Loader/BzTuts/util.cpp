#include "stdafx.h"

#include "util.h"

#include "Shlwapi.h"
#pragma comment(lib, "shlwapi.lib")

inline std::string GetExeDir() {
	char path[MAX_PATH] = "";
	GetModuleFileNameA(NULL, path, MAX_PATH);
	PathRemoveFileSpecA(path);
	PathAddBackslashA(path);
	return path;
}

std::string GetWorkingDir() {
	char path[MAX_PATH] = "";
	GetCurrentDirectoryA(MAX_PATH, path);
	PathAddBackslashA(path);
	return path;
}

void msgBoxCstring(char *s) {
	size_t len = strlen(s);
	WCHAR unistring[280 + 1];
	int result = MultiByteToWideChar(CP_OEMCP, 0, s, -1, unistring, len + 1);

	MessageBox(NULL, unistring, L"msgBoxCstring()", MB_OK);
}

void ComputeQuatW(XMFLOAT4 &quat) {
	float t = 1.0f - (quat.x * quat.x) - (quat.y * quat.y) - (quat.z * quat.z);
	if (t < 0.0f)
	{
		quat.w = 0.0f;
	}
	else
	{
		quat.w = -sqrtf(t);
	}
}

XMFLOAT3 rotateVectorByQuat(XMFLOAT4 q, XMFLOAT3 v) {
	XMFLOAT4 qv = XMFLOAT4(
		q.w * v.x + q.y * v.z - q.z * v.y,
		q.w * v.y + q.z * v.x - q.x * v.z,
		q.w * v.z + q.x * v.y - q.y * v.x,
		-q.x * v.x - q.y * v.y - q.z * v.z
	);

	return XMFLOAT3(
		qv.w * -q.x + qv.x * q.w + qv.y * -q.z - qv.z * -q.y,
		qv.w * -q.y + qv.y * q.w + qv.z * -q.x - qv.x * -q.z,
		qv.w * -q.z + qv.z * q.w + qv.x * -q.y - qv.y * -q.x
	);
}

XMFLOAT4 quatMult(XMFLOAT4 q1, XMFLOAT4 q2) {
	return {
		q1.x * q2.w + q1.y * q2.z - q1.z * q2.y + q1.w * q2.x,
		-q1.x * q2.z + q1.y * q2.w + q1.z * q2.x + q1.w * q2.y,
		q1.x * q2.y - q1.y * q2.x + q1.z * q2.w + q1.w * q2.z,
		-q1.x * q2.x - q1.y * q2.y - q1.z * q2.z + q1.w * q2.w,
	};
}

XMFLOAT4 transformXMFLOAT4(XMFLOAT4 v, XMMATRIX m[], int size) {
	for (int i = 0; i < size; i++) {
		XMStoreFloat4(&v, XMVector4Transform(XMLoadFloat4(&v), m[i]));
	}
	return v;
}

// sets the w value to 1.0f so future XMVECTOR transformations will work
XMFLOAT4 XMFLOAT3toXMFLOAT4(XMFLOAT3 v) {
	return XMFLOAT4(v.x, v.y, v.z, 1.0f);
}

// cuts of the w value
XMFLOAT3 XMFLOAT4toXMFLOAT3(XMFLOAT4 v) {
	return XMFLOAT3(v.x, v.y, v.z);
}
