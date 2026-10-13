#include "Game.h"
#include "Scene.h"

#include "Text.h"

Fade Game::fade;

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

}