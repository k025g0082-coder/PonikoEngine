#include "Scene.h"
#include "Game.h"

#include "ImGuiManager.h"
#include "LightingManager.h"

#include "FileDropManager.h"

#include "Particle.h"
#include "EffectManager.h"

Scene* Scene::instance = nullptr;
Scene* Scene::nextInstance = nullptr;
bool Scene::isChanging = false;

Camera* Scene::camera_ = nullptr;

void Scene::Update(Input* gamepad){
	gamepad;
}

void Scene::Draw(){

}

void Scene::ProcessChange() {

	// フェードアニメ終了後なら
	if (isChanging &&
		Game::GetFade().IsFinished()
		) {

		// エフェクト全削除
		EffectManager::ResetEffect();

		// nullを代入
		Particle::GetInstance()->SetCamera(nullptr);

		delete instance;

		instance = nextInstance;
		isChanging = false;

		instance->Init();
	}
}