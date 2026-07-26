#include "Model.h"

#include "PipelineStateManager.h"
#include "DirectXCommon.h"
#include "DirectXUtility.h"

#include "WinApp.h"

#include "ImGuiManager.h"

#include "ModelLoader.h"

#include "Matrix.h"

#include <fstream>
#include <sstream>

// 静的メンバ関数
Model* Model::CreateOBJ(const std::string& modelPath) {

	Model* model = new Model();

	model->modelData = ModelLoader::LoadObjModel(modelPath);

	//model->texture.Initialize(model->modelData.material.textureFilePath);
	model->texture.Initialize("resources/white100x100.png");

	//頂点リソースを作る
	model->vertexResource = DirectXUtility::CreateBufferResource(
		DirectXCommon::GetInstance()->GetDevice().Get(), sizeof(ModelVertexData) * model->modelData.vertices.size());

	model->vertexBufferView = {};
	model->vertexBufferView.BufferLocation = model->vertexResource->GetGPUVirtualAddress();
	model->vertexBufferView.SizeInBytes =
		UINT(sizeof(ModelVertexData) * model->modelData.vertices.size());
	model->vertexBufferView.StrideInBytes = sizeof(ModelVertexData);

	ModelVertexData* vertexData = nullptr;
	model->vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	std::memcpy(
		vertexData,
		model->modelData.vertices.data(),
		sizeof(ModelVertexData) * model->modelData.vertices.size());

	// 初期化しとく
	model->Initialize();

	return model;
	//// モデル読込
	//modelData = LoadObjFile("resources/Models", modelName + ".obj");

	//// テクスチャ
	//texture.Initialize("resources/test.png");

	////頂点バッファビューを作成する
	//vertexBufferView = {};
	//vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();//リソースの先頭のアドレスから使う
	//vertexBufferView.SizeInBytes = UINT(sizeof(ModelVertexData) * modelData.vertices.size());//使用するリソースのサイズは頂点のサイズ
	//vertexBufferView.StrideInBytes = sizeof(ModelVertexData);//1頂点あたりのサイズ
	//
	//ModelVertexData* vertexData = nullptr;
	//vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	//std::memcpy(
	//	vertexData,
	//	modelData.vertices.data(),
	//	sizeof(ModelVertexData) * modelData.vertices.size()
	//);

	


}

ModelData Model::LoadObjFile(const std::string& directoryPath, const std::string& filename)
{

	//1.中で必要となる変数の宣言

	ModelData modelData;//構築するModelData
	std::vector<Vector4> positions;// 位置
	std::vector<Vector3> normals;//法線
	std::vector<Vector2> texcoords;//テクスチャ座標
	std::string line;//ファイルから読んだ1行を格納するもの

	//2.ファイルを開く
	std::ifstream file(directoryPath + "/" + filename); // ファイルを開く
	assert(file.is_open()); //とりあえず開けなかったら止める

	//3.実際にファイルを読み、ModelDataを構築していく

	while (std::getline(file, line)) {

		std::string identifier;
		std::istringstream s(line);

		s >> identifier;//先頭の識別子を読む

		if (identifier == "v") {
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.x *= -1.0f; // 右手系から左へ
			position.w = 1.0f;
			positions.push_back(position);
		}
		else if (identifier == "vt") {
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;

			texcoord.y = 1.0f - texcoord.y;   // UVのYを反転（右手→左手対応）

			texcoords.push_back(texcoord);
		}
		else if (identifier == "vn") {
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;

			normal.x *= -1.0f; // 右手系から左へ

			normals.push_back(normal);
		}
		else if (identifier == "f") {

			ModelVertexData triangle[3];

			//面は三角形限定。その他は未対応
			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				std::string vertexDefinition;
				s >> vertexDefinition;
				//頂点の要素へのIndexは「位置/UV/法線」で格納されているので、分解してIndexを取得す
				std::istringstream v(vertexDefinition);
				uint32_t elementIndices[3];
				for (int32_t element = 0; element < 3; ++element) {
					std::string index;
					std::getline(v, index, '/');///区切りでインデックスを読んでいく
					elementIndices[element] = std::stoi(index);
				}
				//要素へのIndexから、実際の要素の値を取得して、頂点を構築する
				Vector4 position = positions[elementIndices[0] - 1];
				Vector2 texcoord = texcoords[elementIndices[1] - 1];
				Vector3 normal = normals[elementIndices[2] - 1];
				//VertexData vertex = { position, texcoord, normal };
				//modelData.vertices.push_back(vertex);

				triangle[faceVertex] = { position, texcoord, normal };
			}

			// 頂点を逆で登録
			modelData.vertices.push_back(triangle[2]);
			modelData.vertices.push_back(triangle[1]);
			modelData.vertices.push_back(triangle[0]);

		}
		else if (identifier == "mtllib") {
			//materialTemplateLibraryファイルの名前を取得する
			std::string materialFilename;
			s >> materialFilename;
			//基本的にobjファイルと同一階層にmtlは存在させるので、ディレクトリ名とファイル名を渡す
			modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
		}

	}

	//4.ModelDataを返す
	return modelData;
};

