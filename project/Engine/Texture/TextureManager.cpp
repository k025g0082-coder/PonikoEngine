#include "TextureManager.h"
#include "DirectXCommon.h"



TextureManager* TextureManager::GetInstance()
{
    static TextureManager instance;
    return &instance;
}

void TextureManager::Initialize()
{
    // SRVの数だけ確保
    textureDatas.reserve(DirectXCommon::kMaxSRVCount);
}

void TextureManager::Finalize()
{

}

void TextureManager::LoadTexture(const std::string& filePath)
{
    // テクスチャリストからすでに読み込んでないか検索
    auto it = std::find_if(
        textureDatas.begin(),
        textureDatas.end(),
        [filePath](Texture& texture) {
            return texture.texturePath_ == filePath;
        }
    );

    // すでにあったらreturn
    if (textureDatas.end() != it) return;

    //　読み込み
    auto& texture = textureDatas.emplace_back();

    texture.Initialize(filePath);// 初期化

}

uint32_t TextureManager::GetTextureIndexByFilePath(const std::string& filePath)
{
    // 検索
    auto it = std::find_if(
        textureDatas.begin(),
        textureDatas.end(),
        [filePath](Texture& texture) {
            return texture.texturePath_ == filePath;
        }
    );

    // すでにあったら要素番号を返す
    if (textureDatas.end() != it) {
        return static_cast<uint32_t>(
            std::distance(textureDatas.begin(), it));
    }

    // ちゃんと読み込んでないのでエラー
    assert(0);

    return 0;
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(uint32_t textureIndex)
{
    // 範囲外エラー
    assert(textureIndex < textureDatas.size());

    // 返す
    return textureDatas.at(textureIndex).GetTextureSrvHandleGPU();
}
