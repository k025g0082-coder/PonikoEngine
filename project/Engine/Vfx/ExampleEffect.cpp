#include "ExampleEffect.h"
#include "SceneComponentLoader.h"

void ExampleEffect::Initialize(Vector3 position) {

	Effect::Initialize(position);

	// 素材登録
	Particle::GetInstance()->RegionParticleModel("plane", "resources/Engine/plane.obj");

	Particle::GetInstance()->RegionParticleTexture("circle_05", "resources/Engine/texture/emits/kennyParticles/circle_05.png");
	Particle::GetInstance()->RegionParticleTexture("star06", "resources/Engine/texture/emits/kennyParticles/star_06.png");

}

void ExampleEffect::Update() {

	Effect::Update();

	if (timer_ % 10 == 0) {

		// パーティクル放出

		Effect::EmitParticleCompornent("particle");
		Effect::EmitParticleCompornent("test");

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
