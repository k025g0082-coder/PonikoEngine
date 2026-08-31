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

	Vector4 color{ 1.0f, 1.0f, 1.0f, 1.0f }; // ライト色
	Vector3 position{ 0.0f, 5.0f, 0.0f };             // ライトの位置
	float intensity = 1.0f;                            // ライトの強さ

	Vector3 direction{ 0.0f, -1.0f, 0.0f };           // ライトの向き

	float innerAngle = 15.0f;                           // この角度以内なら最大の明るさ
	float outerAngle = 30.0f;                           // これを超えたら光が0になる

	float distance = 50.0f;                             // 光が届く距離
	float falloff = 1.0f;                               // 距離による減衰
	float padding;										 //  4   ← これを追加して 64 bytes にする
};

struct SpotLightsParam {
	SpotLightParam lights[10];
	int count = 0;
	float padding[3];
};

class SpotLight
{
public:

	// 設定
	void Initialize();

	// 参照w
	SpotLightParam* GetLightData(int index) {
		return &lightDatas_->lights[index];
	};

	// ライト追加 & 参照ゲット
	SpotLightParam& AddLight(SpotLightParam param) {

		if (lightDatas_->count >= kMaxLight)
			return lightDatas_->lights[lightDatas_->count - 1];

		lightDatas_->lights[lightDatas_->count] = param;

		return lightDatas_->lights[lightDatas_->count++];

	}

	// ライトリセット
	void ResetLight()
	{
		for (int i = 0; i < kMaxLight; i++) {

			lightDatas_->lights[i] = {};
		}
	}

	ID3D12Resource* GetResource() { return resourceLight.Get(); }

private:
	// １０こまで
	static const int kMaxLight = 10;

	SpotLightsParam* lightDatas_ = nullptr;

	ComPtr <ID3D12Resource> resourceLight;
};