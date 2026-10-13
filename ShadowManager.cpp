#include "ShadowManager.h"

#include "Model.h"

ShadowManager* ShadowManager::GetInstance() {
    static ShadowManager instance;
    return &instance;
}

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

        //resourceDesc.Format =
        //    DXGI_FORMAT_D32_FLOAT;
        
        resourceDesc.Format = DXGI_FORMAT_R32_TYPELESS;

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
        // ============================================================
      // SRV Heap
      // ============================================================

        D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc{};
        srvHeapDesc.NumDescriptors = 1;
        srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

        hr = device->CreateDescriptorHeap(
            &srvHeapDesc,
            IID_PPV_ARGS(&shadowMapSrvHeap_)
        );

        assert(SUCCEEDED(hr));

        // ディスクリプタサイズ
        srvDescriptorSize_ =
            device->GetDescriptorHandleIncrementSize(
                D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
            );

        // ============================================================
        // ShadowMap SRV
        // ============================================================

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};

        srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Shader4ComponentMapping =
            D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = 1;

        device->CreateShaderResourceView(
            shadowMapResource_.Get(),
            &srvDesc,
            shadowMapSrvHeap_->GetCPUDescriptorHandleForHeapStart()
        );
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

    // バリアー
    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        shadowMapResource_.Get(),
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        D3D12_RESOURCE_STATE_DEPTH_WRITE
    );

    commandList->ResourceBarrier(1, &barrier);


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


    // ③ ShadowMap 用の Viewport / Scissor
    D3D12_VIEWPORT viewport{};
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = 2048.0f;
    viewport.Height = 2048.0f;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    D3D12_RECT scissorRect{
        0, 0, 2048, 2048
    };

    commandList->RSSetViewports(1, &viewport);
    commandList->RSSetScissorRects(1, &scissorRect);

   // ③ ライトのView / Projectionを作る

    // 正規化
    Vector3 direction = Normalize(lightDirection);

    // ライトが見る方向の目標地点
    Vector3 lightTarget = Add(lightPosition, direction);

    // ライトのView行列
    lightView_ = MakeLightViewMatrix(
        lightPosition,
        lightTarget
    );

    // ライトの正射影行列
    lightProjection_ = MakeOrthographicMatrix(
        -20.0f,  // left
        20.0f,  // top
        20.0f,  // right
        -20.0f,  // bottom
        0.1f,   // near
        100.0f   // far
    );

    // ④ 登録されたモデルを全部描画
    for (Model* model : shadowCaster_)
    {
        if (!model) {
            continue;
        }
        
        model->DrawShadow(lightView_, lightProjection_);

    }

    // 転移
    auto barrierToSRV = CD3DX12_RESOURCE_BARRIER::Transition(
        shadowMapResource_.Get(),
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
    );
    commandList->ResourceBarrier(1, &barrierToSRV);
}

// ライトのView行列を作る
Matrix4x4 ShadowManager::MakeLightViewMatrix(
    const Vector3& eye,
    const Vector3& target)
{

    Vector3 forward = Normalize(target - eye);

    // forward がほぼ上下方向のときは別の up を使う
    Vector3 worldUp = { 0.0f, 1.0f, 0.0f };
    if (fabsf(forward.y) > 0.99f) {
        worldUp = { 0.0f, 0.0f, 1.0f };  // 横方向を up にする
    }

    Vector3 right = Normalize(Cross(worldUp, forward));
    Vector3 up = Cross(forward, right);

    Matrix4x4 lightWorldMatrix{ {
        { right.x,   right.y,   right.z,   0.0f },
        { up.x,      up.y,      up.z,      0.0f },
        { forward.x, forward.y, forward.z, 0.0f },
        { eye.x,     eye.y,     eye.z,     1.0f },
    } };

    return Inverse(lightWorldMatrix);
}