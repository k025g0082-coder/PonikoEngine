#pragma once
#include "externals/DirectXTex/DirectXTex.h"
#include <d3d12.h>

#include "wrl.h"
using namespace Microsoft::WRL;

#include "Vector.h"

#include <string>

// テクスチャの頂点データ。要はVS.hlslに送るレイアウト
struct SpriteVertexData {
	Vector4 position;
	Vector2 texcoord;
};

class Texture
{
public:
	void Initialize(std::string texturePath);

	// Spriteとかの描画時に必要になる
	D3D12_CPU_DESCRIPTOR_HANDLE GetTextureSrvHandleCPU() { return textureSrvHandleCPU; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetTextureSrvHandleGPU() { return textureSrvHandleGPU; }

	float GetTextureWidth() { return textureWidth; }
	float GetTextureHeight() { return textureHeight; }

private:

	DirectX::ScratchImage mipImages;

	float textureWidth;
	float textureHeight;

	ComPtr <ID3D12Resource> textureResource;

	// と、そのハンドル。
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU;
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU;

};

