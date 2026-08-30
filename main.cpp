#include "PonikoEngine.h"

#include "ExampleTitleScene.h"

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int)
{

	PonikoEngine::Initialize(L"ぽにこエンジンn");


	// 初期Gameシーンの登録
	Game::GetInstance()->StartGameScene(new ExampleTitleScene);


	// メインループ！
	while (!WinApp::GetInstance()->ProcessMessage())
	{

		PonikoEngine::Update();
		PonikoEngine::Draw();
	
	}

	PonikoEngine::Finalize();

	return 0;
}
