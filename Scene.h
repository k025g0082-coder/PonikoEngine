#pragma once

#include "Input.h"
#include "Camera.h"

class Scene {

public:
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

	virtual ~Scene() = default;

	/// <summary>
	/// 別Sceneに切り替え
	/// </summary>
	static void RequestChange(Scene* d) {
		nextInstance = d;
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

private:
	static Scene* instance;
	static Scene* nextInstance;
	static bool isChanging;

	static Camera* camera_;
};
