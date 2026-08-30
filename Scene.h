#pragma once

#include "GamePad.h"

class Scene {

private:
	static Scene* instance;
	static Scene* nextInstance;
	static bool isChanging;

public:
	virtual void Init() = 0;
	virtual void Update(GamePad* gamepad) = 0;
	virtual void Draw() = 0;

	virtual ~Scene() = default;

	static void RequestChange(Scene* d) {
		nextInstance = d;
		isChanging = true;
	}

	static void ProcessChange();

	static Scene* getInstance() { return instance; }
	static void setInstance(Scene* d) {
		instance = d;
		instance->Init();
	}



};
