#include "Scene.h"
#include "Game.h"

#include "ImGuiManager.h"
#include "LightingManager.h"

#include "FileDropManager.h"

#include "Particle.h"
#include "EffectManager.h"

#include "Editor.h"


Scene* Scene::instance = nullptr;
std::string Scene::nextSceneName;

bool Scene::isChanging = false;

Camera* Scene::camera_ = nullptr;

void Scene::Update(Input* gamepad){
	gamepad;
}

void Scene::Draw(){

}

void Scene::StartGameScene(std::string sceneName)
{
	/*  　Editor側の切り替え処理 　 */
	Editor::GetInstance()->SwitchActiveScene(sceneName,true);

	// 各パネルの初期化
	Editor::GetInstance()->PanelInit(sceneName);


	/* 　Editorからインスタンスをもらう　 */
	instance = Editor::GetInstance()->GetActiveSceneInstance();

	/*
		ここで初めてScene初期化
	*/
	instance->Init();
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

		/*
			Editorモードの場合、Editorから提供されたＳｃｅｎｅを設定する
		*/
		if (Editor::IsEditorMode()) {

			/*  　Editor側の切り替え処理 　 */
			Editor::GetInstance()->SwitchActiveScene(nextSceneName);
	
			/* 　Editorからインスタンスをもらう　 */
			instance = Editor::GetInstance()->GetActiveSceneInstance();

		}
		else
		{
			// 通常　releaseモードの場合

			/*
				現在のインスタンスを破棄
			*/
			delete instance;

			auto scene = SceneFactory::Create(nextSceneName);

			/* 読み込めた */
			if (scene) {

				instance = scene.release();// 移動
				instance->Init();

				
			}
		}

		// 読み込みの有無にかかわらずfade終わったか切り替え
		isChanging = false;

		if (instance) {
			// 現在開いているシーン名を更新
			Game::SetCurrentSceneName(instance->sceneName_);
		}
	}
}