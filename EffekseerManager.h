#pragma once

#include <Effekseer.h>
#include <EffekseerRendererDX12.h>

#include "Matrix.h"
#include "Camera.h"

class EffekseerManager {
public:

    static EffekseerManager* GetInstance();

    void Initialize(
        ID3D12Device* device,
        ID3D12CommandQueue* commandQueue
    );

    // 毎フレーム開始時
    void BeginFrame(ID3D12GraphicsCommandList* commandList);

    // 毎フレーム終了時
    void EndFrame();

    void Update();
    void Draw();

    Effekseer::EffectRef LoadEffect(const std::u16string& path);

    Effekseer::Handle Play(Effekseer::EffectRef effect,Vector3 v);

    void SetCamera(
        Camera* cam,
        const Effekseer::Matrix44& projection,
        const Effekseer::Matrix44& camera);

    Effekseer::ManagerRef GetManager() {
        return manager_;
    }

    EffekseerRenderer::RendererRef GetRenderer() {
        return renderer_;
    }

    // matrix4x4 変換
    static Effekseer::Matrix44 ToEffekseerMatrix(const Matrix4x4& src)
    {
        Effekseer::Matrix44 dst;

        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                dst.Values[i][j] = src.m[i][j];
            }
        }

        return dst;
    }

private:
    Effekseer::ManagerRef manager_;
    EffekseerRenderer::RendererRef renderer_;

    Effekseer::RefPtr<EffekseerRenderer::SingleFrameMemoryPool> memoryPool_;
    Effekseer::RefPtr<EffekseerRenderer::CommandList> commandList_;

    Camera* mainCamera;
};