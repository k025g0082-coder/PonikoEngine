#include "PonikoEngine.h"

#include <memory>
#include <filesystem>

#include "EffekseerManager.h"

namespace PonikoEngine {

	void Initialize(const wchar_t* title) {

		// COM初期化
		CoInitializeEx(0, COINIT_MULTITHREADED);

		// WinApp初期化。ウインドウ生成
		WinApp::GetInstance()->Initialize(title, 1280, 720);

		// DirectXCommon初期化
		DirectXCommon::GetInstance()->Initialize();

		// 各種、パイプライン設定と登録
		PipelineStateManager::GetInstance()->Initialize(DirectXCommon::GetInstance()->GetDevice().Get());

		// Effekseerの初期化
		EffekseerManager::GetInstance()->Initialize(
			DirectXCommon::GetInstance()->GetDevice().Get(),
			DirectXCommon::GetInstance()->GetCommandQueue().Get()
		);

		// ポストエフェクトの初期化
		PostEffectManager::GetInstance()->Initialize();

		// もじシステム初期化
		Text::Initialize();

		// 音初期化
		SoundManager::GetInstance()->Initialize();

		// Imgui初期化
		ImGuiManager::GetInstance()->Initialize();

		// いんぷと
		Input::GetInstance()->Initialize();

		// ３種のライトセットを司るクラスの初期化
		LightingManager::GetInstance()->Initialize();

		// Gameシステムの方の初期化
		Game::GetInstance()->Init();

		// デバッグ用ライン
		DebugLine::GetInstance()->Initialize();

		// タイマー
		Timer::GetInstance()->Initialize();

	};

	void Update() {

		// 時間更新
		Timer::GetInstance()->UpdateDeltaTime();

		// 音楽更新
		SoundManager::GetInstance()->Update();


		// 更新
		ImGuiManager::GetInstance()->Begin();

			/*　ここに更新処理　*/
			Game::GetInstance()->Update();

			Input::GetInstance()->Update();

			// エフェクト更新
			EffectManager::Update();

			// パーティクル更新
			Particle::GetInstance()->Update();

			// こおしｎ(前回の線全削除
			DebugLine::GetInstance()->Render();

		ImGuiManager::GetInstance()->End();

	};

	void Draw() {

		// 描画
		DirectXCommon::GetInstance()->PreDraw();


			PostEffectManager::GetInstance()->BeginScene();

				Game::GetInstance()->Draw();

				// パーティクル
				Particle::GetInstance()->Draw();

				// デバッグ用
				DebugLine::GetInstance()->Draw();

			PostEffectManager::GetInstance()->ApplyScene();


			// IMGUI byouga
			ImGuiManager::GetInstance()->Draw();

		// 描画終了
		DirectXCommon::GetInstance()->PostDraw();
		

	};

	void Finalize() {

		WinApp::GetInstance()->Finalize();

		ImGuiManager::GetInstance()->Finalize();
		Input::GetInstance()->Finalize();
		SoundManager::GetInstance()->Finalize();

		CoUninitialize();

	};
};