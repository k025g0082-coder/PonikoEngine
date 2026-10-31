#include "DemoFieldObject.h"

//
void DemoFieldObject::Initialize()
{

	// なんかあれば

}
void DemoFieldObject::Update()
{
	model_->Update();

	// SRTからviewMatrix生成して更新
	worldTransform.UpdateMatrix();

}
void DemoFieldObject::Draw()
{
	/* worldTransform基に描画 */
	model_->Draw(worldTransform, *camera_);
}

