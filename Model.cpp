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

	// テキスチャ読込
	/*for (MaterialData& m : model->modelData.materials) {
		model->texture.emplace_back();
		model->texture.back().Initialize(m.textureFilePath);
	}*/

	model->texture.Initialize(model->modelData.materials.textureFilePath);

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
}

void Model::Initialize(){

	// マテリアル用のリソース
	materialResource = DirectXUtility::CreateBufferResource(DirectXCommon::GetInstance()->GetDevice().Get(), sizeof(Material));

	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	materialData->color = { 1.0f,1.0f,1.0f,1.0f }; // 白色
	materialData->enableLighting = false; // ライティングはOFFにする
	materialData->uvTransform = MakeIdentity4x4();

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

	skinningParamResource_ =
		DirectXUtility::CreateBufferResource(
			DirectXCommon::GetInstance()->GetDevice().Get(),
			sizeof(ModelSkinningParam)
		);

	skinningParamResource_->Map(
		0,
		nullptr,
		reinterpret_cast<void**>(&skinningParam)
	);

	// アニメ有り無し
	skinningParam->isSkinning =
		modelData.isSkinning ? 1 : 0;
	
	worldTransform_.Initialize();
	worldTransform_.UpdateMatrix();

	// これがrootSignatureをたどってッパイプラインのレイアウトから情報を読み取られるのだーーーーーーーーー
}

