#pragma once
#include "Texture.h"

#include <d3d12.h>

#include <string>
#include <vector>

#include <cassert>

class TextureManager {
public:

	// ｼﾝｸﾞﾙﾄﾝ
	static TextureManager* GetInstance();

	// コピー禁止
	TextureManager() = default;
	~TextureManager() = default;
	TextureManager(TextureManager&) = delete;
	TextureManager& operator=(TextureManager&) = delete;

	// 初期化
	void Initialize();

	// 終了
	void Finalize();

	/// <summary>
	/// テクスチャファイル読込
	/// </summary>
	/// <param name="filePath">ファイルパス</param>
	void LoadTexture(const std::string& filePath);

	/// <summary>
	/// SRVインデックスの開始番号
	/// </summary>
	/// <param name="filePath">読み込むテキスチャのファイルのパス</param>
	/// <returns></returns>
	uint32_t GetTextureIndexByFilePath(const std::string& filePath);

	/// <summary>
	/// 読み込み済みデータからGPUハンドル読み取り
	/// </summary>
	/// <param name="textureIndex">テキスチャ番号</param>
	/// <returns></returns>
	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandleGPU(uint32_t textureIndex);


	// テキスチャの横幅
	float GetTextureWidth(uint32_t textureIndex){
		// 範囲外エラー
		assert(textureIndex < textureDatas.size());

		return textureDatas.at(textureIndex).GetTextureWidth();
	}

	// テキスチャの縦幅
	float GetTextureHeight(uint32_t textureIndex) {
		// 範囲外エラー
		assert(textureIndex < textureDatas.size());

		return textureDatas.at(textureIndex).GetTextureHeight();
	}
private:

	// テクスチャデータまとめ
	std::vector<Texture> textureDatas;

};