MaterialData Model::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename) {

	MaterialData materialData; //構築するMaterialData
	std::string line;//ファイルから読んだ1行を格納するもの
	std::ifstream file(directoryPath + "/" + filename); //ファイルを開く
	assert(file.is_open()); //とりあえず開けなかったら止める

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		//identifierに応じた処理
		if (identifier == "map_Kd") {
			std::string textureFilename;
			s >> textureFilename;
			//連結してファイルパスにする
			materialData.textureFilePath = directoryPath + textureFilename;

		}
	}
	return materialData;
}

//static void PreDraw();
//static void PostDraw();

void Model::Initialize(){

	// WorldViewProjection用のリソース
	wvpResource = DirectXUtility::CreateBufferResource(DirectXCommon::GetInstance()->GetDevice().Get(), sizeof(TransformationMatrix));

	//書き込むためのアドレスを取得
	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&mvpData));
	//単位行列を書きこんでおく
	*mvpData = { MakeIdentity4x4(),MakeIdentity4x4()};

	// アニメ有り無しフラグ
	if (modelData.isSkinning) {
		mvpData->isSkinning = 1;
	}
	else {
		mvpData->isSkinning = 0;
	}

	// マテリアル用のリソース
	materialResource = DirectXUtility::CreateBufferResource(DirectXCommon::GetInstance()->GetDevice().Get(), sizeof(Material));

	Material* materialData = nullptr;

	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	materialData->color = { 1.0f,1.0f,1.0f,1.0f }; // 白色
	materialData->enableLighting = true; // ライティングはOFFにする
	materialData->uvTransform = MakeIdentity4x4();

	worldTransform_.translate = { 0.0f,0,0 };
	worldTransform_.rotate = { 0,0,0 };
	worldTransform_.scale = { 1.0f,1.0f,1.0f };

	// 頂点データ
	indexResource = DirectXUtility::CreateBufferResource(DirectXCommon::GetInstance()->GetDevice().Get(), sizeof(uint32_t) * modelData.indices.size());

	// リソースの先頭のアドレスから使う
	indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();

	//使用するリソースのサイズはインデックスmodelData.indices.size()つ分のサイズ
	indexBufferView.SizeInBytes =
		static_cast<UINT>(sizeof(uint32_t) * modelData.indices.size());//インデックスはuint32_tとする

	indexBufferView.Format = DXGI_FORMAT_R32_UINT;

	uint32_t* indexData = nullptr;

	indexResource->Map(
		0,
		nullptr,
		reinterpret_cast<void**>(&indexData)
	);

	// コピー
	std::memcpy(
		indexData,
		modelData.indices.data(),
		sizeof(uint32_t) * modelData.indices.size()
	);

	indexResource->Unmap(0, nullptr);

	if (modelData.isSkinning) {

		// 生成
		skinningResource_ = DirectXUtility::CreateBufferResource(DirectXCommon::GetInstance()->GetDevice().Get(), sizeof(SkinningData));

		// マップ
		skinningResource_->Map(
			0,
			nullptr,
			reinterpret_cast<void**>(&skinningData_));

	}

	// これがrootSignatureをたどってッパイプラインのレイアウトから情報を読み取られるのだーーーーーーーーー
}

