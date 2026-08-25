#include "Camera.h"
#include "WinApp.h"

void Camera::Initialize() {

	float startZ = -50.0f;
	worldTransform_.translation_.z = startZ;
	worldTransform_.Initialize();

	// 一旦これ固定で
	projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(WinApp::GetInstance()->GetClientWidth()) / float(WinApp::GetInstance()->GetClientHeight()), 0.1f, farZ);
};

void Camera::Update() {
	
	// 計算
	UpdateMatrix();

};

void Camera::UpdateMatrix() {
	
	Matrix4x4 worldMatrix = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	
	viewMatrix = Inverse(worldMatrix);
};

void Camera::SetCamera(Camera camera) {
	viewMatrix = camera.GetViewMatrix();
	projectionMatrix = camera.GetProjectionMatrix();
}
