#include "transform.h"
#include "kj_app.h"

extern D3DApp* gd3dApp;

Transform::Transform() : Component() {
	rotMat = XMMatrixRotationX(0.000f) * XMMatrixRotationY(0.000f) * XMMatrixRotationZ(0.000f);
	scaleMat = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	pos = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

	uiObject = false;
	offsetPos = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

}

Transform::~Transform() {
}

XMMATRIX Transform::getWvpMat(XMMATRIX viewMat, XMMATRIX projMat) {
	if (!uiObject) {
		return scaleMat * rotMat * XMMatrixTranslationFromVector(pos) * viewMat * projMat; // create wvp matrix
	}
	else {
		return scaleMat * XMMatrixTranslationFromVector(XMLoadFloat4(&((KjApp *)gd3dApp)->cameraPosition)) *  XMLoadFloat4x4(&((KjApp *)gd3dApp)->cameraViewMat) * XMMatrixTranslationFromVector(offsetPos) * XMLoadFloat4x4(&((KjApp *)gd3dApp)->uiCameraProjMat); // create wvp matrix
	}
	return XMMATRIX();
}