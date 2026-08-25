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

private:

	// 初期カメラ位置
	WorldTransform worldTransform_;

	// ビュー
	Matrix4x4 viewMatrix;

	// プロジェクション
	Matrix4x4 projectionMatrix;

};

