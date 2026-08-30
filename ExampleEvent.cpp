#include "ExampleEvent.h"

void ExampleEvent::Initialize() {

};

void ExampleEvent::Update() {

	// イベント発火
	scene_->visible = true;

}

void ExampleEvent::TriggerCheck() {

	// イベント条件
	if (scene_->timer >= 180) {
		isExecuted_ = true;
	}
}