#pragma once
#include "DirectionalLight.h"
#include "SpotLight.h"
#include "PointLight.h"

// モデルにあげる用
struct LightSets {
	DirectionalLight* dLight;
	SpotLight* sLight;
	PointLight* pLight;
};

class LightingManager
{
public:
	// シングルトン
	static LightingManager* GetInstance();

	void Initialize();

	DirectionalLightParam& AddLight(DirectionalLightParam p,bool isSceneLight = false) { return dLight.AddLight(p,isSceneLight); }
	SpotLightParam& AddLight(SpotLightParam p,bool isSceneLight = false) { return sLight.AddLight(p,isSceneLight); };
	PointLightParam& AddLight(PointLightParam p,bool isSceneLight = false) { return pLight.AddLight(p,isSceneLight); }

	void RemoveLight(DirectionalLightParam* p) { return dLight.RemoveLight(p); }
	void RemoveLight(SpotLightParam* p) { return sLight.RemoveLight(p); };
	void RemoveLight(PointLightParam* p) { return pLight.RemoveLight(p); }


	void ResetLight() {

		dLight.ResetLight();
		sLight.ResetLight();
		pLight.ResetLight();
	}
	
	bool GetIsSceneLight(DirectionalLightParam* d) { return dLight.GetIsSceneLight(d); }
	bool GetIsSceneLight(SpotLightParam* s) { return sLight.GetIsSceneLight(s); }
	bool GetIsSceneLight(PointLightParam* p) { return pLight.GetIsSceneLight(p); }


	LightSets GetLightSets() {
		return LightSets{ &dLight,&sLight, &pLight };
	}

private:
	
	DirectionalLight dLight;
	SpotLight sLight;
	PointLight pLight;
};

