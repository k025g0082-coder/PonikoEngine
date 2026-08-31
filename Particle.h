#pragma once

#include "Model.h"

#include "ParticleEmit.h"


// シェーダーに転送用
struct ParticleData {

	Matrix4x4 WVP;
	Matrix4x4 World;

	Vector4 color;

};

class Particle {
public:
	void Initialize(ModelData modeldata);

	void Update(Camera camera);
	void Draw();
private:

	// 生成
	ParticleEmit MakeNewParticle(Vector3 pos);

	// 最大数
	static const uint32_t kNumInstance = 10;

	ParticleEmit emits[kNumInstance];

	Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource;

	//書き込むためのアドレス
	ParticleData* instancingData = nullptr;

	D3D12_CPU_DESCRIPTOR_HANDLE instancingSrvHandleCPU;
	D3D12_GPU_DESCRIPTOR_HANDLE instancingSrvHandleGPU;

	// 色
	Material* materialData = nullptr;

	// モデルデータ
	ModelData modelData;
	
	// テクスチャ
	Texture* texture = nullptr;

	ModelVertexData* vertexData = nullptr;


	// マテリアル用のリソース
	ComPtr <ID3D12Resource> materialResource;

	ID3D12Resource* indexResource;
	// 頂点リソース
	ComPtr <ID3D12Resource> vertexResource;

	D3D12_INDEX_BUFFER_VIEW indexBufferView{};
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;


};