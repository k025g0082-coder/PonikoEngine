#include "ExampleEffect.h"
#include "SceneComponentLoader.h"

void ExampleEffect::Initialize(Vector3 position) {

	Effect::Initialize(position);

	/*Particle::GetInstance()->RegionParticleModel("plane", "resources/Engine/plane.obj");*/

	// 使うモデルを登録（既にあるのは弾くのでそれは大丈夫）
	Effect::RegionParticleCompornent("plane", "resources/Engine/plane.obj");
	Effect::RegionParticleCompornent("plane2", "resources/Engine/plane.obj");
	Effect::RegionParticleCompornent("plane3", "resources/Engine/plane.obj");


	// 読込み
	if (SceneComponentLoader::GetInstance()->Load("resources/Effect/ExampleEffect.json")) {

		SceneComponentLoader* loder = SceneComponentLoader::GetInstance();

		// エフェクト内の使うパーティクルの情報のを全部ロード
		SceneComponentLoader::GetInstance()->EffectParticleCompornentLoad(particleEmitParams_);

	}


}

void ExampleEffect::Update() {

	Effect::Update();

	if (timer_ % 10 == 0) {

		/*
		param_.isBilliboard = true;
		Particle::GetInstance()->MakeNewParticle("plane", pos, 2, param_);
		*/

		// パーティクル放出
		Effect::EmitParticleCompornent("plane");
		Effect::EmitParticleCompornent("plane2");
	}

	// 終了処理
	if (timer_ > 600) {
		//isFinished_ = true;
	}

	timer_++;

#ifdef USE_IMGUI

	ImGui::Begin("EffectCompornentSave");

	if (ImGui::Button("Save")) {

		// ※注　ポインタ
		json& data = SceneComponentLoader::GetInstance()->GetJson();

		// 必ず
		SceneComponentLoader::GetInstance()->ClearJson();

		// エフェクト内の使うパーティクルの情報のを全部セーブ
		for (auto& [tag, parametor_] : particleEmitParams_) {
			SceneComponentLoader::GetInstance()->EffectParticleCompornentSave(tag, parametor_);
		}

		SceneComponentLoader::GetInstance()->Save("resources/Effect/ExampleEffect.json");

	}

	ImGui::End();
#endif
}