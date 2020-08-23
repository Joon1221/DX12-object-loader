#pragma once

#include "stdafx.h"

inline std::string GetExeDir();
std::string GetWorkingDir();
void msgBoxCstring(char *s);

void ComputeQuatW(XMFLOAT4 &quat);
XMFLOAT3 rotateVectorByQuat(XMFLOAT4 q, XMFLOAT3 v);
XMFLOAT4 quatMult(XMFLOAT4 q1, XMFLOAT4 q2);

XMFLOAT4 transformXMFLOAT4(XMFLOAT4 v, XMMATRIX m[], int size);

XMFLOAT4 XMFLOAT3toXMFLOAT4(XMFLOAT3 v);
XMFLOAT3 XMFLOAT4toXMFLOAT3(XMFLOAT4 v);