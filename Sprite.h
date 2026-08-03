#pragma once

#include "PipelineStateManager.h"

#include "Texture.h"

#include "Matrix.h"
#include "WorldTransform.h"

class Sprite {
public:

	void Initialize(std::string texturePath);

	void TextTextureInitialize(unsigned char* image, int width, int height);

	void TextTextureUpdate(unsigned char* image);

	void Update();
	void Draw();

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

