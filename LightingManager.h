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

	void AddLight(DirectionalLightParam p) { dLight.AddLight(p); }
	void AddLight(SpotLightParam p) { sLight.AddLight(p); };
	void AddLight(PointLightParam p) { pLight.AddLight(p); }

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

