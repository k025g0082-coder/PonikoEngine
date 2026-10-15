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

// １０個まで
struct PointLightsParam {

	PointLightParam lights[30];
	int count = 0;
	float padding[3];
};

class PointLight
{
public:

	// 設定
	void Initialize();

	// 参照w
	PointLightParam* GetLightData(int index) {
		return &lightDatas_->lights[index];
	};

	// ライト追加 & 参照を返す
	PointLightParam& AddLight(PointLightParam param)
	{
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

	// imgui用
	PointLightsParam& GetPointLightsParam()
	{
		return *lightDatas_;
	}

	// ライト削除
	void RemoveLight(PointLightParam* param)
	{
		if (!param || !lightDatas_)
			return;

		for (int i = 0; i < lightDatas_->count; ++i)
		{
			// ポインタが一致するライトを探す
			if (&lightDatas_->lights[i] == param)
			{
				Remove(i);
				return;
			}
		}
	}

	void Remove(int i)
	{
		if (i < 0 || i >= lightDatas_->count)
			return;

		// i番目以降を1つ前に詰める
		for (int j = i; j < lightDatas_->count - 1; ++j)
		{
			lightDatas_->lights[j] = lightDatas_->lights[j + 1];
		}

		// 要素数を減らす
		--lightDatas_->count;
	}

private:

	// ３０こまで
	static const int kMaxLight = 30;

	PointLightsParam* lightDatas_;

	ComPtr <ID3D12Resource> resourceLight;

};

