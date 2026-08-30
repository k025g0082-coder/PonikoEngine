#pragma once
#include "ExampleTitleScene.h"
#include "Event.h"

class ExampleTitleEvent : public Event {
public:

	// ポインタ受け取り
	ExampleTitleEvent(ExampleTitleScene* scene){
		scene_ = scene;
	}

	void Initialize() override;
	void Update() override;
	void TriggerCheck() override;

private:

	ExampleTitleScene* scene_;

	float timer_ = 0.0f;

};

