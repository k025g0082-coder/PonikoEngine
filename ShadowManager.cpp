#include "ShadowManager.h"

// 初期化
void ShadowManager::Initialize()
{
    /*
        ① Resourceを作る
                ↓
        ┌──────────────────┐
        │ ShadowMap        │
        │ 2048 × 2048      │
        │ D32_FLOAT        │
        │                  │
        │ 深度を保存する箱 │
        └──────────────────┘
                ↑
                │
        ② DSVを作る
                │
        ┌──────────────────┐
        │ DSV              │
        │                  │
        │ 「このResourceを │
        │  Depthとして使う」│
        └──────────────────┘
    */

        ID3D12Device* device =
            DirectXCommon::GetInstance()
            ->GetDevice()
            .Get();

        const UINT shadowMapSize = 2048;

        // ============================================================
        // ShadowMap Resource
        // ============================================================

        D3D12_RESOURCE_DESC resourceDesc{};

        resourceDesc.Dimension =
            D3D12_RESOURCE_DIMENSION_TEXTURE2D;

        resourceDesc.Width =
            shadowMapSize;

        resourceDesc.Height =
            shadowMapSize;

        resourceDesc.DepthOrArraySize = 1;

        resourceDesc.MipLevels = 1;

        resourceDesc.Format =
            DXGI_FORMAT_D32_FLOAT;

        resourceDesc.SampleDesc.Count = 1;

        resourceDesc.Flags =
            D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;


        D3D12_CLEAR_VALUE clearValue{};

        clearValue.Format =
            DXGI_FORMAT_D32_FLOAT;

        clearValue.DepthStencil.Depth =
            1.0f;

        clearValue.DepthStencil.Stencil =
            0;


        CD3DX12_HEAP_PROPERTIES heapProperties(
            D3D12_HEAP_TYPE_DEFAULT
        );

        HRESULT hr = device->CreateCommittedResource(
            &heapProperties,
            D3D12_HEAP_FLAG_NONE,
            &resourceDesc,
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &clearValue,
            IID_PPV_ARGS(&shadowMapResource_)
        );

        assert(SUCCEEDED(hr));


        // ============================================================
        // DSV Heap
        // ============================================================

        D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};

        dsvHeapDesc.NumDescriptors = 1;

        dsvHeapDesc.Type =
            D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

        dsvHeapDesc.Flags =
            D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

        hr = device->CreateDescriptorHeap(
            &dsvHeapDesc,
            IID_PPV_ARGS(
                &shadowMapDsvHeap_));

        assert(SUCCEEDED(hr));


        // ============================================================
        // DSV
        // ============================================================

        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};

        dsvDesc.Format =
            DXGI_FORMAT_D32_FLOAT;

        dsvDesc.ViewDimension =
            D3D12_DSV_DIMENSION_TEXTURE2D;

        dsvDesc.Texture2D.MipSlice = 0;


        device->CreateDepthStencilView(
            shadowMapResource_.Get(),
            &dsvDesc,
            shadowMapDsvHeap_
            ->GetCPUDescriptorHandleForHeapStart());
  
}

void ShadowManager::Register(Model* model) {
	if (!model) {
		return;
	}
	shadowCaster_.push_back(model);
}
void ShadowManager::Clear() {

}

// 常に更新
void ShadowManager::RenderShadowMap() {

    ID3D12GraphicsCommandList* commandList =
        DirectXCommon::GetInstance()
        ->GetCommandList()
        .Get();

    // ============================================================
    // ① ShadowMapを描画先にする
    // ============================================================

    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle =
        shadowMapDsvHeap_
        ->GetCPUDescriptorHandleForHeapStart();

    commandList->OMSetRenderTargets(
        0,
        nullptr,
        FALSE,
        &dsvHandle);


    // ============================================================
    // ② ShadowMapの深度をクリア
    // ============================================================

    commandList->ClearDepthStencilView(
        dsvHandle,
        D3D12_CLEAR_FLAG_DEPTH,
        1.0f,
        0,
        0,
        nullptr);

   // ③ ライトのView / Projectionを作る

// ライトの位置(例
    Vector3 lightPosition = {
        -10.0f,
        20.0f,
        -10.0f
    };

    // ライトが向いている方向
    Vector3 lightDirection = {
        0.0f,
        -1.0f,
        0.0f
    };


    // ④ ShadowMap用PSOをセット

    // ⑤ 登録されたモデルを全部描画
    for (Model* model : shadowCaster_)
    {
        if (!model) {
            continue;
        }
        
        model->DrawShadow(lightView_, lightProjection_);

    }
}
