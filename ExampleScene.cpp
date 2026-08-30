#include "ExampleScene.h"

#include "Game.h"
#include "ExampleEvent.h"

void ExampleScene::Init() {

	// フェードアウト設定
	Game::GetFade().Start(Fade::Status::FadeIn, 1.0f);

	//イベント
	Game::GetInstance()->EventToRegion(new ExampleEvent(this));

	text.Init();
	text.setFont("default");

	eventText.Init();
	eventText.setFont("default");

}
void ExampleScene::Update(GamePad* gamepad){
	timer++;
}
void ExampleScene::Draw(){

	text.SetFontSize(10);
	text.drawString("timer >= 180", { 0,0 });

	if (visible) {
		eventText.SetFontSize(20);
		eventText.drawString("eventText", { 0,60 });
	}
}