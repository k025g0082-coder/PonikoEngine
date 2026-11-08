#pragma once

#include "Input.h"
#include "Camera.h"

#include "SceneFactory.h"

#include <string>

class Scene {

public:

	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="sceneName">シーン名( 必須 )</param>
	explicit Scene(const std::string& sceneName)
		: sceneName_(sceneName)
	{}

	virtual ~Scene() = default;


	/// <summary>
	/// 各Sceneクラス内の初期化
	/// </summary>
	virtual void Init() = 0;

	/// <summary>
	/// 各Sceneクラス内の更新
	/// </summary>
	virtual void Update(Input* gamepad) = 0;

	/// <summary>
	/// 各Sceneクラス内の描画
	/// </summary>
	virtual void Draw() = 0;


	// ゲーム開始時
	static void StartGameScene(std::string sceneName);


	/// <summary>
	/// 現在のSceneのポインタを置き換える
	/// </summary>
	/// <param name="activeScene"></param>
	static void SetActiveInstanceReference(Scene* activeScene) {

		// 旧 instance の delete は行わない。
		// ポインタを置き換えるだけ。SceneのInitializeはEditor側で行う
		instance = activeScene;

	}

	

	/// <summary>
	/// 別のシーンに切り替え
	/// </summary>
	/// <param name="sceneName">シーン名</param>
	static void RequestChange(std::string sceneName) {
		nextSceneName = sceneName;
		isChanging = true;
	}

	static void ProcessChange();

	static Scene* getInstance() { return instance; }

	static void setInstance(Scene* d) {
		instance = d;
		instance->Init();
	}

	/* ゲーム内で使うメインカメラを登録する。 */
	static void SetSceneCamera(Camera* camera) {

		camera_ = camera;
	}
	static Camera* GetSceneCamera() { return camera_; }

protected:

	/// <summary>
	/// シーン名
	/// </summary>
	std::string sceneName_;


private:

	static std::string nextSceneName;

	static Scene* instance;

	static bool isChanging;

	static Camera* camera_;
};
