#include "ExampleGameScene.h"
#include "ExampleGameEvent.h"

#include "Game.h"

void ExampleGameScene::Init() {

	// フェードアウト設定
	Game::GetFade().Start(Fade::Status::FadeIn, 1.0f);

	//イベント
	Game::GetInstance()->EventToRegion(new ExampleGameEvent(this));

	text.Init();
	text.setFont("default");

	eventText.Init();
	eventText.setFont("default");

}
void ExampleGameScene::Update(GamePad* gamepad) {
	timer++;
}
void ExampleGameScene::Draw() {

	text.SetFontSize(10);
	text.drawString("Event: 条件はtimer >= 180", { 0,0 });

	if (visible) {
		eventText.SetFontSize(20);
		eventText.drawString("イベント実行されました！", { 0,60 });
	}
}