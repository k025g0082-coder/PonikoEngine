#pragma once
#include "Model.h"

#include <d3d12.h>

#include <string>
#include <vector>

#include <cassert>
#include <memory>

class ModelManager {
public:

	// ｼﾝｸﾞﾙﾄﾝ
	static ModelManager* GetInstance();

	// コピー禁止
	ModelManager() = default;
	~ModelManager() = default;
	ModelManager(ModelManager&) = delete;
	ModelManager& operator=(ModelManager&) = delete;

	// 初期化
	void Initialize();

	// 終了
	void Finalize();


	/// <summary>
	/// モデル読込
	/// </summary>
	/// <param name="filePath">ファイルパス</param>
	void LoadModel(const std::string& filePath);

	/// <summary>
	/// ファイルパスからモデルデータを取得する
	/// </summary>
	/// <param name="filePath">読み込むモデルのファイルのパス</param>
	/// <returns></returns>
	SharedModelData* GetModelDataByFilePath(const std::string& filePath);

private:

	// モデルデータまとめ
	std::vector<std::unique_ptr<SharedModelData>> modelDatas;

};
