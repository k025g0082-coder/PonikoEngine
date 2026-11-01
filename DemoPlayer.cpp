#include "DemoPlayer.h"

#include "Input.h"
#include "Collision.h"

void DemoPlayer::Update()
{
	// 共通
	GameObject::Update();

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

	/* ステージのオブジェクト内のすべてとのAABB */
	for (auto& hierarchy : *stageHierarchyList_) {

		// FieldObjectのAABBを取得
		const AABB& targetAABB = hierarchy->gameObject_->GetAABB();

		// AABB同士の衝突判定と押し戻し処理
		CollisionResult collisionResult = ResolveAABBCollision(aabb_, worldTransform.translation_, targetAABB);

		/* 結果を論理和 (OR) で積み重ねてく */
		collisionResult_.isGrounded |= collisionResult.isGrounded;
		collisionResult_.hitCeiling |= collisionResult.hitCeiling;
		collisionResult_.hitWall    |= collisionResult.hitWall;
	}

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

void DemoPlayer::ImGuiView()
{
	//

}
