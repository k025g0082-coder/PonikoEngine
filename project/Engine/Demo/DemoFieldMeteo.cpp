#include "DemoFieldMeteo.h"
#include "Scene.h"
#include "SceneComponentLoader.h"

#include "Random.h"
#include "Timer.h"

#include "ImGuiManager.h"

Vector3 DemoFieldMeteo::velocity = {-0.1f,-0.4f,0.3f};

void DemoFieldMeteo::Initialize(Model* model, Vector3 pos, ParticleParam& meteoEmitParam, ParticleParam& meteoBreakEmitParam)
{

	model_ = model;

	meteoEmitParam_ = &meteoEmitParam;
	meteoBreakEmitParam_ = &meteoBreakEmitParam;


	/* 色はランダム */
	model_->ModelColor({
		Random(0.0f,1.0f),
		Random(0.0f,1.0f),
		Random(0.0f,1.0f),
		1.0f
	});

	// 大きさもランダム
	float s = Random(1.0f, 3.0f);
	world_.scale_ = { s,s,s };

	// 判定用の大きさも設定する
	aabb_.width_ = s;
	aabb_.height_ = s;
	aabb_.depth_ = s;


	world_.Initialize();
	world_.translation_ = pos;


	/* 
		パーティクル用 - テキスチャ読込
	*/
	Particle::GetInstance()->RegionParticleModel("block", "resources/Models/block/cube.obj");
	Particle::GetInstance()->RegionParticleTexture("white", "resources/Models/block/cube.png");



}

void DemoFieldMeteo::Update()
{

	if (!isBreak_)
	{

		/* 移動 */
		world_.translation_ += velocity;


		float emitDistance = 0.1f;

		/* 落下中は定期的にエミット */
		if (emitElapsedTimer >= emitDistance) {
			Particle::GetInstance()->MakeNewParticle("block", "white", world_.translation_, 6, *meteoEmitParam_);
			emitElapsedTimer -= emitDistance;
		}
		emitElapsedTimer += Timer::GetDeltaTime();

	}
	else {

	

	}


	aabb_.UpdatePosition(world_.translation_);

	world_.UpdateMatrix();
	model_->Update();


	// もし基底時間が経過で削除
	if (timer_ > kLimit) { isDelete_ = true; }

	// 計測
	timer_ += Timer::GetDeltaTime();
	
}
void DemoFieldMeteo::Draw()
{
	if (!isBreak_)
		model_->Draw(world_, * Scene::GetSceneCamera());
}

void DemoFieldMeteo::Break()
{
	isBreak_ = true;

	// 破壊エフェクト
	Particle::GetInstance()->MakeNewParticle("block", "white", world_.translation_, 6, *meteoBreakEmitParam_);
}