void Model::Update(float deltaTime){

	// 時間加算
	//animationTime_ += (deltaTime / 60.0f);

	if (!modelData.animations.empty())
	{
		// アニメーションできないので工事中
		//UpdateAnimation();
	}
	else
	{
		// ボーンはあるけどアニメ無
		for (size_t i = 0; i < modelData.bones.size(); i++)
		{
			skinningData_->boneMatrices[i] = MakeIdentity4x4();
		}
	}

	//for (size_t i = 0; i < modelData.bones.size(); i++)
	//{
	//	skinningData_->boneMatrices[i] = MakeIdentity4x4();
	//}

	/*modelData.bones[0].localMatrix = makeTranslationMatrix({ 1.1f,0,0 });

	modelData.bones[0].globalMatrix =
		modelData.bones[0].localMatrix;

	modelData.bones[0].finalMatrix =
		modelData.bones[0].globalMatrix;

	skinningData_->boneMatrices[0] =
		modelData.bones[0].finalMatrix;*/
	//for (size_t i = 0; i < modelData.vertices.size(); i++)
	//{
	//	auto& bw = modelData.vertices[i].boneWeight;

	//	DirectXUtility::Log("Vertex %zu\n", i);
	//	DirectXUtility::Log("  Index : %u %u %u %u\n",
	//		bw.boneIndex[0],
	//		bw.boneIndex[1],
	//		bw.boneIndex[2],
	//		bw.boneIndex[3]);

	//	DirectXUtility::Log("  Weight: %.3f %.3f %.3f %.3f\n",
	//		bw.weight[0],
	//		bw.weight[1],
	//		bw.weight[2],
	//		bw.weight[3]);

	////	if (i >= 20) break; // 最初の20頂点だけ
	//}

	Matrix4x4 worldMatrix = MakeAffineMatrix(worldTransform_.scale, worldTransform_.rotate, worldTransform_.translate);

	Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(camera_->GetViewMatrix(), camera_->GetProjectionMatrix()));

	mvpData->WVP = worldViewProjectionMatrix;
	mvpData->World = worldMatrix;

}

