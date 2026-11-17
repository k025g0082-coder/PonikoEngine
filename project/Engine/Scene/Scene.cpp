#include "Scene.h"
#include "Game.h"

#include "ImGuiManager.h"
#include "LightingManager.h"

#include "FileDropManager.h"

#include "EffectManager.h"
#include "PostEffectManager.h"

#include "Editor.h"


Scene* Scene::instance = nullptr;
std::string Scene::nextSceneName;
std::string Scene::nextSceneHierarchyJsonPath;

bool Scene::isChanging = false;

Camera* Scene::camera_ = nullptr;

void Scene::Update(Input* gamepad){
	gamepad;
}

void Scene::Draw(){

}

void Scene::StartGameScene(std::string sceneName, std::string sceneHierarchyJsonPath)
{
	/*
	　Editor側の切り替え処理
	*/
	Editor::GetInstance()->SwitchActiveScene(sceneName, sceneHierarchyJsonPath,true);//			最初からカスタムのJsonファイルをmain.cppから読み込むなんで無いっしょｗｗｗｗ

	// 各パネルの初期化
	Editor::GetInstance()->PanelInit(sceneName, sceneHierarchyJsonPath);


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

		// PostエフェクトのBloom対象などの全削除
		PostEffectManager::GetInstance()->Reset();

		/*
			Editorモードの場合、Editorから提供されたＳｃｅｎｅを設定する
		*/
		if (Editor::IsEditorMode()) {

			/*  　Editor側の切り替え処理 　 */
			Editor::GetInstance()->SwitchActiveScene(nextSceneName,nextSceneHierarchyJsonPath);
			
			/* 　Editorからインスタンスをもらう　 */
			instance = Editor::GetInstance()->GetActiveSceneInstance();

			// 初期化
			instance->Init();

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