#include "SpotLight.h"

#include "DirectXUtility.h"
#include "DirectXCommon.h"

// 設定
void SpotLight::Initialize(SpotLightParam param) {
	lightParam_ = param;

	// リソース作成
	resourceLight = DirectXUtility::CreateBufferResource(DirectXCommon::GetInstance()->GetDevice().Get(), sizeof(SpotLight));

	resourceLight->Map(0, nullptr, reinterpret_cast<void**>(&lightData_));

	// 各設定
	lightData_->color = lightParam_.color;
	lightData_->direction = lightParam_.direction;
	lightData_->intensity = lightParam_.intensity;
	lightData_->position = lightParam_.position;

	lightData_->innerAngle = lightParam_.innerAngle;
	lightData_->outerAngle = lightParam_.outerAngle;

	lightData_->distance = lightParam_.distance;
	lightData_->falloff = lightParam_.falloff;

}