#pragma once

#include "externals/DirectXTex/DirectXTex.h"
#include <d3d12.h>

#include "wrl.h"
using namespace Microsoft::WRL;

#include "Vector.h"

	/// <summary>
	/// pointLight 太陽みたいなやつ
	/// 
	/// <param name=".color">ライト色</param>
	/// <param name=".position">座標</param>
	/// <param name=".intensity">ライトの強さ</param>
	/// <param name=".radius">光が届く距離</param>
	/// 
	/// </summary>
struct PointLightParam {

	Vector4 color{ 1.0f, 1.0f, 1.0f, 1.0f }; // ライト色
	Vector3 position{ 0.0f, 0.0f, 0.0f };             // ライトの位置
	float intensity = 1.0f;                            // ライトの強さ

	float radius = 10.0f;                              // 光が届く範囲
	float padding[3];

};

class PointLight
{
public:

	// 設定
	void Initialize(PointLightParam param);

	// 参照w
	PointLightParam* GetLightData() {
		return lightData_;
	};

	ID3D12Resource* GetResource() { return resourceLight.Get(); }

private:
	PointLightParam lightParam_;

	PointLightParam* lightData_ = nullptr;

	ComPtr <ID3D12Resource> resourceLight;
};

