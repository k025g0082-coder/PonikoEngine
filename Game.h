#pragma once

#include "Scene.h"
#include "Input.h"
#include "Fade.h"


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

private:

	static Fade fade;
	Input gamepad;
};
