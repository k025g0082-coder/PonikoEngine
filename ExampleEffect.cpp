#include "ExampleEffect.h"
#include "SceneComponentLoader.h"

void ExampleEffect::Initialize(Vector3 position) {

	Effect::Initialize(position);

	// 素材登録
	Particle::GetInstance()->RegionParticleModel("plane", "resources/Engine/plane.obj");

	Particle::GetInstance()->RegionParticleTexture("texture", "resources/Engine/uvChecker.png");
	Particle::GetInstance()->RegionParticleTexture("texture2", "resources/Engine/default.png");
	Particle::GetInstance()->RegionParticleTexture("texture3", "resources/white100x100.png");

	Particle::GetInstance()->RegionParticleTexture("star04", "resources/Engine/texture/emits/kennyParticles/star_04.png");

}

void ExampleEffect::Update() {

	Effect::Update();

	if (timer_ % 10 == 0) {

		// パーティクル放出

		Effect::EmitParticleCompornent("particle");
		Effect::EmitParticleCompornent("particle2");

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
