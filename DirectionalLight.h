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
	Vector4 color = { 1.0f,1.0f,1.0f,1.0f };    //ライト色
	Vector3 direction = { 0,-1.0f,0 };			//ライトの向き
	float intensity = 1.0f;				   	 //ライトの強さ
};

struct DirectionalLightsParam {
	DirectionalLightParam lights[10];
	int count = 0;
	float padding[3];
};

class DirectionalLight
{
public:

	// 設定
	void Initialize();

	// 参照w
	DirectionalLightParam* GetLightData(int index) {
		return &lightDatas_->lights[index];
	};

	// ライト追加 & 参照
	DirectionalLightParam& AddLight(DirectionalLightParam param) {

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

	DirectionalLightsParam* lightDatas_;

	ComPtr <ID3D12Resource> resourceLight;
};
