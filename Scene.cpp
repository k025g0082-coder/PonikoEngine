#include "Scene.h"
#include "Game.h"

Scene* Scene::instance = nullptr;
Scene* Scene::nextInstance = nullptr;
bool Scene::isChanging = false;

void Scene::ProcessChange() {

	// フェードアニメ終了後なら
	if (isChanging &&
		Game::GetFade().IsFinished()
		) {

		delete instance;

		instance = nextInstance;
		isChanging = false;

		instance->Init();
	}
}