#pragma once

#include "externals/DirectXTex/DirectXTex.h"
#include <d3d12.h>

#include "wrl.h"
using namespace Microsoft::WRL;

#include "Vector.h"

/// <summary>
/// SpotLight 懐中電灯
/// 
/// <param name=".color">ライト色</param>
/// <param name=".position">ライトの位置</param>
/// <param name=".intensity">ライトの強さ</param>
/// <param name=".direction">ライトの向き</param>
/// <param name=".innerAngle">ライトの</param>
/// <param name=".direction">ライトの向き</param>
/// <param name=".innerAngle">この角度以内なら、ほぼ最大の明るさ</param>
/// <param name=".outerAngle">この角度超えたら、ほぼ０の暗くなる</param>
/// <param name=".distance">光が届く距離</param>
/// <param name=".falloff">距離による減衰</param>
/// 
/// </summary>
struct SpotLightParam {

	Vector4 color{ 255.0f, 255.0f, 255.0f, 255.0f }; // ライト色
	Vector3 position{ 0.0f, 0.0f, 0.0f };             // ライトの位置
	float intensity = 1.0f;                            // ライトの強さ

	Vector3 direction{ 0.0f, -1.0f, 0.0f };           // ライトの向き

	float innerAngle = 15.0f;                           // この角度以内なら最大の明るさ
	float outerAngle = 30.0f;                           // これを超えたら光が0になる

	float distance = 10.0f;                             // 光が届く距離
	float falloff = 1.0f;                               // 距離による減衰

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

