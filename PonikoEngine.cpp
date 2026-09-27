#include "PonikoEngine.h"

#include <memory>
#include <filesystem>

#include "EffekseerManager.h"


// ここにメンバを書く
//namespace {
//	class EngineImpl { // 内部実装、の略
//	public:
//		Triangle triangle;
//		Sprite sprite;
//		Model* model;
//		Model* model2;
//		Text text;
//		Camera camera;
//		DirectionalLight light;
//	};
//	static std::unique_ptr<EngineImpl> impl; //　一つのポインタ、そしてメモリを自動で開放してくれるらしい
//}

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

		// もじシステム初期化
		Text::Initialize();

		// 音初期化
		SoundManager::GetInstance()->Initialize();

		// Imgui初期化
		ImGuiManager::GetInstance()->Initialize();

		// いんぷと
		GamePad::GetInstance()->Initialize();

		// ３種のライトセットを司るクラスの初期化
		LightingManager::GetInstance()->Initialize();

		// Gameシステムの方の初期化
		Game::GetInstance()->Init();

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

			GamePad::GetInstance()->Update();

			// エフェクト更新
			EffectManager::Update();

			// パーティクル更新
			Particle::GetInstance()->Update();


		ImGuiManager::GetInstance()->End();

	};

	void Draw() {

		// 描画
		DirectXCommon::GetInstance()->PreDraw();

			Game::GetInstance()->Draw();

			// パーティクル
			Particle::GetInstance()->Draw();

		// IMGUI byouga
		ImGuiManager::GetInstance()->Draw();

		// 描画終了
		DirectXCommon::GetInstance()->PostDraw();
		

	};

	void Finalize() {

		WinApp::GetInstance()->Finalize();

		ImGuiManager::GetInstance()->Finalize();


		CoUninitialize();

	};
};