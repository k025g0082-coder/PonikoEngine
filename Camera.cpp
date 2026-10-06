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


void Camera::SphericalCameraUpdate(Vector3 targetPos, float offset, float theta, float phi) {

	// もし０だったら何も映んないので
	if (phi == 0.0f) {
		phi = 0.01f;
	}

	Spherical s{
		offset,
		theta,
		phi
	};

	// カメラの位置
	Vector3 offsett = ToCartesian(s);
	Vector3 eye = targetPos + offsett;

	// カメラの角度
	Vector3 worldUp{ 0.0f, 1.0f, 0.0f };
	Vector3 forward = Normalize(targetPos - eye);
	Vector3 right = Normalize(Cross(worldUp, forward));
	Vector3 up = Cross(forward, right);

	// カメラの行列に合体
	Matrix4x4 cameraMatrix{ {
		{right.x,  right.y, right.z, 0.0f},
		{up.x,     up.y,    up.z   , 0.0f},
		{forward.x,forward.y,forward.z, 0.0f},
		{eye.x,    eye.y,   eye.z,    1.0f},
	} };

	viewMatrix = Inverse(cameraMatrix);
}

void Camera::SphericalCameraUpdate(Vector3 targetPos) {

	// カメラの位置
	Vector3 offsett = ToCartesian(spherical);
	Vector3 eye = targetPos + offsett;

	// カメラの角度
	Vector3 worldUp{ 0.0f, 1.0f, 0.0f };
	Vector3 forward = Normalize(targetPos - eye);
	Vector3 right = Normalize(Cross(worldUp, forward));
	Vector3 up = Cross(forward, right);

	// カメラの行列に合体
	Matrix4x4 cameraMatrix{ {
		{right.x,  right.y, right.z, 0.0f},
		{up.x,     up.y,    up.z   , 0.0f},
		{forward.x,forward.y,forward.z, 0.0f},
		{eye.x,    eye.y,   eye.z,    1.0f},
	} };

	viewMatrix = Inverse(cameraMatrix);

	// 上の行列から回転行列を割り出す
	float rotationY = std::atan2(forward.x, forward.z);
	float rotationX = std::atan2(
		-forward.y,
		std::sqrt(
			forward.x * forward.x +
			forward.z * forward.z
		)
	);

	worldTransform_.rotation_ = { rotationX, rotationY, 0.0f };
	
	/*worldTransform_.translation_ = eye;*/

}

void Camera::SphericalImGuiView() {

#ifdef USE_IMGUI
	ImGui::DragFloat("CameraOffset", &spherical.radius, 0.01f);
	ImGui::DragFloat("CameraTheta", &spherical.theta, 0.01f);
	ImGui::DragFloat("CameraPhi", &spherical.phi, 0.01f);
#endif

}