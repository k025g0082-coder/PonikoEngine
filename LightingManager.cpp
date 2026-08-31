#include "LightingManager.h"

LightingManager* LightingManager::GetInstance() {
	static LightingManager instance;
	return &instance;
}

void  LightingManager::Initialize() {

	dLight.Initialize();
	sLight.Initialize();
	pLight.Initialize();
}