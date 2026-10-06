#pragma once
#include <d3d12.h>

#include "Vector.h"
#include "Matrix.h"

#include "AABB.h"

#include "Camera.h"

#include <vector>

#include <wrl.h>

struct DebugLineVertex {
	Vector3 position;
	Vector4 color;
};

class DebugLine
{
public:

	// シングルトン
	static DebugLine* GetInstance();

	// 初期化
	void Initialize();

    // 線追加(SceneのDraw内限定)
	static void AddLine(const Vector3& start, const Vector3& end, const Vector4& color);


	// AABB描画(SceneのDraw内限定)
	static void DrawAABB(const AABB& aabb, const Vector4& color);


	void Render();

	// 線描画
	void Draw();

	void Clear();

	// カメラセット
	static void SetCamera(Camera* camera) {
		camera_ = camera;
	}

private:

	static std::vector<DebugLineVertex> vertices_;


	ID3D12Device* device_;

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;

	// b0 カメラのviewProjectionMatrixを送るためのリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> viewProjectionResource_;

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

	// 転送用
	Matrix4x4* viewProjectionData_ = nullptr;

	static Camera* camera_;
};

