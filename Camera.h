#pragma once
#include "Matrix.h"
#include "WorldTransform.h"

class Camera
{
public:
	void Initialize();
	void Update();

	// ビュー
	Matrix4x4 GetViewMatrix() { return viewMatrix; }

	// プロジェクション
	Matrix4x4 GetProjectionMatrix() { return projectionMatrix; }

	// ワールドざひょ


private:

	// 初期カメラ位置
	WorldTransform worldTransform_ = {
		.scale = {1.0f,1.0f,1.0f},
		.rotate = {0,0,0},
		.translate = {0,0,-50.0f},
	};

	// ビュー
	Matrix4x4 viewMatrix;

	// プロジェクション
	Matrix4x4 projectionMatrix;

};

