#include "HierarchyPanel.h"

#include "Editor.h"

#include "ImGuiManager.h"

#include "SceneComponentLoader.h"

#include "FileDropManager.h"

#include <algorithm>
#include <memory>

void HierarchyPanel::Initialize(const std::string jsonPath, ImGuizmoGrid& g)
{	
	grid = &g;

	/* 初期化で読込んどく */
	HierarchyLoad(jsonPath);
}

void HierarchyPanel::Update()
{
	// モデル更新
	for (int i = 0; i < hierarchyModelResouces_.size(); i++)
	{
		hierarchyModelResouces_.at(i).model_->Update();
	}

	// ヒエラルキのWorldTransform更新
	for (int i = 0; i < hierarchyModels_.size(); i++)
	{
		// キャ・・・キャストして更新・・ｗ
		if (auto* world = dynamic_cast<WorldTransform*>(
			hierarchyModels_.at(i).transform_.get())) {
			world->UpdateMatrix();
		}

	}
}

void HierarchyPanel::ViewPanel(bool& show)
{

#ifdef USE_IMGUI

	/* 色 */
	ImGui::PushStyleColor(
		ImGuiCol_WindowBg,
		ImVec4(0.10f, 0.14f, 0.20f, 1.0f)
	);

	if (ImGui::Begin("Hierarchy",&show)) {

		
		// === ヒエラルキー
		ImGui::SeparatorText("Hierarchys");

		// モデル一覧
		if (ImGui::TreeNode("Models")) {

			for (int i = 0; i < hierarchyModels_.size(); i++)
			{
				// 選択中のは色変え
				if (selectedModelIndex_.has_value()) {
					ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 100, 50, 255));
					    ImGui::BulletText(hierarchyModels_.at(i).name_.c_str());
					ImGui::PopStyleColor();
				}
				else {
					ImGui::BulletText(hierarchyModels_.at(i).name_.c_str());
				}


				ImGui::SameLine(ImGui::GetWindowWidth() - 100.0f);

				ImGui::PushID(i);

				// 削除ボタンを右寄りに配置（↑のおかげ
				if (ImGui::Button("delete")) {

					hierarchyModels_.erase(hierarchyModels_.begin() + i);
					ImGui::PopID();
					break;
				}
				ImGui::PopID();


			}

			ImGui::TreePop();
		}


	ImGui::End();

	// 色の
	ImGui::PopStyleColor();
#endif
}


}
void HierarchyPanel::Draw()
{
}



void HierarchyPanel::DeleteHierarchyResouce(int index)
{
	// リソースに紐づけられたIDのモデルを全て削除
	for (int i = 0; i < hierarchyModels_.size(); i++)
	{

		Hierarchy<Model>& model = hierarchyModels_.at(i);

		if (model.id_ == hierarchyModelResouces_.at(index).pathId_) {
			hierarchyModels_.erase(hierarchyModels_.begin() + i);
			i--;
		}
	}

	// 先にモデルと紐づけたマップを削除
	hierarchyModelResourceList_.erase(hierarchyModelResouces_.at(index).pathId_);

	// モデルのリソース情報削除
	hierarchyModelResouces_.erase(hierarchyModelResouces_.begin() + index);

}



void HierarchyPanel::HierarchyLoad(const std::string jsonPath) {

	// 読込み
	if (SceneComponentLoader::GetInstance()->Load(jsonPath)) {

		SceneComponentLoader* loder = SceneComponentLoader::GetInstance();

		// ライト配置のロード
		SceneComponentLoader::GetInstance()->LightLoad();

		/* ヒエラルキー */
		loder->BeginGroup("Grid");

		loder->Property("translation", grid->translation_);
		loder->Property("rotation", grid->rotation_);
		loder->Property("isSnap", grid->isSnap_);

		loder->EndGroup();
		loder->LoadGroup("Grid");


		/* ヒエラルキー */
		loder->BeginGroup("Hierarchy");


		// ヒエラルキのモデルのパスリスト 登録
		loder->Property("ModelResources", hierarchyModelResouces_);

		// ヒエラルキのモデル情報リスト 登録
		loder->Property("ModelProperties", hierarchyModels_);


		loder->EndGroup();
		loder->LoadGroup("Hierarchy");
	}


	// 読み込んだモデルリソースのモデルを全て生成
	for (ModelResource& r : hierarchyModelResouces_) {

		r.model_ = Model::CreateOBJ(r.pathId_);

		// モデルのポインタ登録
		hierarchyModelResourceList_[r.pathId_] = r.model_;
		r.name_ = FileDropManager::GetFileNameWithoutExtension(FileDropManager::ToWide(r.pathId_));// 表示名 拡張子を抜いたのを設定する
	}


}

void HierarchyPanel::HierarchySave(const std::string jsonPath) {

	// ※注　ポインタ
	SceneComponentLoader* loder = SceneComponentLoader::GetInstance();

	json& data = loder->GetJson();

	// 必ず
	SceneComponentLoader::GetInstance()->ClearJson();

	// ライト配置のセーブ
	SceneComponentLoader::GetInstance()->LightSave();

	loder->SaveGroup("Grid");
	loder->SaveGroup("Hierarchy");

	SceneComponentLoader::GetInstance()->Save(jsonPath);
}
