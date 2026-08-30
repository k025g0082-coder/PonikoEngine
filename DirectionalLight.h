#pragma once

#include "externals/DirectXTex/DirectXTex.h"
#include <d3d12.h>

#include "wrl.h"
using namespace Microsoft::WRL;

#include "Vector.h"

/// <summary>
/// DirectionalLight 一定の直線から照らす
/// 
/// <param name=".color">ライト色</param>
/// <param name=".direction">ライトの向き</param>
/// <param name=".intensity">ライトの強さ</param>
/// 
/// </summary>
struct DirectionalLightParam {
	Vector4 color;    //ライト色
	Vector3 direction;//ライトの向き
	float intensity; //ライトの強さ
};

class DirectionalLight
{
public:

	// 設定
	void Initialize(DirectionalLightParam param);

	// 参照w
	DirectionalLightParam* GetLightData() {
		return lightData_;
	};

	ID3D12Resource* GetResource() { return resourceLight.Get(); }

private:
	DirectionalLightParam lightParam_;

	DirectionalLightParam* lightData_ = nullptr;

	ComPtr <ID3D12Resource> resourceLight;
};

