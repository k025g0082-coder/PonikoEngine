#include "Texture.h"
#include "TextureLoader.h"

#include "DirectXCommon.h"
#include "DirectXUtility.h"

void Texture::Initialize(std::string texturePath) {

	//Texture読込み
	mipImages = TextureLoader::LoadTexture(texturePath);

	const DirectX::TexMetadata & metadata = mipImages.GetMetadata();

	// サイズ読み取り
	textureWidth = static_cast<float>(metadata.width);
	textureHeight = static_cast<float>(metadata.height);

	textureResource = TextureLoader::CreateTextureResource(DirectXCommon::GetInstance()->GetDevice().Get(), metadata);

	// 転送
	TextureLoader::UploadTextureData(textureResource.Get(), mipImages);

	// === metaDataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; //2D++
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

	//SRVを作成するDescriptorHeapの場所を決める
	textureSrvHandleCPU = DirectXCommon::GetInstance()->GetSrvHeap()->GetCPUDescriptorHandleForHeapStart();
	textureSrvHandleGPU = DirectXCommon::GetInstance()->GetSrvHeap()->GetGPUDescriptorHandleForHeapStart();

	// 1～
	uint32_t index = DirectXCommon::GetInstance()->GetAllocateSrv();
	srvIndex_ = index;

	// 現在のサイズをもらう（３２らしい。ディスクリプタは３２、６４、１２８と並んでいる）
	UINT increment =
		DirectXCommon::GetInstance()->GetDevice().Get()->GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// 先頭はImGuiが使っているのでその次を使う。　上の法則により、掛け算？
	textureSrvHandleCPU.ptr += increment * index;
	textureSrvHandleGPU.ptr += increment * index;
	
	// 要はしたのも３２
	//textureSrvHandleCPU.ptr += DirectXCommon::GetInstance()->GetDevice().Get()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	//textureSrvHandleGPU.ptr += DirectXCommon::GetInstance()->GetDevice().Get()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//SRVの生成
	DirectXCommon::GetInstance()->GetDevice().Get()->CreateShaderResourceView(textureResource.Get(), &srvDesc, textureSrvHandleCPU);
	texturePath_ = texturePath;
}

	// imageからtextureを作る場合の初期化処理
void Texture::Initialize(unsigned char* image, int width, int height) {

	// 初期化
	mipImages.Initialize2D(
		DXGI_FORMAT_R8G8B8A8_UNORM,
		width,
		height,
		1,
		1);

	// imageから中身をコピー
	memcpy(
		mipImages.GetPixels(),
		image,
		width * height * 4);

	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();

	// サイズ読み取り
	textureWidth = static_cast<float>(metadata.width);
	textureHeight = static_cast<float>(metadata.height);

	textureResource = TextureLoader::CreateTextureResource(DirectXCommon::GetInstance()->GetDevice().Get(), metadata);

	// 転送
	TextureLoader::UploadTextureData(textureResource.Get(), mipImages);

	// === metaDataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; //2D++
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

	//SRVを作成するDescriptorHeapの場所を決める
	textureSrvHandleCPU = DirectXCommon::GetInstance()->GetSrvHeap()->GetCPUDescriptorHandleForHeapStart();
	textureSrvHandleGPU = DirectXCommon::GetInstance()->GetSrvHeap()->GetGPUDescriptorHandleForHeapStart();

	// 1～
	uint32_t index = DirectXCommon::GetInstance()->GetAllocateSrv();
	srvIndex_ = index;

	// 現在のサイズをもらう（３２らしい。ディスクリプタは３２、６４、１２８と並んでいる）
	UINT increment =
		DirectXCommon::GetInstance()->GetDevice().Get()->GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// 先頭はImGuiが使っているのでその次を使う。　上の法則により、掛け算？
	textureSrvHandleCPU.ptr += increment * index;
	textureSrvHandleGPU.ptr += increment * index;

	// 要はしたのも３２
	//textureSrvHandleCPU.ptr += DirectXCommon::GetInstance()->GetDevice().Get()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	//textureSrvHandleGPU.ptr += DirectXCommon::GetInstance()->GetDevice().Get()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//SRVの生成
	DirectXCommon::GetInstance()->GetDevice().Get()->CreateShaderResourceView(textureResource.Get(), &srvDesc, textureSrvHandleCPU);


}

void Texture::TextTextureUpdate(unsigned char* image) {

	memcpy(
		mipImages.GetPixels(),
		image,
		static_cast<int>(textureWidth)*
		static_cast<int>(textureHeight) * 4);

	TextureLoader::UploadTextureData(textureResource.Get(), mipImages);
};