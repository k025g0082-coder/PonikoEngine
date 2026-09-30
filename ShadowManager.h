#pragma once

#include "d3d12.h"
#include "d3dx12.h"

#include "wrl.h"
using namespace Microsoft::WRL;

#include "Matrix.h"
#include "Model.h"

#include <vector>

// シェーダー用
struct ShadowTransform
{
    Matrix4x4 world;
    Matrix4x4 lightViewProjection;
};

class ShadowManager
{

private:

	// 登録してるモデルのアドレスs
	std::vector<Model*> shadowCaster_;

    // ShadowMap用
    WorldTransform worldTransform_;

    // ライトをカメラとして扱うための・・？
    Matrix4x4 lightView_;
    Matrix4x4 lightProjection_;

    // ShadowMap本体
    ComPtr<ID3D12Resource> shadowMapResource_;

    // ShadowMapを見るためのDSV
    ComPtr<ID3D12DescriptorHeap> shadowMapDsvHeap_;

public:

    void Initialize();

    // 登録
    void Register(Model* model);
    void Clear();

    // 常に更新
    void RenderShadowMap();
    
    WorldTransform& GetWorldTransform() { return worldTransform_; }

};

