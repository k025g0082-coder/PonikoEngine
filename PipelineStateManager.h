#pragma once
#include "DxcCompiler.h"

#include <unordered_map>
#include <d3d12.h>

#include "ParticleEmit.h"

#include "Vector.h"

enum class PipelineType {

	Sprite,
	Model,

	Triangle,

	Particle,
	Shadow,

	DebugLine,

	PostEffect,

	/* ブルーム */
	SceneBloom,

	/* ブラー ピラミッド方式のため２つ用意 */
	SceneBlurDownSample,
	SceneBlurUpSample,

	SceneComposite,

	ModelBloom,

};

struct PipelineState {

	// rootSignature
	ID3D12RootSignature* rootSignature = nullptr;

	// PSO
	ID3D12PipelineState* graphicsPipelineState = nullptr;

};

class PipelineStateManager
{
public:

	// シングルトン
	static PipelineStateManager* GetInstance();

	void Initialize(ID3D12Device* device);

	// パイプライン取得
	static PipelineState GetPipelineState(PipelineType type) {
		return pipelines_[type];
	}

	// パーティクルブレンドモードのパイプライン取得
	static ID3D12PipelineState* GetParticleBlendPipelineState(BlendMode type) {
		return blendGraphicsPipelineState[(int)type];
	}

	// 描画時に呼べて便利
	//PipelineState SetPipelineState(ID3D12GraphicsCommandList* commandList, PipelineType type);
private:

	PipelineStateManager() = default;

	ID3D12Device* device_;

	// シェーダーコンパイラ
	DxcCompiler dxcCompiler_;

	// モデルやスプライトたちのパイプライン
	static std::unordered_map<PipelineType, PipelineState> pipelines_;

	// パーティクルの、ブレンドモードの　PSOたち
	static ID3D12PipelineState* blendGraphicsPipelineState[(int)BlendMode::Count];

	
	void createTrianglePipeline();

	void createSpritePipeline();
	void createModelPipeline();
	void createParticlePipeline();

	void createShadowPipeline();

	void createDebugLinePipeline();

	void createPostEffectPipeline();

	void createModelBloomPipeline();
	void createModelBlurPipeline();

	// ポストエフェクトの
	D3D12_GRAPHICS_PIPELINE_STATE_DESC postEffectDesc{};
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	// bufferResourceを作る関数
	ID3D12Resource* CreateBufferResource(
		ID3D12Device* device, // デバイス
		size_t sizeInBytes // バッファのサイズ
	);
};