// アニメ更新
void Model::UpdateAnimation() {
	
	// 空なら帰る
	if (modelData.animations.empty())
	{
		return;
	}

	// アニメ情報読み取り。
	Animation& animation =
		modelData.animations[animationIndex_];

	// 秒→Tick
	// assimpは例、120tick / 30tick = 4秒のようになるので。
	// 掛け算で求める
	float currentTick =
		animationTime_ * animation.ticksPerSecond;

	// ループ
	currentTick =
		fmod(currentTick, animation.duration);

	// SRT、それぞれ取り出して計算
	// arm, leg, など各部位ひとつづつ取り出し更新
	for (NodeAnimation& nodeAnim : animation.nodeAnimations)
	{
		auto it = modelData.boneMap.find(nodeAnim.boneName);
		if (it == modelData.boneMap.end()) continue;

		uint32_t boneIndex = it->second;

		// 回転キーが無い場合は、このボーンは更新しない（バインドを完全維持）
		if (nodeAnim.rotations.empty())
		{
			continue;
		}

		Vector3 position = {
			modelData.bones[boneIndex].localBindMatrix.m[3][0],
			modelData.bones[boneIndex].localBindMatrix.m[3][1],
			modelData.bones[boneIndex].localBindMatrix.m[3][2]
		};
		Vector4 rotation = SampleRotation(nodeAnim, currentTick);
		Vector3 scale = { 1.0f, 1.0f, 1.0f };

		if (!nodeAnim.translations.empty())
			position = SampleTransition(nodeAnim, currentTick);

		if (!nodeAnim.scales.empty())
			scale = SampleScale(nodeAnim, currentTick);

		Matrix4x4 scaleMatrix = makeScaleMatrix(scale);
		Matrix4x4 rotateMatrix = MakeRotateMatrix(rotation);
		Matrix4x4 translateMatrix = makeTranslationMatrix(position);

		modelData.bones[boneIndex].localMatrix =
			Multiply(Multiply(translateMatrix, rotateMatrix), scaleMatrix);
	}

	// 親子関係を基に計算する
	for (Bone& bone : modelData.bones)
	{
		if (bone.parentIndex == -1)
		{
			bone.globalMatrix = bone.localMatrix;
		}
		else
		{
			/* 固定！！！ */
			bone.globalMatrix =
				Multiply(
					modelData.bones[bone.parentIndex].globalMatrix,bone.localMatrix
					
				);
		}

		// 最終的にシェーダーに転送する行列
		bone.finalMatrix =
			Multiply(bone.globalMatrix,bone.offsetMatrix
				
				);
	}// ルートの代わりにこれ
	if (modelData.bones.size() > 2)
	{
		auto& b = modelData.bones[2]; // 適当な子供
		DirectXUtility::Log("Time: %f  Bone2 LOCAL pos: %f %f %f\n",
			currentTick,
			b.localMatrix.m[3][0],
			b.localMatrix.m[3][1],
			b.localMatrix.m[3][2]);

		DirectXUtility::Log("Bone2 FINAL pos: %f %f %f\n",
			b.finalMatrix.m[3][0],
			b.finalMatrix.m[3][1],
			b.finalMatrix.m[3][2]);
	}

	//// 毎回コピーする
	for (size_t i = 0; i < modelData.bones.size(); i++)
	{
		//boneMatrices_[i] = modelData.bones[i].finalMatrix;

		skinningData_->boneMatrices[i] =
			modelData.bones[i].finalMatrix;
	}
		
//	}
//auto& b = modelData.bones[0]; // 親
//
//
//DirectXUtility::Log("GLOBAL\n");
//for (int i = 0; i < 3; i++)
//{
//	DirectXUtility::Log(
//		"%f %f %f\n",
//		b.globalMatrix.m[i][0],
//		b.globalMatrix.m[i][1],
//		b.globalMatrix.m[i][2]);
//}
//
//DirectXUtility::Log("FINAL\n");
//for (int i = 0; i < 3; i++)
//{
//	DirectXUtility::Log(
//		"%f %f %f\n",
//		b.finalMatrix.m[i][0],
//		b.finalMatrix.m[i][1],
//		b.finalMatrix.m[i][2]);
//}
//DirectXUtility::Log("OFFSET\n");
//
//for (int i = 0; i < 3; i++)
//{
//	DirectXUtility::Log(
//		"%f %f %f\n",
//		b.offsetMatrix.m[i][0],
//		b.offsetMatrix.m[i][1],
//		b.offsetMatrix.m[i][2]);
//}
//DirectXUtility::Log("Local\n");
//
//for (int i = 0; i < 3; i++)
//{
//	DirectXUtility::Log(
//		"%f %f %f\n",
//		b.localMatrix.m[i][0],
//		b.localMatrix.m[i][1],
//		b.localMatrix.m[i][2]);
//}
//	//auto& b0 = modelData.bones[0]; // 親
	//auto& b1 = modelData.bones[1]; // 子

	//DirectXUtility::Log("===== Bone0 =====\n");
	//DirectXUtility::Log(
	//	"GLOBAL pos : %f %f %f\n",
	//	b0.globalMatrix.m[3][0],
	//	b0.globalMatrix.m[3][1],
	//	b0.globalMatrix.m[3][2]
	//);

	//DirectXUtility::Log("===== Bone1 =====\n");
	//DirectXUtility::Log(
	//	"LOCAL pos : %f %f %f\n",
	//	b1.localMatrix.m[3][0],
	//	b1.localMatrix.m[3][1],
	//	b1.localMatrix.m[3][2]
	//);

	//DirectXUtility::Log(
	//	"GLOBAL pos : %f %f %f\n",
	//	b1.globalMatrix.m[3][0],
	//	b1.globalMatrix.m[3][1],
	//	b1.globalMatrix.m[3][2]
	//);
	//// 毎回コピーする
	//for (size_t i = 0; i < modelData.bones.size(); i++)
	//{
	//	//boneMatrices_[i] = modelData.bones[i].finalMatrix;

	//	skinningData_->boneMatrices[i] =
	//		modelData.bones[i].finalMatrix;


	//	auto PrintMatrix = [](const char* name, const Matrix4x4& mat)
	//		{
	//			DirectXUtility::Log("%s\n", name);

	//			for (int row = 0; row < 4; row++)
	//			{
	//				DirectXUtility::Log(
	//					"[ %7.3f %7.3f %7.3f %7.3f ]\n",
	//					mat.m[row][0],
	//					mat.m[row][1],
	//					mat.m[row][2],
	//					mat.m[row][3]
	//				);
	//			}

	//			DirectXUtility::Log("\n");
	//		};


	//	auto& m = modelData.bones[i];

	//	DirectXUtility::Log("=====================================\n");
	//	DirectXUtility::Log("Bone[%d] %s\n", (int)i, m.name.c_str());


	//	PrintMatrix("LOCAL", m.localMatrix);
	//	PrintMatrix("GLOBAL", m.globalMatrix);
	//	PrintMatrix("OFFSET", m.offsetMatrix);
	//	PrintMatrix("FINAL", m.finalMatrix);
	//	// test
	//	//skinningData_->boneMatrices[i] =
	//	//	MakeIdentity4x4();

	//	
	//}

}

