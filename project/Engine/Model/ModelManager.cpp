#include "ModelManager.h"

#include "ModelLoader.h"
#include "TextureManager.h"

ModelManager* ModelManager::GetInstance()
{
	static ModelManager instance;
	return &instance;
}

void ModelManager::Initialize()
{

}

void ModelManager::Finalize()
{
	modelDatas.clear();
}

void ModelManager::LoadModel(const std::string& filePath)
{
	// 既にあるか検索
    auto it = std::find_if(
        modelDatas.begin(),
        modelDatas.end(),
        [filePath](const std::unique_ptr<SharedModelData>& model) {
            return model->filePath == filePath;
        }
    );

    // すでにあったらreturn
    if (modelDatas.end() != it) return;


	// 追加
	SharedModelData* data = modelDatas.emplace_back(std::make_unique<SharedModelData>()).get();


	// よみこんだファイルパス
	data->filePath = filePath;


	// メイン - 頂点・ボーンやアニメ情報など
	data->modelData = ModelLoader::LoadObjModel(filePath);

	
	if (!data->modelData.materials.textureFilePath.empty()) {
		
		// モデルの標準テクスチャもここで読み込んでおく!
		TextureManager::GetInstance()->LoadTexture(data->modelData.materials.textureFilePath);
	}


	/* 
		頂点リソースを作る
	*/
	data->vertexResource = DirectXUtility::CreateBufferResource(
		DirectXCommon::GetInstance()->GetDevice().Get(), sizeof(ModelVertexData) * data->modelData.vertices.size());

	data->vertexBufferView = {};
	data->vertexBufferView.BufferLocation = data->vertexResource->GetGPUVirtualAddress();
	data->vertexBufferView.SizeInBytes =
		UINT(sizeof(ModelVertexData) * data->modelData.vertices.size());
	data->vertexBufferView.StrideInBytes = sizeof(ModelVertexData);

	ModelVertexData* vertexData = nullptr;
	data->vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	std::memcpy(
		vertexData,
		data->modelData.vertices.data(),
		sizeof(ModelVertexData) * data->modelData.vertices.size());

	//
	data->vertexResource->Unmap(0, nullptr);

	/*
		インデックスリソース
	*/
	data->indexResource = DirectXUtility::CreateBufferResource(DirectXCommon::GetInstance()->GetDevice().Get(), sizeof(uint32_t) * data->modelData.indices.size());

	// リソースの先頭のアドレスから使う
	data->indexBufferView.BufferLocation = data->indexResource->GetGPUVirtualAddress();

	//使用するリソースのサイズはインデックスmodelData.indices.size()つ分のサイズ
	data->indexBufferView.SizeInBytes =
		static_cast<UINT>(sizeof(uint32_t) * data->modelData.indices.size());//インデックスはuint32_tとする

	data->indexBufferView.Format = DXGI_FORMAT_R32_UINT;

	uint32_t* indexData = nullptr;

	data->indexResource->Map(
		0,
		nullptr,
		reinterpret_cast<void**>(&indexData)
	);

	// コピー
	std::memcpy(
		indexData,
		data->modelData.indices.data(),
		sizeof(uint32_t) * data->modelData.indices.size()
	);

	data->indexResource->Unmap(0, nullptr);


	/* 
		アニメ有り無し
	*/
	data->skinningParamResource_ =
		DirectXUtility::CreateBufferResource(
			DirectXCommon::GetInstance()->GetDevice().Get(),
			sizeof(ModelSkinningParam)
		);

	data->skinningParamResource_->Map(
		0,
		nullptr,
		reinterpret_cast<void**>(&data->skinningParam)
	);

	data->skinningParam->isSkinning = data->modelData.isSkinning ? 1 : 0;


}

SharedModelData* ModelManager::GetModelDataByFilePath(const std::string & filePath)
{
	// 既にあるか検索
	auto it = std::find_if(
		modelDatas.begin(),
		modelDatas.end(),
		[filePath](const std::unique_ptr<SharedModelData>& model) {
			return model->filePath == filePath;
		}
	);

	// すでにあったら返す
	if (modelDatas.end() != it) {
		return (*it).get();
	}

	/*
		ない
	*/
	assert(0);
	return nullptr;
}

