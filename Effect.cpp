#include "Effect.h"
#include "ImGuiManager.h"

void Effect::Initialize(Vector3 position) {
	pos = position;
}

void Effect::Update() {

	// エフェクト内のImGuiコンポーネントを表示
	ImGuiViewParticleCompornent();
}

void Effect::RegionParticleCompornent(std::string tag, std::string path) {

	// 使うモデルを登録（既にあるのは弾くのでそれは大丈夫）
	Particle::GetInstance()->RegionParticleModel(tag, path);

	// もしpathが既に読み込まれてたものだったら、そのタグ名を返す　ないなら空
	std::string getParticleTag = Particle::GetInstance()->GetExistencePathToTag(path);

	// 見つかったら
	if (!getParticleTag.empty()) {
	
		// 登録
		particleEmitParams_[tag] = ParticleCompornent{
			.tag = getParticleTag,
			.position = {0,0},
			.num = 1,
			
		};
	}

}

// ImGui表示用
void Effect::ImGuiViewParticleCompornent() {

	if (particleEmitParams_.size() == 0) return;

#ifdef USE_IMGUI
	ImGui::Begin("EffectCompornents - Debug");

	ImGui::PushID(imguiId_);

	for (auto& [tag, parametor_] : particleEmitParams_) {

		if (ImGui::TreeNode(tag.c_str())) {

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
	}

	ImGui::PopID();

	ImGui::End();

#endif

}


