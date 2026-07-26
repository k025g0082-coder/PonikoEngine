#include "Camera.h"
#include "WinApp.h"

void Camera::Initialize() {

};

void Camera::Update() {
	
	// tesuto
	//worldTransform_.translate.z -= 0.01f;

	// 計算
	Matrix4x4 worldMatrix = MakeAffineMatrix(worldTransform_.scale, worldTransform_.rotate, worldTransform_.translate);
	viewMatrix = Inverse(worldMatrix);
	projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(WinApp::GetInstance()->GetClientWidth()) / float(WinApp::GetInstance()->GetClientHeight()), 0.1f, 100.0f);

};