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

// １０0個まで
struct PointLightsParam {

	PointLightParam lights[100];
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
	PointLightParam& AddLight(PointLightParam param,bool isSceneLight = false)
	{
		if (lightDatas_->count >= kMaxLight)
			return lightDatas_->lights[lightDatas_->count - 1];

		/* 追加 */
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

		/* 参照アドレスからインデックス番号検索 */

		int index = ToIndex(param);
		if (index == -1)return;

		Remove(index);
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
	int ToIndex(PointLightParam* param) {

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
	bool GetIsSceneLight(PointLightParam* param) {

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

	// 10０こまで
	static const int kMaxLight = 100;

	/* シーン内の固定の光源かを記録する */
	bool isSceneLight_[kMaxLight]{};


	PointLightsParam* lightDatas_;

	ComPtr <ID3D12Resource> resourceLight;

};

