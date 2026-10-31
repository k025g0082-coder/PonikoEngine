#include "DemoPlayer.h"

#include "Input.h"

void DemoPlayer::Update()
{
	// 共通
	GameObject::Update();

	velocity_ = { 0 };

	float speed = 0.05f;

	/* 左右 */
	if (Input::GetInstance()->PushKey(DIK_A)) {
		velocity_.x += speed;
	}
	if (Input::GetInstance()->PushKey(DIK_D)) {
		velocity_.x -= speed;
	}

	/* 奥 */
	if (Input::GetInstance()->PushKey(DIK_W)) {
		velocity_.z -= speed;
	}
	if (Input::GetInstance()->PushKey(DIK_S)) {
		velocity_.z += speed;
	}

	/* ここにFieldObjectとのAABB接触押し戻し判定を書く？？ */

	// 移動
	worldTransform.translation_ += velocity_;
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
