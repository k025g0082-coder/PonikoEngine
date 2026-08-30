#include "ExampleGameEvent.h"
void ExampleGameEvent::Initialize() {

};

void ExampleGameEvent::Update() {

	// イベント発火
	scene_->visible = true;

}

void ExampleGameEvent::TriggerCheck() {

	// イベント条件
	if (scene_->timer >= 180) {
		isExecuted_ = true;
	}
}