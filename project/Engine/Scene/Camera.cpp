#include "Camera.h"
#include "WinApp.h"

#include "Timer.h"

void Camera::Initialize() {

	float startZ = -50.0f;
	worldTransform_.translation_.z = startZ;
	worldTransform_.Initialize();

	spherical = targetSpherical;

	// 計算
	UpdateMatrix();

	// なるべく大きい値にしとく
	farZ = 1000.0f;

	// 45度をラジアンに変換して指定する場合の例
	float fovAngleY = 45.0f * (3.14159265f / 180.0f); // 約 0.785 rad

	// 一旦これ固定で
	projectionMatrix = MakePerspectiveFovMatrix(fovAngleY, float(WinApp::GetInstance()->GetClientWidth()) / float(WinApp::GetInstance()->GetClientHeight()), 0.1f, farZ);
};

void Camera::Update() {
	
	if (isEditorMode_)return;

	// 計算
	UpdateMatrix();

};

void Camera::UpdateMatrix() {

	if (isEditorMode_)return;

	Matrix4x4 worldMatrix = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	
	viewMatrix = Inverse(worldMatrix);
};

void Camera::SetCamera(Camera camera) {
	viewMatrix = camera.GetViewMatrix();
	projectionMatrix = camera.GetProjectionMatrix();
}

Vector3 Camera::GetForward(Vector3 target, ProjectionPlane plane) const
{
	// カメラ座標からターゲット座標を引き方向を作る
	Vector3 forward = target - worldTransform_.translation_;

	// 指定した平面に投影 - 要らない軸は除外する
	switch (plane)
	{
	case ProjectionPlane::XY:
		forward.z = 0.0f;
		break;

	case ProjectionPlane::XZ:
		forward.y = 0.0f;
		break;

	case ProjectionPlane::YZ:
		forward.x = 0.0f;
		break;
	}

	// 正規化してreturn
	return Normalize(forward);
};

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

void Camera::SphericalCameraUpdate(Vector3 targetPos, float interpolation) {

	followInterpolation_ = interpolation;

	/*
		設定されたSphericalにFollow補間
	*/

	FollowSpherical();


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
	
	worldTransform_.translation_ = eye;

}

void Camera::SphericalImGuiView() {

#ifdef USE_IMGUI
	ImGui::DragFloat("CameraOffset", &targetSpherical.radius, 0.01f);
	ImGui::DragFloat("CameraTheta", &targetSpherical.theta, 0.01f);
	ImGui::DragFloat("CameraPhi", &targetSpherical.phi, 0.01f);
#endif

}

Vector2 Camera::WorldToScreen(const Vector3& worldPosition) const {

	// ワールド座標を同次座標にする
	Vector4 position = {
		worldPosition.x,
		worldPosition.y,
		worldPosition.z,
		1.0f
	};

	// ViewProjection行列
	Matrix4x4 viewProjection =
		Multiply(viewMatrix, projectionMatrix);

	// クリップ空間へ変換
	position = Transform(position, viewProjection);

	// カメラの後ろ、または投影できない位置
	if (position.w <= 0.0f) {
		return { -1.0f, -1.0f };
	}

	// 透視除算 → NDC座標
	float ndcX = position.x / position.w;
	float ndcY = position.y / position.w;

	// NDC → スクリーン座標
	Vector2 screenPosition;

	screenPosition.x = (ndcX + 1.0f) * 0.5f * float(WinApp::GetInstance()->GetClientWidth());
	screenPosition.y = (1.0f - ndcY) * 0.5f * float(WinApp::GetInstance()->GetClientHeight());

	return screenPosition;
}

Ray Camera::ScreenPointToRay(Vector2 point, float width, float height) {

	// NDC座標を求める
	float ndcX = (point.x / width) * 2.0f - 1.0f;
	float ndcY = 1.0f - (point.y / height) * 2.0f;

	// ViewProjectionの逆行列
	Matrix4x4 viewProjection =
		Multiply(viewMatrix, projectionMatrix);

	Matrix4x4 inverseViewProjection = Inverse(viewProjection);

	// DirectXのNDC深度範囲は 0～1
	Vector4 nearPoint = {
		ndcX, ndcY, 0.0f, 1.0f
	};

	Vector4 farPoint = {
		ndcX, ndcY, 1.0f, 1.0f
	};

	nearPoint = Transform(nearPoint, inverseViewProjection);
	farPoint = Transform(farPoint, inverseViewProjection);

	// 透視除算
	Vector3 nearWorld = {
		nearPoint.x / nearPoint.w,
		nearPoint.y / nearPoint.w,
		nearPoint.z / nearPoint.w
	};

	Vector3 farWorld = {
		farPoint.x / farPoint.w,
		farPoint.y / farPoint.w,
		farPoint.z / farPoint.w
	};

	Ray ray;
	ray.origin = nearWorld;
	ray.direction = Normalize(farWorld - nearWorld);

	return ray;
}


void Camera::FollowSpherical()
{
	// 現在の球面座標を目標に近づける

	// 大きさ
	spherical.radius = Follow(
		spherical.radius,
		targetSpherical.radius,
		followInterpolation_,
		Timer::GetDeltaTime()
	);

	// 方位核（横
	spherical.phi = Follow(
		spherical.phi,
		targetSpherical.phi,
		followInterpolation_,
		Timer::GetDeltaTime()
	);

	// 方位核（縦
	spherical.theta = FollowAngle(
		spherical.theta,
		targetSpherical.theta,
		followInterpolation_,
		Timer::GetDeltaTime()
	);

}
