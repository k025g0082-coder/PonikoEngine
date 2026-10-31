#include "GameObject.h"

//
GameObject::GameObject(Model* model, Camera* camera, const GameObjectData& data_)
{
	// オブジェクトごとにIDを持たせる
	objectID_ = countId_++;


	model_ = model;
	camera_ = camera;

	//
	aabb_ = data_.aabb_;


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

	// SRTからviewMatrix生成して更新
	worldTransform.UpdateMatrix();

}
void GameObject::Draw()
{
	/* worldTransform基に描画 */
	model_->Draw(worldTransform, *camera_);
}