Vector3 Model::SampleTransition(NodeAnimation& nodeAnim, float currentTick) {

	if (nodeAnim.translations.empty())
	{
		return { 0,0,0 };
	}

	if (currentTick <= nodeAnim.translations[0].time)
	{
		return nodeAnim.translations[0].value;
	}

	if (currentTick >= nodeAnim.translations.back().time)
	{
		return nodeAnim.translations.back().value;
	}

	Vector3 vec;

	KeyframeVector3* key0 = nullptr;
	KeyframeVector3* key1 = nullptr;

	for (size_t i = 0; i + 1 < nodeAnim.translations.size(); i++)
	{
		if (currentTick >= nodeAnim.translations[i].time &&
			currentTick <= nodeAnim.translations[i + 1].time)
		{
			key0 = &nodeAnim.translations[i];
			key1 = &nodeAnim.translations[i + 1];
			break;
		}
	}

	// t求め
	float t = 0.0f;

	float timeLength = key1->time - key0->time;

	if (timeLength > 0.0f)
	{
		t = (currentTick - key0->time) / timeLength;
	}

	t = std::clamp(t, 0.0f, 1.0f);

	// 補間！
	vec = Lerp(key0->value, key1->value, t);

	return vec;
}
Vector3 Model::SampleScale(NodeAnimation& nodeAnim, float currentTick) {

	if (nodeAnim.scales.empty())
	{
		return { 1.0f,1.0f,1.0f };
	}

	if (currentTick <= nodeAnim.scales[0].time)
	{
		return nodeAnim.scales[0].value;
	}

	if (currentTick >= nodeAnim.scales.back().time)
	{
		return nodeAnim.scales.back().value;
	}
	Vector3 vec;

	KeyframeVector3* key0 = nullptr;
	KeyframeVector3* key1 = nullptr;

	for (size_t i = 0; i + 1 < nodeAnim.scales.size(); i++)
	{
		if (currentTick >= nodeAnim.scales[i].time &&
			currentTick <= nodeAnim.scales[i + 1].time)
		{
			key0 = &nodeAnim.scales[i];
			key1 = &nodeAnim.scales[i + 1];
			break;
		}
	}

	// t求め
	float t = 0.0f;

	float timeLength = key1->time - key0->time;

	if (timeLength > 0.0f)
	{
		t = (currentTick - key0->time) / timeLength;
	}

	// 補間！
	vec = Lerp(key0->value, key1->value, t);

	return vec;
}
Vector4 Model::SampleRotation(NodeAnimation& nodeAnim, float currentTick) {

	if (nodeAnim.rotations.empty())
	{
		return { 0,0,0,1.0f };
	}

	if (currentTick <= nodeAnim.rotations[0].time)
	{
		return nodeAnim.rotations[0].value;
	}

	if (currentTick >= nodeAnim.rotations.back().time)
	{
		return nodeAnim.rotations.back().value;
	}

	Vector4 vec;

	KeyframeQuaternion* key0 = nullptr;
	KeyframeQuaternion* key1 = nullptr;

	
	for (size_t i = 0; i + 1 < nodeAnim.rotations.size(); i++)
	{
		if (currentTick >= nodeAnim.rotations[i].time &&
			currentTick <= nodeAnim.rotations[i + 1].time)
		{
			key0 = &nodeAnim.rotations[i];
			key1 = &nodeAnim.rotations[i + 1];
			break;
		}
	}

	// t求め
	float t = 0.0f;

	float timeLength = key1->time - key0->time;

	if (timeLength > 0.0f)
	{
		t = (currentTick - key0->time) / timeLength;
	}

	
	/*DirectXUtility::Log(
		"k0(%f,%f,%f,%f) k1(%f,%f,%f,%f) tick:%f\n",
		key0->value.x,
		key0->value.y,
		key0->value.z,
		key0->value.w,
		key1->value.x,
		key1->value.y,
		key1->value.z,
		key1->value.w,
		currentTick
	);*/

	// 補間!
	vec = Slerp(key0->value, key1->value, t);
	//vec = Normalize(vec);
	return vec;
}

