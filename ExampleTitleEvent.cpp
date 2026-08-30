#include "ExampleTitleEvent.h"

void ExampleTitleEvent::Initialize() {

};

void ExampleTitleEvent::Update() {

	// イベント発火
	scene_->visible = true;

}

void ExampleTitleEvent::TriggerCheck() {

	// イベント条件
	if (scene_->timer >= 180) {
		isExecuted_ = true;
	}
}