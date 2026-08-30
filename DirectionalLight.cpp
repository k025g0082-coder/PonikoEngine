#include "DirectionalLight.h"

#include "DirectXUtility.h"
#include "DirectXCommon.h"

// 設定
void DirectionalLight::Initialize(DirectionalLightParam param) {

	lightParam_ = param;

	// リソース作成
	resourceLight = DirectXUtility::CreateBufferResource(DirectXCommon::GetInstance()->GetDevice().Get(), sizeof(DirectionalLight));

	resourceLight->Map(0, nullptr, reinterpret_cast<void**>(&lightData_));

	// 各設定
	lightData_->color = lightParam_.color;
	lightData_->direction = lightParam_.direction;
	lightData_->intensity = lightParam_.intensity;

}