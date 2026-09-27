#pragma once

#include "Model.h"
#include "Camera.h"

#include "ParticleEmit.h"

#include "ImGuiManager.h"

#include <map>

// シェーダーに転送用
struct ParticleData {

	Matrix4x4 WVP;
	Matrix4x4 World;

	Vector4 color;

	uint32_t textureIndex;// 描画に使うテクスチャのSRVの番号

};

struct ParticleModelData {

	// モデルデータ
	ModelData modelData;

	// テクスチャ（のアドレス）
	Texture* texture = nullptr;

	std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> textureSrvHandles;


	D3D12_INDEX_BUFFER_VIEW indexBufferView{};// 頂点ビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

	Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource;

	ID3D12Resource* indexResource;

	// マテリアル用のリソース
	ComPtr <ID3D12Resource> materialResource;
	ComPtr <ID3D12Resource> vertexResource;

	D3D12_CPU_DESCRIPTOR_HANDLE instancingSrvHandleCPU;
	D3D12_GPU_DESCRIPTOR_HANDLE instancingSrvHandleGPU;// 描画時に必要


	//書き込むためのアドレス
	ParticleData* instancingData = nullptr;
	std::string modelPath;

	// 描画してる数
	int currentDrawNum = 0;
};

/// <summary>
/// <param name="color"> 色 </param>
/// <param name="speed"> 速度 </param>
/// <param name="life"> 生存時間 </param>
/// <param name="isBilliboard"> 常に前向くか（板ポリ用） </param>
/// <param name="isDilute"> 色を薄くしていくか </param>
/// </summary>

struct ParticleParam {

	// 色
	Vector4 color = { 1.0f,1.0f,1.0f,1.0f };
	
	// スピード
	float speed = 1.0f;
	
	// 生成時のスピードの幅
	float speedAmp = 0.0f;

	// 重力(0.25f～)
	float gravity = 0.0f;

	// 生成時の最小の小ささ(1.0f基準)
	float scaleAmpMin = 1.0f;

	// 生成時の最大の大きさ(1.0f基準)
	float scaleAmpMax = 1.0f;

	// 生存時間
	int life = 20;

	// 常にカメラ方向を向くか
	bool isBilliboard = false;

    // 色を薄くしていくか
	bool isDilute = true;


	// ImGuiデバッグ用
	void ImGuiView() {

#ifdef USE_IMGUI
			ImGui::ColorEdit4("Color", &color.x);

			ImGui::DragFloat("Speed", &speed, 0.01f, 0.0f, 10.0f);
			ImGui::DragFloat("Speed Amp", &speedAmp, 0.01f, 0.0f, 10.0f);

			ImGui::DragFloat("Gravity", &gravity, 0.01f, 0.0f, 10.0f);

			ImGui::DragFloat("Scale Min", &scaleAmpMin, 0.01f, 0.1f, scaleAmpMax);
			ImGui::DragFloat("Scale Max", &scaleAmpMax, 0.01f, scaleAmpMin, 10.0f);

			ImGui::InputInt("Life", &life);

			ImGui::Checkbox("Billboard", &isBilliboard);
			ImGui::Checkbox("Dilute", &isDilute);
#endif
	
	}
};

class Particle {
public:

	// シングルトゥン
	static Particle* GetInstance();

	void Initialize();

	void Update();

	void Draw();

	// パーティクル用モデル登録関数
	void RegionParticleModel(std::string tag,std::string path);

	// パーティクル用テキスチャ登録関数
	void RegionParticleTexture(std::string tag, std::string path);


	// 生成
	void MakeNewParticle(std::string tag, std::string textureTag, Vector3 pos, int num, ParticleParam param);


	// 登録済のパーティクルモデルのパスゲット   どうせ登録でなかったら弾いてるのでいいでしょこれで
	std::string GetModelPath(std::string name) {
		return particleModelDatas_[name].modelPath;
	};
	// 登録済のパーティクルモデルのパスゲット   どうせ登録でなかったら弾いてるのでいいでしょこれで
	std::string GetTexturePath(std::string name) {
		return particleModelDatas_[name].modelPath;
	};

	// 登録済のパーティクルモデルの名前表ゲット
	std::vector<std::string> GetParticleModelList() {

		std::vector<std::string> list;

		for (const auto& [tag, data] : particleModelDatas_) {
			list.push_back(tag);
		}

		return list;

	};
	// 登録済のパーティクルテキスチャーの名前表ゲット
	std::vector<std::string> GetParticleTextureList() {

		std::vector<std::string> list;

		for (const auto& [tag, data] : particleTextureDatas_) {
			list.push_back(tag);
		}

		return list;

	};

	// Path存在チェック見つかったらタグ名で返して　みつかんなかったら
	std::string GetExistencePathToTag(std::string path) {

		for (const auto& [tag, data] : particleModelDatas_) {
			if (data.modelPath == path) {
				return tag;
				break;
			}
		}

		return "";
	}

	void Draw(std::string tag);

	// 描画に使うカメラをセットする
	void SetCamera(Camera* camera) { camera_ = camera; }

	// パーティクル用モデル辞典
	std::map<std::string, ParticleModelData> particleModelDatas_;

	// パーティクル用テキスチャ辞典
	std::map<std::string, Texture*> particleTextureDatas_;


private:

	// 生成
	void MakeNewParticle(Vector3 pos,int num, ParticleParam param);


	// ひとモデルの最大描画数
	static const uint32_t kNumInstance = 100;

	// すべてのモデルの最大描画数
	static const uint32_t kNumMaxInstances = 1000;


	// 現在のすべてのエミット
	ParticleEmit emits[kNumMaxInstances];

	Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource;

	//書き込むためのアドレス
	ParticleData* instancingData = nullptr;

	D3D12_CPU_DESCRIPTOR_HANDLE instancingSrvHandleCPU;
	D3D12_GPU_DESCRIPTOR_HANDLE instancingSrvHandleGPU;

	// 色
	Material* materialData = nullptr;

	// モデルデータ
	ModelData modelData;
	
	// テクスチャ
	Texture* texture = nullptr;

	ModelVertexData* vertexData = nullptr;


	// マテリアル用のリソース
	ComPtr <ID3D12Resource> materialResource;

	ID3D12Resource* indexResource;
	// 頂点リソース
	ComPtr <ID3D12Resource> vertexResource;

	D3D12_INDEX_BUFFER_VIEW indexBufferView{};
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

	// 描画する際のカメラ
	Camera* camera_ = nullptr;

};