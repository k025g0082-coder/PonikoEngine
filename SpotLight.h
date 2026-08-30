#pragma once

#include "externals/DirectXTex/DirectXTex.h"
#include <d3d12.h>

#include "wrl.h"
using namespace Microsoft::WRL;

#include "Vector.h"

// 構造体
struct SpotLightParam {

	Vector4 color;      // ライト色
	Vector3 position;   // ライトの位置
	float intensity;    // ライトの強さ

	Vector3 direction;  // ライトの向き
	float angle;        // 光の広がる角度

	float distance;     // 光が届く距離
	float falloff;      // 距離による減衰

};

class SpotLight
{
public:

	// 設定
	void Initialize(SpotLightParam param);

	// 参照w
	SpotLightParam* GetLightData() {
		return lightData_;
	};

	ID3D12Resource* GetResource() { return resourceLight.Get(); }

private:
	SpotLightParam lightParam_;

	SpotLightParam* lightData_ = nullptr;

	ComPtr <ID3D12Resource> resourceLight;
};

