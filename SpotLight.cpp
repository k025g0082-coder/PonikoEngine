#include "SpotLight.h"

#include "DirectXUtility.h"
#include "DirectXCommon.h"

// 設定
void SpotLight::Initialize() {

	// リソース作成
	resourceLight = DirectXUtility::CreateBufferResource(DirectXCommon::GetInstance()->GetDevice().Get(), sizeof(SpotLightsParam));

	resourceLight->Map(0, nullptr, reinterpret_cast<void**>(&lightDatas_));

}