void Model::Draw(ID3D12GraphicsCommandList* commandList){

	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);

	//RootSignatureを設定。PSOに設定しているけど別途設定が必要
	commandList->SetGraphicsRootSignature(PipelineStateManager::GetInstance()->GetPipelineState(PipelineType::Model).rootSignature);
	// PSOを設定
	commandList->SetPipelineState(PipelineStateManager::GetInstance()->GetPipelineState(PipelineType::Model).graphicsPipelineState);


	commandList->IASetIndexBuffer(&indexBufferView);

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());

	if (light_ != nullptr) {
		commandList->SetGraphicsRootConstantBufferView(3, light_->GetResource()->GetGPUVirtualAddress());
	}

	ID3D12DescriptorHeap* heaps[] = {
	DirectXCommon::GetInstance()->GetSrvHeap().Get()
	};

	commandList->SetDescriptorHeaps(1, heaps);

	// SRVDescriptorTableの先頭を設定。２，はRootParametor[2]の２
	commandList->SetGraphicsRootDescriptorTable(2, texture.GetTextureSrvHandleGPU());

	// もしアニメがあったら
	if (modelData.isSkinning) {
		commandList->SetGraphicsRootConstantBufferView(
			4,
			skinningResource_->GetGPUVirtualAddress());
	}

	commandList->DrawIndexedInstanced(static_cast<UINT>(modelData.indices.size()), 1, 0, 0, 0);
}
