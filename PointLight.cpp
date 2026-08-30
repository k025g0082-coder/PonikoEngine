#include "PointLight.h"

#include "DirectXUtility.h"
#include "DirectXCommon.h"

// 設定
void PointLight::Initialize(PointLightParam param) {
	lightParam_ = param;

	// リソース作成
	resourceLight = DirectXUtility::CreateBufferResource(DirectXCommon::GetInstance()->GetDevice().Get(), sizeof(PointLight));

	resourceLight->Map(0, nullptr, reinterpret_cast<void**>(&lightData_));

	// 各設定
	lightData_->color = lightParam_.color;
	lightData_->intensity = lightParam_.intensity;
	lightData_->position = lightParam_.position;
	lightData_->radius = lightParam_.radius;

}