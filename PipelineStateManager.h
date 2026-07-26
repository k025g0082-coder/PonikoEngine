#pragma once
#include "DxcCompiler.h"

#include <unordered_map>
#include <d3d12.h>

#include "Vector.h"

enum class PipelineType {
	Sprite,
	Model,
	Triangle
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

	// 描画時に呼べて便利
	//PipelineState SetPipelineState(ID3D12GraphicsCommandList* commandList, PipelineType type);
private:

	PipelineStateManager() = default;

	ID3D12Device* device_;

	// シェーダーコンパイラ
	DxcCompiler dxcCompiler_;

	static std::unordered_map<PipelineType, PipelineState> pipelines_;
	
	void createTrianglePipeline();
	void createSpritePipeline();
	void createModelPipeline();

	// bufferResourceを作る関数
	ID3D12Resource* CreateBufferResource(
		ID3D12Device* device, // デバイス
		size_t sizeInBytes // バッファのサイズ
	);
};

