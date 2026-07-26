#pragma once

#include "PipelineStateManager.h"

#include "Texture.h"

#include "Matrix.h"
#include "WorldTransform.h"

class Sprite {
public:
	void Initialize(ID3D12Device* device);
	void Update();
	void Draw(ID3D12GraphicsCommandList* commandList);

	void SetPos(Vector3 pos) {
		
		worldTransform_.translate = pos;

		// 更新
		UpdateMatrix();
	}

	

private:

	// 行列更新
	void UpdateMatrix();

	// SRT
	WorldTransform worldTransform_;

	// テクスチャ
	Texture texture;

	ID3D12Device* device_;

	// 頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};

	// 0201 マテリアル用のリソース
	ID3D12Resource* materialResource = nullptr;

	//0202 mvp用のリソース作り
	Matrix4x4* mvpData = nullptr;
	ID3D12Resource* wvpResource = nullptr;

};

