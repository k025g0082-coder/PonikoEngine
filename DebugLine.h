#pragma once
#include <d3d12.h>

#include "Vector.h"
#include "Matrix.h"

#include "Plane.h"
#include "AABB.h"
#include "Sphere.h"

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

	// Plane描画(SceneのDraw内限定)
	static void DrawPlane(const Plane& aabb, const Vector4& color);
	static void DrawPlane(const Plane& aabb,const Vector3& rotation, const Vector4& color);

	// AABB描画(SceneのDraw内限定)
	static void DrawAABB(const AABB& aabb, const Vector4& color);

	/// <summary>
	/// 球描画(SceneのDraw内限定)
	/// </summary>
	/// <param name="sphere">球</param>
	/// <param name="color">球の色</param>
	/// <param name="subdivision">分割数</param>
	
	static void DrawSphere(const Sphere& sphere, const Vector4& color,const int subdivision = 20);


	//　簡単な座標軸を描画 
	static void DrawAxis(const Vector3& Position, float axisLength = 1.0f);


	void Render();

	// 線描画
	void Draw();
	void Draw(Camera camera);

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

	// あるベクトルから９０度おるらしい
	static Vector3 Perpendicular(const Vector3& vector) {
		if (vector.x != 0.0f || vector.y != 0.0f) {
			return { -vector.y, vector.x, 0.0f };
		}
		return { 0.0f, -vector.z, vector.y };
	};

};

