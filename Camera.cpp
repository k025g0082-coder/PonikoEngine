#include "Camera.h"
#include "WinApp.h"

void Camera::Initialize() {

	float startZ = -50.0f;
	worldTransform_.translation_.z = startZ;
	worldTransform_.Initialize();

	// 計算
	UpdateMatrix();

	// 一旦これ固定で
	projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(WinApp::GetInstance()->GetClientWidth()) / float(WinApp::GetInstance()->GetClientHeight()), 0.1f, farZ);
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

Camera::Ray Camera::ScreenPointToRay(Vector2 point, float width, float height) {

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

bool Camera::IntersectRayAABB(
	const Ray& ray,
	const AABB& aabb,
	float& distance)
{
	float tMin = 0.0f;
	float tMax = FLT_MAX;

	const float origin[3] = {
		ray.origin.x, ray.origin.y, ray.origin.z
	};
	const float dir[3] = {
		ray.direction.x, ray.direction.y, ray.direction.z
	};
	const float boxMin[3] = {
		aabb.min.x, aabb.min.y, aabb.min.z
	};
	const float boxMax[3] = {
		aabb.max.x, aabb.max.y, aabb.max.z
	};

	for (int i = 0; i < 3; ++i) {
		if (std::abs(dir[i]) < 1.0e-6f) {
			// この軸にほぼ平行
			if (origin[i] < boxMin[i] ||
				origin[i] > boxMax[i]) {
				return false;
			}
		}
		else {
			float t1 = (boxMin[i] - origin[i]) / dir[i];
			float t2 = (boxMax[i] - origin[i]) / dir[i];

			if (t1 > t2) {
				std::swap(t1, t2);
			}

			tMin = (std::max)(tMin, t1);
			tMax = (std::min)(tMax, t2);

			if (tMin > tMax) {
				return false;
			}
		}
	}

	distance = tMin;
	return true;
}

std::optional<Vector3> Camera::IntersectRayPlane(const Camera::Ray& ray, const Plane& plane)
{
	/*
		
		レイトレーシング（光線追跡法）
		直線のベクトル方程式（レイ・放射線の数式）

		P(t) = O + tD

		P(t)　当たった座標
		O   plane原点
		t   原点Oからどれだけ進んだか、距離
		D   Rayの方向。正規化して使う

	*/

	// 平面の法線
	Vector3 normal = Normalize(plane.normal);
	Vector3 rayDirection = Normalize(ray.direction);

	// レイの方向と法線の内積
	float denominator = Dot(rayDirection, normal);

	// 0除算対策
	if (std::abs(denominator) < 0.0001f) {
		return std::nullopt;
	}

	// レイの始点から平面までの距離を求める
	float t = Dot(
		plane.point - ray.origin,
		normal
	) / denominator;

	// レイの後方に交点がある
	if (t < 0.0f) {
		return std::nullopt;
	}

	/* 公式に則って当たった座標を返す */
	return ray.origin + t * rayDirection;
}
