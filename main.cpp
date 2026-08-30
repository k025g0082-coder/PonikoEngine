#include "PonikoEngine.h"

#include "TestScene.h"


/*

	exeデバッグ用　

	構成プロパティー＞全般から出力ディレクトリを libー＞exe に
			構成の種類をlibー＞exe にして実行

	【重要】きりがいいところで　構成プロパティー＞全般から出力ディレクトリを exeー＞lib に
			構成の種類をexeー＞lib にする！！

*/

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int)
{

	PonikoEngine::Initialize(L"ぽにこエンジンn");


	// 初期Gameシーンの登録
	Game::GetInstance()->StartGameScene(new TestScene);


	// メインループ！
	while (!WinApp::GetInstance()->ProcessMessage())
	{

		PonikoEngine::Update();
		PonikoEngine::Draw();
	
	}

	PonikoEngine::Finalize();

	return 0;
}
