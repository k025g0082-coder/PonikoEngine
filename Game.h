#pragma once

#include "Scene.h"
#include "Input.h"
#include "Fade.h"

class Game {
public:
	
	// シングルトン
	static Game* GetInstance();

	// 最初に実行するSceneのとうろく
	void StartGameScene(std::string sceneName);

	void Init();
	void Update();
	void Draw();
	void Finalize();


	// 画面のモード
	enum class WindowViewMode {

		kEditorMode,			// エディタ用
		kFullScreenMode,			// Release等フルスクリーン用

		count
	};
	
	// 画面モード変更
	static void SetWindowViewMode(WindowViewMode mode) {

		currentViewMode_ = mode;

		// ゲーム画面の大きさを設定されたのに変更
		ResizeGameScreen(
			windowViewParams_[(int)mode].width,
			windowViewParams_[(int)mode].height,
			windowViewParams_[(int)mode].offset
		);
	}

	/// <summary>
	/// ゲーム画面の大きさ変更
	/// </summary>
	/// <param name="width">横</param>
	/// <param name="height">縦</param>
	/// <param name="offset">オフセット</param>
	static void ResizeGameScreen(int32_t width, int32_t height, Vector2 offset = { 0.0f, 0.0f });

	static void SetGameScreenOffset(Vector2 v);

	static uint32_t GetGameScreenWidth() { return kGameScreenWidth; }
	static uint32_t GetGameScreenHeight() { return kGameScreenHeight; }

	static Vector2  GetGameScreenOffset() { return kGameScreenOffset; }


	static Fade& GetFade() { return fade; }

	/// <summary>
	/// 現在開いているシーン名
	/// </summary>
	static const std::string& GetCurrentSceneName() { return currentSceneName; }


	/// <summary>
	/// 現在開いているシーン名の更新
	/// </summary>
	/// <param name="sceneName"></param>
	static void SetCurrentSceneName(std::string sceneName) { currentSceneName = sceneName; }

private:

	/// <summary>
	/// 現在開いているシーン名
	/// </summary>
	static std::string currentSceneName;


	// 画面もーど
	static WindowViewMode currentViewMode_;


	struct WindowViewParam {

		int32_t width;
		int32_t height;

		Vector2 offset;

	};

	// 設定まとめ
	static WindowViewParam windowViewParams_[(int)WindowViewMode::count];


	// ゲーム画面の大きさ
	static uint32_t kGameScreenWidth;
	static uint32_t kGameScreenHeight;

	static Vector2  kGameScreenOffset;


	static Fade fade;
	Input gamepad;
};
