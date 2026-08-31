#pragma once
#include "Matrix.h"
#include "WorldTransform.h"

class Camera
{
public:
	void Initialize();
	void Update();

	// Srt
	Vector3& GetWorldPosition() {return worldTransform_.translation_;}
	Vector3& GetRotation() { return worldTransform_.rotation_; }

	// 読み取り専用
	const WorldTransform& GetWorldTransform() const
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

private:

	// 初期カメラ位置
	WorldTransform worldTransform_;

	// ビュー
	Matrix4x4 viewMatrix;

	// プロジェクション
	Matrix4x4 projectionMatrix;

};

