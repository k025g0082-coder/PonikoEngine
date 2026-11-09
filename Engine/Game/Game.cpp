#include "Game.h"
#include "Scene.h"

#include "Text.h"

#include "PostEffectManager.h"

#include "SceneFactory.h"

Fade Game::fade;

uint32_t Game::kGameScreenWidth;
uint32_t Game::kGameScreenHeight;
Vector2 Game::kGameScreenOffset;

Game::WindowViewMode Game::currentViewMode_ = Game::WindowViewMode::kFullScreenMode;

Game::WindowViewParam Game::windowViewParams_[(int)WindowViewMode::count] = {

	// EditorMode
	{ 
		878,
		494,
		0,
		16.0f,

	}, 

	// FullScreenMode
	{
		1280,
		720,
		0,
		0,

	},

}; 

std::string Game::currentSceneName;

// シングルトン
Game* Game::GetInstance() {
	static Game game;
	return &game;
};

void Game::StartGameScene(std::string sceneName)
{
	Scene::StartGameScene(sceneName);
}

void Game::Init() {

	//SetWindowViewMode(currentViewMode_);

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

void Game::ResizeGameScreen(int32_t width, int32_t height, Vector2 offset)
{

	kGameScreenWidth = width;
	kGameScreenHeight = height;

	kGameScreenOffset = offset;

	// ポストエフェクト側のオフスクリーンの大きさも変更
	PostEffectManager::GetInstance()->ResizeOffscreen(width, height, offset);

}

void Game::SetGameScreenOffset(Vector2 v)
{
	kGameScreenOffset = v;

	// ポストエフェクト側のオフスクリーンの大きさも変更
	PostEffectManager::GetInstance()->SetViewOffset(v);
}
