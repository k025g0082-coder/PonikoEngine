#include "DemoPlayer.h"

#include "DemoGimmick.h"

#include "Editor.h"

#include "Input.h"
#include "Collision.h"

void DemoPlayer::Initialize()
{}

void DemoPlayer::Update()
{
	// 共通
	DemoFieldObject::Update();

	float speed = 0.1f;
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

void DemoPlayer::CollisionToAllBlock()
{

	auto* layerNames_ = Editor::GetAlignHierarchyLayerNames();

	/* レイヤー名一覧から取得 */
for (std::string name : *layerNames_) {

		/* ヒエラルキ内の、シーン内の連携された全てのとAABB */
		for (auto& hierarchy : Editor::GetHierarchyModelLayer(name)) {

			if (hierarchy->gameObject_ == nullptr) continue;

			// Nullなら
				auto type = hierarchy->gameObject_->GetId();
				if (type != (uint32_t)DemoFieldObject::Type::kBlock) continue;// ブロックじゃなければReturn

				// FieldObjectのAABBを取得
					const AABB& targetAABB = hierarchy->gameObject_->GetAABB();

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

void DemoPlayer::ImGuiView()
{
	//
	DemoFieldObject::ImGuiView();
}
