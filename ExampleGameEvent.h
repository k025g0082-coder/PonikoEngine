#pragma once
#include "ExampleGameScene.h"
#include "Event.h"

class ExampleGameEvent : public Event {
public:

	// ポインタ受け取り
	ExampleGameEvent(ExampleGameScene* scene) {
		scene_ = scene;
	}

	void Initialize() override;
	void Update() override;
	void TriggerCheck() override;

private:

	ExampleGameScene* scene_;

	float timer_ = 0.0f;

};

