#include "GameObject.h"
#include "Collision.h"

//
GameObject::GameObject(Model* model, Camera* camera, const GameObjectData& data_)
{
	// オブジェクトごとにIDを持たせる
	/*objectID_ = countId_++;*/

	objectID_ = data_.objectID;
	objectSubID_ = data_.subObjectID;

	model_ = model;
	camera_ = camera;

	//
	aabb_ = data_.aabb_;

	
	hierarchy = data_.hierarchy;//



	/* ワールド初期化 */
	worldTransform.Initialize();

	worldTransform.translation_ = data_.position;
	worldTransform.rotation_ = data_.rotation;
	worldTransform.scale_ = data_.scale;

	worldTransform.UpdateMatrix();
}


void GameObject::Update()
{
	model_->Update();

	aabb_.UpdatePosition(worldTransform.translation_);

	// SRTからviewMatrix生成して更新
	worldTransform.UpdateMatrix();

}
void GameObject::Draw()
{
	/* worldTransform基に描画 */
	model_->Draw(worldTransform, *camera_);
}

void GameObject::Collision(GameObject* a, GameObject* b)
{
	if (IsCollision(a->GetAABB(), b->GetAABB())) {

		a->onCollision(b);
		b->onCollision(a);

	}
}

