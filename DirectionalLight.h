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
	DirectionalLightParam lights[30];
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
	DirectionalLightParam& AddLight(DirectionalLightParam param,bool isSceneLight = false) {

		if (lightDatas_->count >= kMaxLight)
			return lightDatas_->lights[lightDatas_->count - 1];

		lightDatas_->lights[lightDatas_->count] = param;

		// シーンライトか否か
		isSceneLight_[lightDatas_->count] = isSceneLight;


		return lightDatas_->lights[lightDatas_->count++];

	}
	

	// ライトリセット
	void ResetLight()
	{
		for (int i = 0; i < kMaxLight; i++) {

			lightDatas_->lights[i] = {};
			isSceneLight_[i] = false;

		}

	}

	ID3D12Resource* GetResource() { return resourceLight.Get(); }

	// imgui用
	DirectionalLightsParam& GetDirectionalLightsParam()
	{
		return *lightDatas_;
	}

	// ライト削除
	void RemoveLight(DirectionalLightParam* param)
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
			isSceneLight_[j] = isSceneLight_[j + 1];
		}

		// 要素数を減らす
		--lightDatas_->count;

		// 最後の要素を初期化
		lightDatas_->lights[lightDatas_->count] = {};
		isSceneLight_[lightDatas_->count] = false;
	}

	// アドレスからインデックス番号検索
	int ToIndex(DirectionalLightParam* param) {

		for (int i = 0; i < lightDatas_->count; ++i)
		{
			// ポインタが一致するライトを探す
			if (&lightDatas_->lights[i] == param)
			{
				return i;
			}
		}

		// 見つからない
		return -1;
	}

	// シーンライトかを返す
	bool GetIsSceneLight(DirectionalLightParam* param) {

		if (ToIndex(param) != -1 && ToIndex(param) < kMaxLight) {
			return isSceneLight_[ToIndex(param)];
		}
		else {
			return false;
		}

	}
	void SetIsSceneLight(int index, bool b) {

		if (index != -1 && index < kMaxLight) {
			isSceneLight_[index] = b;
		}
	}


private:

	// ３０こまで
	static const int kMaxLight = 30;


	/* シーン内の固定の光源かを記録する */
	bool isSceneLight_[kMaxLight]{};


	DirectionalLightsParam* lightDatas_;

	ComPtr <ID3D12Resource> resourceLight;
};
