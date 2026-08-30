#pragma once

#include "Scene.h"
#include "GamePad.h"
#include "Fade.h"

#include "Event.h"

class Game {
public:
	
	// シングルトン
	static Game* GetInstance();

	// 最初に実行するSceneのとうろく
	void StartGameScene(Scene* scene) {
		Scene::RequestChange(scene);
	}

	void Init();
	void Update();
	void Draw();
	void Finalize();

	static int kWindowWidth;
	static int kWindowHeight;

	static Fade& GetFade() { return fade; }

	// イベント追加
	static void EventToRegion(Event* e) {
		events_.push_back(e);
		events_.back()->Initialize();
	}

private:

	// イベントリスト
	static std::list<Event*> events_;

	static Fade fade;
	GamePad gamepad;
};
