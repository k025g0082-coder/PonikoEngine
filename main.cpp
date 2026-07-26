#include "PonikoEngine.h"

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int)
{
	PonikoEngine::Initialize(L"ぽにこエンジン");


	while (!WinApp::GetInstance()->ProcessMessage())
	{
		// 更新
		PonikoEngine::Update();

		// 描画
		PonikoEngine::Draw();
	}

	PonikoEngine::Finalize();

	return 0;
}