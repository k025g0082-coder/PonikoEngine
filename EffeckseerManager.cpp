#include "EffekseerManager.h"

EffekseerManager* EffekseerManager::GetInstance()
{
    static EffekseerManager instance;
    return &instance;
}

void EffekseerManager::Initialize(
    ID3D12Device* device,
    ID3D12CommandQueue* commandQueue)
{
    // Effekseer本体
    manager_ = Effekseer::Manager::Create(8000);

    manager_->SetCoordinateSystem(
        Effekseer::CoordinateSystem::LH
    );

    // DX12 GraphicsDevice
    auto graphicsDevice =
        EffekseerRendererDX12::CreateGraphicsDevice(
            device,
            commandQueue,
            3
        );

    // Renderer
    DXGI_FORMAT renderTargetFormats[] = {
        DXGI_FORMAT_R8G8B8A8_UNORM
    };

    renderer_ = EffekseerRendererDX12::Create(
        graphicsDevice,
        renderTargetFormats,
        1,
        DXGI_FORMAT_UNKNOWN,
        false,
        8000
    );

    // 描画モジュール
    manager_->SetSpriteRenderer(
        renderer_->CreateSpriteRenderer()
    );

    manager_->SetRibbonRenderer(
        renderer_->CreateRibbonRenderer()
    );

    manager_->SetRingRenderer(
        renderer_->CreateRingRenderer()
    );

    manager_->SetTrackRenderer(
        renderer_->CreateTrackRenderer()
    );

    manager_->SetModelRenderer(
        renderer_->CreateModelRenderer()
    );

    // ローダー
    manager_->SetTextureLoader(
        renderer_->CreateTextureLoader()
    );

    manager_->SetModelLoader(
        renderer_->CreateModelLoader()
    );

    manager_->SetMaterialLoader(
        renderer_->CreateMaterialLoader()
    );

    manager_->SetCurveLoader(
        Effekseer::MakeRefPtr<Effekseer::CurveLoader>()
    );


    // Effekseer用メモリプール
    memoryPool_ =
        EffekseerRenderer::CreateSingleFrameMemoryPool(
            renderer_->GetGraphicsDevice()
        );

    // Effekseer用コマンドリスト
    commandList_ =
        EffekseerRenderer::CreateCommandList(
            renderer_->GetGraphicsDevice(),
            memoryPool_
        );
}

void EffekseerManager::BeginFrame(
    ID3D12GraphicsCommandList* commandList)
{
    // フレーム開始
    memoryPool_->NewFrame();

    // EffekseerのCommandList開始
    EffekseerRendererDX12::BeginCommandList(
        commandList_,
        commandList
    );

    // RendererにCommandListを渡す
    renderer_->SetCommandList(commandList_);
}

void EffekseerManager::EndFrame()
{
    renderer_->EndRendering();

    // RendererからCommandListを外す
    renderer_->SetCommandList(nullptr);

    // Effekseer CommandList終了
    EffekseerRendererDX12::EndCommandList(
        commandList_
    );
}

Effekseer::EffectRef EffekseerManager::LoadEffect(
    const std::u16string& path)
{
    return Effekseer::Effect::Create(
        manager_,
        path.c_str()
    );
}

Effekseer::Handle EffekseerManager::Play(Effekseer::EffectRef effect,Vector3 v)
{
    return manager_->Play(effect, v.x,v.y,v.z);
}

void EffekseerManager::Update() {

    auto pos = mainCamera->GetWorldPosition();

    Effekseer::Manager::LayerParameter layerParameter;
    layerParameter.ViewerPosition =
        Effekseer::Vector3D(pos.x, pos.y, pos.z);

    EffekseerManager::GetInstance()->GetManager()->SetLayerParameter(
        0,
        layerParameter
    );

    Effekseer::Manager::UpdateParameter updateParameter;

    manager_->Update(updateParameter);

}
void EffekseerManager::Draw() {

    GetRenderer()->BeginRendering();

    Effekseer::Manager::DrawParameter drawParameter;

    //drawParameter.ZNear = 0.0f;
    //drawParameter.ZFar = 1.0f;
    drawParameter.ViewProjectionMatrix =
        GetRenderer()->GetCameraProjectionMatrix();

    GetManager()->Draw(drawParameter);

    GetRenderer()->EndRendering();

}

void EffekseerManager::SetCamera(
    Camera* cam,
    const Effekseer::Matrix44& projection,
    const Effekseer::Matrix44& camera)
{
    mainCamera = cam;
    renderer_->SetProjectionMatrix(projection);
    renderer_->SetCameraMatrix(camera);
}