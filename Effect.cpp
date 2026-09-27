#include "Effect.h"
#include "ImGuiManager.h"

#include "SceneComponentLoader.h"

void Effect::Initialize(Vector3 position) {

	pos = position;

	// コンポーネントの読込み
	if (SceneComponentLoader::GetInstance()->Load("resources/Effect/" + effectName + ".json")) {

		SceneComponentLoader* loder = SceneComponentLoader::GetInstance();

		// エフェクト内の使うパーティクルの情報のを全部ロード
		SceneComponentLoader::GetInstance()->EffectParticleCompornentLoad(particleEmitParams_);

	}
}

void Effect::Update() {

	// エフェクト内のImGuiコンポーネントを表示
	ImGuiViewParticleCompornent();
}

void Effect::RegionParticleCompornent(std::string tag, std::string particleModelTag, std::string particleTextureTag) {
	//void Effect::RegionParticleCompornent(std::string tag, std::string path) {

		// もし既に登録されているタグがあれば帰る
	if (particleEmitParams_.contains(tag)) return;

	// 使うモデルを登録（既にあるのは弾くのでそれは大丈夫）
	//Particle::GetInstance()->RegionParticleModel(tag, path);

	// もしpathが既に読み込まれてたものだったら、そのタグ名を返す　ないなら空
	//std::string getParticleTag = Particle::GetInstance()->GetExistencePathToTag(path);

	// 見つかったら
//	if (!getParticleTag.empty()) {

		// 登録
	particleEmitParams_[tag] = ParticleCompornent{

		.tag = particleModelTag,
		.textureTag = particleTextureTag,

		.position = {0,0},
		.num = 1,

	};
	//	}
}


ParticleCompornent* Effect::GetParticleCompornent(std::string tag) {

	// もしタグなけりゃサヨナラ
	if (!particleEmitParams_.contains(tag)) return nullptr;

	return &particleEmitParams_[tag];
}

// ImGui表示用
void Effect::ImGuiViewParticleCompornent() {

	if (particleEmitParams_.size() == 0) return;

	// デバッグでなければ帰る
	if (!isDebugMode_) return;

#ifdef USE_IMGUI

	ImGui::Begin("EffectCompornents - Debug");


	// === セーブボタン
	if (ImGui::Button("Save")) {

		// ※注　ポインタ
		json& data = SceneComponentLoader::GetInstance()->GetJson();

		// 必ず
		SceneComponentLoader::GetInstance()->ClearJson();

		// エフェクト内の使うパーティクルの情報のを全部セーブ
		for (auto& [tag, parametor_] : particleEmitParams_) {
			SceneComponentLoader::GetInstance()->EffectParticleCompornentSave(tag, parametor_);
		}

		SceneComponentLoader::GetInstance()->Save("resources/Effect/" + effectName + ".json");

	}

	ImGui::SameLine(ImGui::GetWindowWidth() - 80.0f);


	// === 既に登録済みのパーティクルモデルデータを使用して増やす
	if (ImGui::Button("Create")) {

		ImGui::OpenPopup("AddParticle");

	}

	static std::string selectedModelTag;
	static std::string selectedTextureTag;

	if (ImGui::BeginPopup("AddParticle")) {

		static char tag_[64] = "";

		bool modelSelected = false;
		bool textureSelected = false;

		ImGui::InputText("Tag", tag_, sizeof(tag_));


		// モデル選択中の名前
		const char* modelPreview = selectedModelTag.empty()
			? "Select Particle"
			: selectedModelTag.c_str();

		if (ImGui::BeginCombo("Model", modelPreview)) {

			// Particle側の登録済みタグを回す
			for (const auto& tag : Particle::GetInstance()->GetParticleModelList()) {

				modelSelected = (selectedModelTag == tag);

				if (ImGui::Selectable(tag.c_str(), modelSelected)) {
					selectedModelTag = tag;
				}

				if (modelSelected) {
					ImGui::SetItemDefaultFocus();
				}
			}

			ImGui::EndCombo();
		}

		// テキスチャ選択中の名前
		const char* texturePreview = selectedTextureTag.empty()
			? "Select Particle"
			: selectedTextureTag.c_str();

		if (ImGui::BeginCombo("Texture", texturePreview)) {

			// Particle側の登録済みタグを回す
			for (const auto& tag : Particle::GetInstance()->GetParticleTextureList()) {

				textureSelected = (selectedTextureTag == tag);

				if (ImGui::Selectable(tag.c_str(), textureSelected)) {
					selectedTextureTag = tag;
				}

				if (textureSelected) {
					ImGui::SetItemDefaultFocus();
				}
			}

			ImGui::EndCombo();
		}



		// 追加
		if (ImGui::Button("Add")) {

			// 空チェック
			if (tag_[0] != '\0' && !selectedModelTag.empty() && !selectedTextureTag.empty()) {

				RegionParticleCompornent(
					tag_,
					selectedModelTag,
					selectedTextureTag
				);
				
				ImGui::CloseCurrentPopup();
			}
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel")) {
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}


	// === 一覧表示

	ImGui::PushID(imguiId_);

	for (auto it = particleEmitParams_.begin();
		it != particleEmitParams_.end();)
	{
		auto& [tag, parametor_] = *it;

		bool open = ImGui::TreeNode(tag.c_str());

		ImGui::SameLine(ImGui::GetWindowWidth() - 80.0f);

		// 削除ボタンを右寄りに配置（↑のおかげ
		bool deleteButton = ImGui::Button("delete");


		if (open) {

			ImGui::DragFloat3(
				"position_",
				&parametor_.position.x,
				0.1f
			);

			ImGui::DragInt(
				"num_",
				&parametor_.num,
				1.0f,
				0
			);

			if (ImGui::TreeNode("param")) {

				parametor_.param_.ImGuiView();

				ImGui::TreePop();
			}

			ImGui::TreePop();
		}

		ImGui::Dummy(ImVec2{ 0, 5 });
		ImGui::Separator();
		ImGui::Dummy(ImVec2{ 0, 5 });

		//　消す
		if (deleteButton) {
			it = particleEmitParams_.erase(it);
		}
		else {
			++it;
		}

	}

	ImGui::PopID();

	ImGui::End();

#endif

}


