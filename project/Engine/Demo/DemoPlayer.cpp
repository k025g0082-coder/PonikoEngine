#include "DemoPlayer.h"

#include "DemoGimmick.h"

#include "Editor.h"

#include "Input.h"
#include "Collision.h"

#include "SceneComponentLoader.h"

#include "EffectManager.h"
#include "ExampleEffect.h"

#include "timer.h"


void DemoPlayer::Initialize()
{
	//
	beginPosition_ = worldTransform.translation_;


	// 赤
	model_->ModelColor({ 1.0f,1.0f,0,1.0f });


	// パーチクル用 - テキスチャ読込
	Particle::GetInstance()->RegionParticleModel("plane", "resources/Engine/plane.obj");
	Particle::GetInstance()->RegionParticleTexture("smoke", "resources/EffectTexture/smoke.png");




	//  パーチクル用 -　データ読込
	if (SceneComponentLoader::GetInstance()->Load("resources/Effect/ParticleProp.json")) {

		// ※注　ポインタ
		SceneComponentLoader* loder = SceneComponentLoader::GetInstance();

		loder->BeginGroup("particle");

		loder->Property("playerMoveEmit", moveEmitParam);
		loder->Property("playerGetEmit", getEmitParam);
		
		loder->EndGroup();

		/* 読込 */
		loder->LoadGroup("particle");
	}
}

void DemoPlayer::Update()
{
	// 共通
	DemoFieldObject::Update();

	float speed = 0.2f;
	Vector3 move{ 0,0,0 };

	/* 左右 */
	if (Input::GetInstance()->PushKey(DIK_A)) {
		move.x -= speed;
	}
	if (Input::GetInstance()->PushKey(DIK_D)) {
		move.x += speed;
	}

	/* 奥 */
	if (Input::GetInstance()->PushKey(DIK_W)) {
		move.z += speed;
	}
	if (Input::GetInstance()->PushKey(DIK_S)) {
		move.z -= speed;
	}

	/*
		動いた
	*/
	if (move.x != 0.0f || move.z != 0.0f) {

		/* エフェクト用のプレイヤーの下ら辺を設定 */

		// 大きさはスケール基準
		float w = 1.0f;
		float h = 1.0f;
		float d = 1.0f;


		float footY = worldTransform.translation_.y - worldTransform.scale_.y / 2.0f;


		AABB frontAABB;

		frontAABB.min = {
				worldTransform.translation_.x - w / 2.0f,
				footY,
				worldTransform.translation_.z - d / 2.0f
		};

		frontAABB.max = {
			worldTransform.translation_.x + w / 2.0f,
			footY + h,
			worldTransform.translation_.z + d / 2.0f
		};

		//
		emitElapsedTimer += Timer::GetDeltaTime();

		/*  動いたら一定間隔エミット */
		if (emitElapsedTimer >= emitDistance) {
			Particle::GetInstance()->MakeNewParticle("plane", "smoke", frontAABB, 1, moveEmitParam);
			emitElapsedTimer -= emitDistance;
		}

	}
	// ジャンプ
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {

		// 地面のとき
		if (collisionResult_.isGrounded) {
			velocity_.y += kJumpAcceleration;
		}
	}

	// 空中
	if (!collisionResult_.isGrounded) {
		velocity_.y -= kGravityAcceleration;	
	}

	// 移動
	worldTransform.translation_ += move;
	worldTransform.translation_ += velocity_;

	aabb_.UpdatePosition(worldTransform.translation_);

	collisionResult_ = { false, false, false };


	// 押し戻し、各判定
	CollisionToAllBlock();


	// 地面についた
	if (collisionResult_.isGrounded) {
		velocity_.y = 0.0f;
	}
}

void DemoPlayer::Draw()
{
	// 共通
	GameObject::Draw();

}

void DemoPlayer::Respown()
{
	worldTransform.translation_ = beginPosition_;
	velocity_ = { 0 };
}

void DemoPlayer::CollisionToAllBlock()
{

	auto* layerNames_ = Editor::GetAlignHierarchyLayerNames();

	/* レイヤー名一覧から取得 */
	for (std::string name : *layerNames_) {

		/* ヒエラルキ内の、シーン内の連携された全てのとAABB */
		for (auto& hierarchy : Editor::GetHierarchyModelLayer(name)) {

			if (hierarchy->type_ != HierarchyType::kModel)
				continue;

			auto* model = dynamic_cast<ModelNode*>(hierarchy);


			if (!model->gameObject_) continue;

				auto type = model->gameObject_->GetId();
				if (type != (uint32_t)DemoFieldObject::Type::kBlock) continue;// ブロックじゃなければReturn

				// FieldObjectのAABBを取得
				const AABB& targetAABB = model->gameObject_->GetAABB();

				// AABB同士の衝突判定と押し戻し処理
				CollisionResult collisionResult = ResolveAABBCollision(aabb_, worldTransform.translation_, targetAABB, true);

				/* 結果を論理和 (OR) で積み重ねてく */
				collisionResult_.isGrounded |= collisionResult.isGrounded;
				collisionResult_.hitCeiling |= collisionResult.hitCeiling;
				collisionResult_.hitWall |= collisionResult.hitWall;

			}
		}
	}

void DemoPlayer::onCollisionToGimmick(DemoGimmick* gimmick,CollisionResult result)
{
	(void)result;
	uint32_t id = gimmick->GetSubId();

	switch ((DemoGimmick::GimmickType)id) {

		case DemoGimmick::GimmickType::kSwitch:
			// スイッチ当たった
			id;
		break;

	}

}

void DemoPlayer::onCollisionToItem(DemoFieldItem* item, CollisionResult result)
{
	(void)result;
	uint32_t id = item->GetSubId();

	switch ((DemoFieldItem::ItemType)id) {

	case DemoFieldItem::ItemType::kCoin:
		// コインに当たった

		// エフェクト
		Particle::GetInstance()->MakeNewParticle("plane", "symbol_02", worldTransform.translation_, 6, getEmitParam);

		break;

	}
}

void DemoPlayer::ImGuiView()
{

	//moveEmitParam.ImGuiView();
	getEmitParam.ImGuiView();
	if (ImGui::Button("getVfx")) {
		//Particle::GetInstance()->MakeNewParticle("plane", "symbol_02", worldTransform.translation_, 6, getEmitParam);
		EffectManager::AddEffect(new ExampleEffect, worldTransform.translation_);
	}
	if (ImGui::Button("Save")) {

		SceneComponentLoader* loder = SceneComponentLoader::GetInstance();

		// jsonに
		if (loder->SaveSwitchJson("resources/Effect/ParticleProp.json")) {

			/* 登録 */
			loder->SaveGroup("particle");

			// JSONファイルに保存する
			loder->Save("resources/Effect/ParticleProp.json");
		}
	}
	//
	//DemoFieldObject::ImGuiView();
}
