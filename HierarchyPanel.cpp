#include "HierarchyPanel.h"

#include "Editor.h"

#include "ImGuiManager.h"

#include "SceneComponentLoader.h"

#include "FileDropManager.h"

#include <algorithm>
#include <memory>

void HierarchyPanel::Initialize(const std::string jsonPath, Grid& g)
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

	if (ImGui::Begin("Hierarchy", &show)) {


		// === ヒエラルキー
		ImGui::SeparatorText("Hierarchys");

		// モデル一覧
		if (ImGui::TreeNode("Models")) {

			for (int i = 0; i < hierarchyModels_.size(); i++)
			{
				// 選択中のは色変え
				if (selectedModelIndex_.has_value() && selectedModelIndex_.value() == i) {
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
	}

	ImGui::End();

	// 色の
	ImGui::PopStyleColor();
#endif
}

void HierarchyPanel::Draw()
{
}



WorldTransform* HierarchyPanel::PasteHierarchyModel()
{

	///* モデルのヒエラルキ追加 */
	Hierarchy<Model> h;

	// セレクト中のから各コピー
	h.type_ = hierarchyModels_.at(selectedModelIndex_.value()).type_;
	h.id_ = hierarchyModels_.at(selectedModelIndex_.value()).id_;
	h.name_ = hierarchyModels_.at(selectedModelIndex_.value()).name_;

	h.isView_ = hierarchyModels_.at(selectedModelIndex_.value()).isView_;

	// WorldTransform座標決定
	h.transform_ = std::make_unique<WorldTransform>();

	h.transform_->Initialize();
	h.transform_->translation_ = hierarchyModels_.at(selectedModelIndex_.value()).transform_->translation_;
	h.transform_->rotation_ = hierarchyModels_.at(selectedModelIndex_.value()).transform_->rotation_;
	h.transform_->scale_ = hierarchyModels_.at(selectedModelIndex_.value()).transform_->scale_;

	WorldTransform* world = static_cast<WorldTransform*>(h.transform_.get());

	// 位置更新
	world->UpdateMatrix();

	/* ヒエラルキ-モデル - 追加 */
	AddHierarchyModel(h);


	// セレクト中のインデックスを末尾のに変更
	selectedModelIndex_.value() = (int)hierarchyModels_.size() - 1;


	//*  BaseTransform -> WorldTransform
	WorldTransform* rworld = dynamic_cast<WorldTransform*>(hierarchyModels_.back().transform_.get());

	// ついかしたヒエラルキのワールド
	return rworld;
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

void HierarchyPanel::ChangeHierarchyModelView(bool b, int assetIndex)
{

	hierarchyModelResouces_.at(assetIndex).isView_ = b;

	// 対応したアセットが紐づけられたヒエラルキモデル表示・非表示切り替え
	for (int i = 0; i < hierarchyModels_.size(); i++)
	{
		Hierarchy<Model>& model = hierarchyModels_.at(i);

		if (model.id_ == hierarchyModelResouces_.at(assetIndex).pathId_) {
			model.isView_ = b;
		}
	}
}

void HierarchyPanel::HierarchyClear()
{
	hierarchyModels_.clear();
}



void HierarchyPanel::HierarchyLoad(const std::string jsonPath) {

	if (jsonPath.empty()) return;

	/* 前のヒエラルキ画面は初期化 */
	HierarchyClear();

	// 読込み
	if (SceneComponentLoader::GetInstance()->Load(jsonPath)) {

		SceneComponentLoader* loder = SceneComponentLoader::GetInstance();

		// ライト配置のロード
		SceneComponentLoader::GetInstance()->LightLoad();

		/* ヒエラルキー */
		loder->BeginGroup("Grid");

		loder->Property("gridWidth", grid->gridWidth_);
		loder->Property("gridHeight", grid->gridHeight_);
		loder->Property("gridDepth", grid->gridDepth_);

		loder->Property("vertical", grid->vertical_);
		loder->Property("horizontal", grid->horizontal_);

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
	grid->Update();


	// 読み込んだモデルリソースのモデルを全て生成
	for (int i = 0; i < hierarchyModelResouces_.size(); i++)
	{
		
		ModelResource& r = hierarchyModelResouces_.at(i);

		r.model_ = Model::CreateOBJ(r.pathId_);
		
		/* ライティングはデフォルトでON */
		r.model_->SetLightSets();

		// モデルのポインタ登録
		hierarchyModelResourceList_[r.pathId_] = r.model_;
		r.name_ = FileDropManager::GetFileNameWithoutExtension(FileDropManager::ToWide(r.pathId_));// 表示名 拡張子を抜いたのを設定する

		/* 読み込んだ表示非表示の切り替え */
		ChangeHierarchyModelView(r.isView_,i);
	}

}

void HierarchyPanel::HierarchySave(const std::string jsonPath) {

	if (jsonPath.empty()) return;

	// ※注　ポインタ
	auto loder = SceneComponentLoader::GetInstance();

	if (loder->SaveSwitchJson(jsonPath)) {

		// ライト配置のセーブ
		SceneComponentLoader::GetInstance()->LightSave();

		loder->SaveGroup("Grid");
		loder->SaveGroup("Hierarchy");

		SceneComponentLoader::GetInstance()->Save(jsonPath);
	}
}
