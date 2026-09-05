#include "ExampleEffect.h"
#include "SceneComponentLoader.h"

void ExampleEffect::Initialize(Vector3 position) {

	Effect::Initialize(position);

	// 素材登録
	Particle::GetInstance()->RegionParticleModel("plane", "resources/Engine/plane.obj");
	Particle::GetInstance()->RegionParticleTexture("texture", "resources/Engine/uvChecker.png");
	Particle::GetInstance()->RegionParticleTexture("texture2", "resources/Engine/default.png");

	//Effect::RegionParticleCompornent("particle", "plane","texture");

	// 使うモデルを登録（既にあるのは弾くのでそれは大丈夫）
	//Effect::RegionParticleCompornent("plane", "resources/Engine/plane.obj");
	//Effect::RegionParticleCompornent("plane2", "resources/Engine/plane.obj");
	//Effect::RegionParticleCompornent("plane3", "resources/Engine/plane.obj");

}

void ExampleEffect::Update() {

	Effect::Update();

	if (timer_ % 10 == 0) {

		/*
		param_.isBilliboard = true;
		Particle::GetInstance()->MakeNewParticle("plane", pos, 2, param_);
		*/

		// モデル登録
		//Particle::GetInstance()->RegionParticleModel("plane", "resources/Engine/plane.obj");

		// パーティクル放出
		Effect::EmitParticleCompornent("a");

		Effect::EmitParticleCompornent("particle");

		//Effect::EmitParticleCompornent("plane2");
	}

	// 終了処理
	if (timer_ > 600) {
		//isFinished_ = true;
	}

	timer_++;
//
//#ifdef USE_IMGUI
//
//	ImGui::Begin("EffectCompornentSave");
//
//	
//
//	ImGui::End();
//#endif
}
