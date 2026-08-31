#include "PointLight.h"

#include "DirectXUtility.h"
#include "DirectXCommon.h"

// 設定
void PointLight::Initialize() {

	// リソース作成
	resourceLight = DirectXUtility::CreateBufferResource(DirectXCommon::GetInstance()->GetDevice().Get(), sizeof(PointLightsParam));

	resourceLight->Map(0, nullptr, reinterpret_cast<void**>(&lightDatas_));

}