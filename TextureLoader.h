#pragma once
#include <d3d12.h>
#include "externals/DirectXTex/DirectXTex.h"


#include <string>

#include "Texture.h"

// 主にテキスチャ読込

namespace TextureLoader {

	// Textureデータを読む
	DirectX::ScratchImage LoadTexture(const std::string& filePath);

	// TextureResourceを作る
	ID3D12Resource* CreateTextureResource(ID3D12Device* device, const DirectX::TexMetadata& metadata);

	// データを転送する
	void UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages);

};

