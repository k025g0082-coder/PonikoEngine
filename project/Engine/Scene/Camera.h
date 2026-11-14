#pragma once
#include "Matrix.h"
#include "WorldTransform.h"

#include "Coordinate.h"

#include "AABB.h"
#include "Plane.h"

#include "Ray.h"

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

	/*
		投影タイプ
	*/
	enum class ProjectionPlane {
		XY,// 水平
		XZ,// 地面など
		YZ // カメラに対して縦
	};

	/// <summary>
	/// ある座標をもとに前方向を作る
	/// </summary>
	/// <param name="target">ターゲットの座標</param>
	/// <param name="plane">投影タイプ</param>
	/// <returns></returns>
	Vector3 GetForward(Vector3 target, ProjectionPlane plane = ProjectionPlane::XY) const;


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
	/// <param name="targetPos">ターゲット座標</param>
	/// <param name="Follow">Followでゆっくり補間時間</param>
	void SphericalCameraUpdate(Vector3 targetPos,float interpolation = 0.2f);

	void SphericalImGuiView();

	Spherical& GetSpherical() { return targetSpherical; }


	/* 
		ワールド座標 → スクリーン座標
	*/
	Vector2 WorldToScreen(const Vector3& worldPosition) const;


	/*
		縦横の大きさからRayをつくる
	*/
	Ray ScreenPointToRay(Vector2 point, float width, float height);


private:

	// 初期カメラ位置
	WorldTransform worldTransform_;

	// ビュー
	Matrix4x4 viewMatrix;

	// プロジェクション
	Matrix4x4 projectionMatrix;


	// エディターモードか？
	bool isEditorMode_ = false;

	/*
		SphericalをFollow補間
	*/
	void FollowSpherical();


	// 球面座標系。補間対象のターゲット用
	Spherical targetSpherical	{
			6.0f, // どれくらい原点から離れるか
			0,
			-std::numbers::pi_v<float> / 2.0f
	};

	// 補間する方
	Spherical spherical;

	// Follow補完時間
	float followInterpolation_ = 0.0f;
};

