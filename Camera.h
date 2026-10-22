#pragma once
#include "Matrix.h"
#include "WorldTransform.h"

#include "Coordinate.h"

#include "AABB.h"
#include "Plane.h"

#include <numbers>
#include <optional>

class Camera
{
public:
	void Initialize();
	void Update();

	// Srt
	Vector3& GetWorldPosition() {return worldTransform_.translation_;}
	Vector3& GetRotation() { return worldTransform_.rotation_; }

	// 読み取り専用
	WorldTransform& GetWorldTransform()
	{
		return worldTransform_;
	}

	// 設定
	void SetWorldTransform(const WorldTransform& w)
	{
		worldTransform_ = w;
	}


	// ビュー
	Matrix4x4 GetViewMatrix() { return viewMatrix; }
	void SetViewMatrix(Matrix4x4 matWorld) { 
		viewMatrix = matWorld;
	}

	// プロジェクション
	Matrix4x4 GetProjectionMatrix() { return projectionMatrix; }

	void UpdateMatrix();

	// 他のカメラの情報をコピーする
	void SetCamera(Camera camera);


	// エディターモードかをセット
	void SetEditorMode(bool b) { isEditorMode_ = b; }


	// farZ(必ずInitialize前に書く)
	float farZ = 100.0f;


	// カメラの前方向をとる（方向ベクトル）
	Vector3 GetForward() const
	{
		Matrix4x4 worldMatrix =
			MakeAffineMatrix(
				worldTransform_.scale_,
				worldTransform_.rotation_,
				worldTransform_.translation_
			);

		Vector3 forward{
			worldMatrix.m[2][0],
			worldMatrix.m[2][1],
			worldMatrix.m[2][2]
		};

		return Normalize(forward);
	}

	/// <summary>
	/// ある原点中心にカメラ回す. camera.Updateはよばない
	/// </summary>
	/// <param name="pos">原点</param>
	/// <param name="offset">原点からの距離</param>
	/// <param name="theta">横</param>
	/// <param name="phi">縦</param>
	void SphericalCameraUpdate(Vector3 targetPos, float offset, float theta, float phi = -std::numbers::pi_v<float> / 2.0f);

	/// <summary>
	/// ある原点中心にカメラ回す. カメラ内蔵用　camera.Updateはよばない
	/// </summary>
	void SphericalCameraUpdate(Vector3 targetPos);

	void SphericalImGuiView();

	Spherical& GetSpherical() { return spherical; }

	// ワールド座標 → スクリーン座標
	Vector2 WorldToScreen(const Vector3& worldPosition) const;

	struct Ray {
		Vector3 origin;    // レイの始点
		Vector3 direction; // 正規化した方向
	};

	// 縦横の大きさからレイを作る
	Ray ScreenPointToRay(Vector2 point, float width, float height);

	// レイとAABB判定
	bool IntersectRayAABB(
		const Camera::Ray& ray,
		const AABB& aabb,
		float& distance);

	// レイとPlane判定 あたった座標を返す。当たらなかったらstd::nullopt
	std::optional<Vector3> IntersectRayPlane(
		const Camera::Ray& ray,
		const Plane& plane
	);

private:

	// 初期カメラ位置
	WorldTransform worldTransform_;

	// ビュー
	Matrix4x4 viewMatrix;

	// プロジェクション
	Matrix4x4 projectionMatrix;


	// エディターモードか？
	bool isEditorMode_ = false;

	// 球面座標系よう
	Spherical spherical	{
			6.0f, // どれくらい原点から離れるか
			0,
			-std::numbers::pi_v<float> / 2.0f
	};

};

