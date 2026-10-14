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

	DirectionalLightParam& AddLight(DirectionalLightParam p) { return dLight.AddLight(p); }
	SpotLightParam& AddLight(SpotLightParam p) { return sLight.AddLight(p); };
	PointLightParam& AddLight(PointLightParam p) { return pLight.AddLight(p); }

	void RemoveLight(DirectionalLightParam* p) { return dLight.RemoveLight(p); }
	void RemoveLight(SpotLightParam* p) { return sLight.RemoveLight(p); };
	void RemoveLight(PointLightParam* p) { return pLight.RemoveLight(p); }


	void ResetLight() {

		dLight.ResetLight();
		sLight.ResetLight();
		pLight.ResetLight();
	}
	
	LightSets GetLightSets() {
		return LightSets{ &dLight,&sLight, &pLight };
	}

private:
	
	DirectionalLight dLight;
	SpotLight sLight;
	PointLight pLight;
};

