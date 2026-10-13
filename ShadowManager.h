#pragma once

#include "d3d12.h"
#include "d3dx12.h"

#include "wrl.h"
using namespace Microsoft::WRL;

#include "Matrix.h"
#include "Vector.h"

#include "WorldTransform.h"

#include <vector>

// シャドウマップ用の行列
struct ShadowTransform {
    Matrix4x4 lightWVP;
};

class Model;

class ShadowManager
{

private:

    Matrix4x4 MakeLightViewMatrix(
        const Vector3& eye,
        const Vector3& target);

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


    // ShadowMapをシェーダーから読むためのヒープ
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> shadowMapSrvHeap_;

    // SRVのディスクリプタサイズ
    UINT srvDescriptorSize_ = 0;
public:

    // ライトの位置(例
    Vector3 lightPosition;

    Vector3 lightDirection;


    // シングルトン
    static ShadowManager* GetInstance();

    void Initialize();

    // 登録
    void Register(Model* model);
    void Clear();

    // 常に更新
    void RenderShadowMap();
    
    WorldTransform& GetWorldTransform() { return worldTransform_; }

    D3D12_GPU_DESCRIPTOR_HANDLE GetShadowMapSrvHandle() const {
        return shadowMapSrvHeap_->GetGPUDescriptorHandleForHeapStart();
    }

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetShadowMapSrv() {
        return shadowMapSrvHeap_;
    }

    Matrix4x4 GetLightWVP() const {
        return Multiply(lightView_, lightProjection_);
    }
};

