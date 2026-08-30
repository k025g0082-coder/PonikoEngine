#pragma once
#include "ExampleScene.h"
#include "Event.h"

class ExampleEvent : public Event {
public:

	// ポインタ受け取り
	ExampleEvent(ExampleScene* scene){
		scene_ = scene;
	}

	void Initialize() override;
	void Update() override;
	void TriggerCheck() override;

private:

	ExampleScene* scene_;

	float timer_ = 0.0f;

};