void Model::Update(float deltaTime){

	// 時間加算
	animationTime_ += (deltaTime / 60.0f);

	if (!modelData.animations.empty())
	{
		// アニメーションできないので工事中
		UpdateAnimation();
	}
	else
	{
		// ボーンはあるけどアニメ無
		for (size_t i = 0; i < modelData.bones.size(); i++)
		{

			skinningData_->boneMatrices[i] = ModelLoader::ConvertYZ();
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

	/*Matrix4x4 worldMatrix = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);*/

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

		//// 回転キーが無い場合は、このボーンは更新しない（バインドを完全維持）
		//if (nodeAnim.rotations.empty())
		//{

		//	continue;
		//}

		Vector3 position = {
			modelData.bones[boneIndex].localBindMatrix[3][0],
			modelData.bones[boneIndex].localBindMatrix[3][1],
			modelData.bones[boneIndex].localBindMatrix[3][2]
		};
		Vector4 rotation = { 0.0f, 0.0f, 0.0f, 1.0f };  // 単位クォータニオン

		Vector3 scale = { 1.0f, 1.0f, 1.0f };

		if (!nodeAnim.rotations.empty()) {
			rotation = SampleRotation(nodeAnim, currentTick);
			//rotation = { 0.0f, 0.0f, 0.0f, 1.0f };  // 単位クォータニオン
		}

		if (!nodeAnim.translations.empty())
			position = SampleTransition(nodeAnim, currentTick);

		if (!nodeAnim.scales.empty())
			scale = SampleScale(nodeAnim, currentTick);

		aiMatrix4x4 scaleAiMatrix;
		aiMatrix4x4 rotateAiMatrix;
		aiMatrix4x4 translateAiMatrix;

		scaleAiMatrix = aiMatrix4x4(
			scale.x, 0, 0, 0,
			0, scale.y, 0, 0,
			0, 0, scale.z, 0,
			0, 0, 0, 1
		);

		aiQuaternion q(
			rotation.w,
			rotation.x,
			rotation.y,
			rotation.z
		);

		rotateAiMatrix = aiMatrix4x4(q.GetMatrix());

		translateAiMatrix = aiMatrix4x4(
			1, 0, 0, position.x,
			0, 1, 0, position.y,
			0, 0, 1, position.z,
			0, 0, 0, 1
		);

		modelData.bones[boneIndex].localMatrix =
			(translateAiMatrix *
			rotateAiMatrix) *
			scaleAiMatrix;
		
		/*modelData.bones[boneIndex].localMatrix = modelData.bones[boneIndex].localBindMatrix;*/

		Bone bone = modelData.bones[boneIndex];
		DirectXUtility::Log("%f %f %f\n",
			bone.localBindMatrix.a4,
			bone.localBindMatrix.b4,
			bone.localBindMatrix.c4);
	}

	// 親子関係を基に計算する
	
	// ルートボーンから再帰で計算
	for (size_t i = 0; i < modelData.bones.size(); i++)
	{
		if (modelData.bones[i].parentIndex == -1)
		{
			aiMatrix4x4 identity;
			identity = aiMatrix4x4();

			CalculateGlobalMatrices(static_cast<int>(i), identity);
		}
	}
	

	//// 毎回コピーする
	for (size_t i = 0; i < modelData.bones.size(); i++)
	{
		//boneMatrices_[i] = modelData.bones[i].finalMatrix;

		skinningData_->boneMatrices[i] = Multiply(
			ModelLoader::ConvertMatrix(
				modelData.bones[i].finalMatrix),
			ModelLoader::ConvertYZ());
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

// 再帰でグローバル行列を計算する関数
void Model::CalculateGlobalMatrices(int boneIndex, const aiMatrix4x4& parentGlobal)
{
	Bone& bone = modelData.bones[boneIndex];

	// 親 * ローカル に変更
	bone.globalMatrix = parentGlobal *  bone.localMatrix;

	// 最終行列も逆順に
	bone.finalMatrix =  bone.globalMatrix * bone.offsetMatrix;

	// 子供を再帰的に計算
	for (int childIndex : bone.children)
	{
		CalculateGlobalMatrices(childIndex, bone.globalMatrix);
	}
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

Vector4 Model::SampleRotation(NodeAnimation& nodeAnim, float currentTick)
{
	if (nodeAnim.rotations.empty())
	{
		return { 0.0f, 0.0f, 0.0f, 1.0f };
	}

	if (currentTick <= nodeAnim.rotations[0].time)
	{
		return nodeAnim.rotations[0].value;
	}

	if (currentTick >= nodeAnim.rotations.back().time)
	{
		return nodeAnim.rotations.back().value;
	}

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

	if (!key0 || !key1)
	{
		return nodeAnim.rotations.back().value;
	}

	float t = 0.0f;
	float timeLength = key1->time - key0->time;
	if (timeLength > 0.0f)
	{
		t = (currentTick - key0->time) / timeLength;
	}
	t = std::clamp(t, 0.0f, 1.0f);

	Vector4 q0 = key0->value;
	Vector4 q1 = key1->value;

	// ★ここが重要：最短経路になるように符号を揃える
	float dot = q0.x * q1.x + q0.y * q1.y + q0.z * q1.z + q0.w * q1.w;
	if (dot < 0.0f)
	{
		q1.x = -q1.x;
		q1.y = -q1.y;
		q1.z = -q1.z;
		q1.w = -q1.w;
	}

	Vector4 result = Slerp(q0, q1, t);
	// 念のため正規化
	// result = Normalize(result);
	return result;
}


void Model::Draw(Camera camera) {

	Matrix4x4 worldViewProjectionMatrix = Multiply(worldTransform_.matWorld_, Multiply(camera.GetViewMatrix(), camera.GetProjectionMatrix()));

	worldTransform_.mvpData_->World = worldTransform_.matWorld_;
	worldTransform_.mvpData_->WVP = worldViewProjectionMatrix;


	ID3D12GraphicsCommandList* commandList = DirectXCommon::GetInstance()->GetCommandList().Get();

	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);

	//RootSignatureを設定。PSOに設定しているけど別途設定が必要
	commandList->SetGraphicsRootSignature(PipelineStateManager::GetInstance()->GetPipelineState(PipelineType::Model).rootSignature);
	// PSOを設定
	commandList->SetPipelineState(PipelineStateManager::GetInstance()->GetPipelineState(PipelineType::Model).graphicsPipelineState);


	commandList->IASetIndexBuffer(&indexBufferView);

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(1, worldTransform_.GetWvpResource()->GetGPUVirtualAddress());

	if (light_ != nullptr) {
		commandList->SetGraphicsRootConstantBufferView(3, light_->GetResource()->GetGPUVirtualAddress());
	}

	commandList->SetGraphicsRootConstantBufferView(5, skinningParamResource_->GetGPUVirtualAddress());

	ID3D12DescriptorHeap* heaps[] = {
	DirectXCommon::GetInstance()->GetSrvHeap().Get()
	};

	commandList->SetDescriptorHeaps(1, heaps);

	// MaterialDataの番号とtexture配列の番号が同じなら
	//const Texture& tex =
	//	texture[mesh.materialIndex];

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


void Model::Draw(WorldTransform& world,Camera camera){

	Matrix4x4 worldViewProjectionMatrix = Multiply(world.matWorld_, Multiply(camera.GetViewMatrix(), camera.GetProjectionMatrix()));

	world.mvpData_->World = world.matWorld_;
	world.mvpData_->WVP = worldViewProjectionMatrix;


	ID3D12GraphicsCommandList* commandList = DirectXCommon::GetInstance()->GetCommandList().Get();

	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);

	//RootSignatureを設定。PSOに設定しているけど別途設定が必要
	commandList->SetGraphicsRootSignature(PipelineStateManager::GetInstance()->GetPipelineState(PipelineType::Model).rootSignature);
	// PSOを設定
	commandList->SetPipelineState(PipelineStateManager::GetInstance()->GetPipelineState(PipelineType::Model).graphicsPipelineState);


	commandList->IASetIndexBuffer(&indexBufferView);

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(1, world.GetWvpResource()->GetGPUVirtualAddress());

	if (light_ != nullptr) {
		commandList->SetGraphicsRootConstantBufferView(3, light_->GetResource()->GetGPUVirtualAddress());
	}

	commandList->SetGraphicsRootConstantBufferView(5, skinningParamResource_->GetGPUVirtualAddress());

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
