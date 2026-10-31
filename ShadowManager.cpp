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

        // 真下に落とすためのライト初期値
        lightPosition = { 0.0f, 50.0f, 0.0f };  // 上空 50 の位置
        lightDirection = { 0.0f, -1.0f, 0.0f }; // 真下を向く

        shadowRange = 30.0f; // プレイヤー周囲 30 単位をカバー
        lightNear = 1.0f;
        lightFar = 100.0f;
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
    viewport.MinDepth = -3.0f;
    viewport.MaxDepth = 3.0f;

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
  
    lightProjection_ = MakeOrthographicMatrix(
        -shadowRange, shadowRange,  // left, top
        shadowRange, -shadowRange, // right, bottom
        lightNear, lightFar                // near, far
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


Matrix4x4 ShadowManager::MakeLightViewMatrix(
    const Vector3& eye,
    const Vector3& target)
{
    // Z軸 (Forward): eye から target への方向
    Vector3 zAxis = Normalize(Subtract(target, eye));

    // 真下・真上を向いているかの判定
    Vector3 worldUp = { 0.0f, 0.0f, 1.0f }; // 真下を向く時は Z 軸(+Z)を仮のUpにする
    if (std::abs(zAxis.y) < 0.999f) {
        worldUp = { 0.0f, 1.0f, 0.0f };     // 斜めの時は通常の Y 軸(+Y)をUpにする
    }

    // X軸 (Right)
    Vector3 xAxis = Normalize(Cross(worldUp, zAxis));
    // Y軸 (Up)
    Vector3 yAxis = Cross(zAxis, xAxis);

    Matrix4x4 viewMatrix;
    viewMatrix.m[0][0] = xAxis.x;
    viewMatrix.m[1][0] = xAxis.y;
    viewMatrix.m[2][0] = xAxis.z;
    viewMatrix.m[3][0] = -Dot(xAxis, eye);

    viewMatrix.m[0][1] = yAxis.x;
    viewMatrix.m[1][1] = yAxis.y;
    viewMatrix.m[2][1] = yAxis.z;
    viewMatrix.m[3][1] = -Dot(yAxis, eye);

    viewMatrix.m[0][2] = zAxis.x;
    viewMatrix.m[1][2] = zAxis.y;
    viewMatrix.m[2][2] = zAxis.z;
    viewMatrix.m[3][2] = -Dot(zAxis, eye);

    viewMatrix.m[0][3] = 0.0f;
    viewMatrix.m[1][3] = 0.0f;
    viewMatrix.m[2][3] = 0.0f;
    viewMatrix.m[3][3] = 1.0f;

    return viewMatrix;
}