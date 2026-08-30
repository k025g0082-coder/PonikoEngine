#include "ExampleTitleScene.h"
#include "ExampleTitleEvent.h"

#include "ExampleGameScene.h"

#include "Game.h"

void ExampleTitleScene::Init() {

	// フェードアウト設定
	Game::GetFade().Start(Fade::Status::FadeIn, 1.0f);

	// テキストの初期化
	text.Init();
	text.setFont("default");

	text2.Init();
	text2.setFont("default");

}
void ExampleTitleScene::Update(GamePad* gamepad){

	// スペースキーでGameシーンに切り替える
	if (gamepad->TriggerKey(DIK_SPACE)) {
		Scene::RequestChange(new ExampleGameScene());
	}

}

void ExampleTitleScene::Draw(){

	text.SetFontSize(30);
	text.drawString("Example", { 0,0 });

	text2.SetFontSize(10);
	text2.drawString("Spaceでスタート", { 0,60 });

}