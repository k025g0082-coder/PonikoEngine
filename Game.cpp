#include "Game.h"
#include "Scene.h"

#include "Text.h"

Fade Game::fade;

std::list<Event*> Game::events_;

// シングルトン
Game* Game::GetInstance() {
	static Game game;
	return &game;
};

void Game::Init() { 

	// 入力の初期化
	gamepad.Initialize();

	fade.Initialize();

	// フォントロード
	// デフォルトフォント登録
	Text::RegisterFont("default", "resources/Engine/default.ttf", 32, 32);
}

void Game::Update() {

	Scene::ProcessChange();

	// イベントアプデ
	for (Event* e_ : events_) {

		// 条件確認
		e_->TriggerCheck();

		if (!e_->IsExecuted()) {
		
			// 条件満たしてないなら
			continue;
		}

		// アプデ
		e_->Update();
	}

	// 各画面のアプデ
	Scene::getInstance()->Update(&gamepad);

	fade.Update();
	gamepad.Update();

}
void Game::Draw() {

	// 各画面の描画
	Scene::getInstance()->Draw();

	fade.Draw();

}

void Game::Finalize() {

	for (Event* event : events_) {
		delete event;
	}

	events_.clear();

}