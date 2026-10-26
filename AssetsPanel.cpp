#include "AssetsPanel.h"

#include "ImGuiManager.h"

#include "FileDropManager.h"

#include "WinApp.h"

#include <algorithm>
#include <memory>


void AssetsPanel::Initialize()
{
	// エフェクト専用部屋の初期化
	selectedModelViewport_.Initialize(320, 180);
}

void AssetsPanel::PanelView(bool& show)
{

	showView = &show;

	if (!selectedHierarchyResource_.pathId_.empty()) {
		/* エフェクトビューポートの更新 */
		selectedModelViewport_.Update();
	}

#ifdef USE_IMGUI

	/* 色 */
	ImGui::PushStyleColor(
		ImGuiCol_WindowBg,
		ImVec4(0.10f, 0.14f, 0.20f, 1.0f)
	);

	if (ImGui::Begin("Assets-Manager", &show)) {


		ImGui::SeparatorText("Models");

		// === モデル
		for (int i = 0; i < hierarchyPanel_->GetHierarchyModelResources().size(); i++)
		{

			const ModelResource& resource = hierarchyPanel_->GetHierarchyModelResources().at(i);

			int alpha = 255;

			//// ひひょうじは薄く
			//if (!selectedHierarchyResource_.isView_) {
			//	alpha = 100;
			//}

			/* 選択中は色変え */
			if (selectedHierarchyResource_.pathId_ == resource.pathId_) 
				ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 100, 50, alpha));

			if (ImGui::TreeNode(resource.name_.c_str())) {

				// ここに何か書く

				
				ImGui::TreePop();
			}


			if (selectedHierarchyResource_.pathId_ == resource.pathId_)
				ImGui::PopStyleColor();

			ImGui::SameLine(ImGui::GetWindowWidth() - 190.0f);

			/* 見えるか */
			ImGui::PushID(i);

			if (!resource.isView_) {
				if (ImGui::Button("ON")) {

					// 紐づけられたモデルの表示非表示切り替え
					hierarchyPanel_->ChangeHierarchyModelView(true, i);

					ImGui::PopID();
					break;
				}
			}
			else {
				if (ImGui::Button("OFF")) {

					// 紐づけられたモデルの表示非表示切り替え
					hierarchyPanel_->ChangeHierarchyModelView(false, i);

					ImGui::PopID();
					break;
				}
			}
			ImGui::PopID();

			ImGui::SameLine(ImGui::GetWindowWidth() - 135.0f);


			/* モデルのリソース選択 */
			ImGui::PushID(i);
			if (ImGui::Button("Select")) {

				// 
				selectedHierarchyResource_ = resource;

				// ビューポートのモデル選びなおし
				selectedModelViewport_.ClearModel();
				selectedModelViewport_.AddModel(resource.model_);


				ImGui::PopID();
				break;
			}
			ImGui::PopID();

			ImGui::SameLine();

			ImGui::PushID(i);

			// 選択されたヒエラルキモデルで追加ボタン
			if (ImGui::Button("Add")) {

				Hierarchy<Model> h;

				h.type_ = HierarchyType::kModel; // タイプ
				h.id_ = resource.pathId_;
				h.name_ = resource.name_;// 表示名 拡張子を抜いたのを設定する

				// WorldTransform初期化
				h.transform_ = std::make_unique<WorldTransform>();
				h.transform_->Initialize();

				h.isView_ = true;

				/* ヒエラルキ追加 */
				hierarchyPanel_->AddHierarchyModel(h);

				ImGui::PopID();
				break;
			}

			ImGui::PopID();

			ImGui::SameLine();

			/* モデルのリソース削除 */
			ImGui::PushID(i);

			if (ImGui::Button("Del")) {

				hierarchyPanel_->DeleteHierarchyResouce(i);

				ImGui::PopID();
				break;
			}
			ImGui::PopID();
		}


		/* 選択中のモデルのビューポート表示 */
		ImGui::Image(
			(ImTextureID)selectedModelViewport_.GetRenderTarget()->GetSRVHandleGPU().ptr,
			ImVec2(320.0f, 180.0f)
		);

		selectedModelViewport_.ImGuiView();
	}

	ImGui::End();

	ImGui::PopStyleColor();
#endif

}

void AssetsPanel::Update()
{
	// ウインドウ上にドロップされたモデルを追加
	AddDropModel();
}

void AssetsPanel::Draw()
{
	if (!showView) return;
	if (selectedHierarchyResource_.pathId_.empty()) return;

	/* エフェクトビューポートの描画！！！！ */
	selectedModelViewport_.Draw();
	
}


void AssetsPanel::AddDropModel()
{

	// 新しくドロップされたパスを取得
	auto paths =
		FileDropManager::TakeFilePaths(FileType::kModel);

	for (const auto& path : paths) {

		// 絶対パス → resources/ 以下の相対パス
		std::string relativePath =
			FileDropManager::GetRelativePath(path);

		// 相対パスが取得できなければスキップ
		if (relativePath.empty()) {
			continue;
		}


		//　モデルのIDの取得（モデルのパス）
		std::string modelId = relativePath;

		{// モデルの管理情報から、すでに登録されたIDのがあるかvector検索

			auto it = std::find_if(

				hierarchyPanel_->GetHierarchyModelResources().begin(),
				hierarchyPanel_->GetHierarchyModelResources().end(),

				//* ラムダ式 *//
				// ↓キャプチャ  ↓forのように取り出し
				[&modelId](const auto& h) {
					return h.pathId_ == modelId;
				}
			);

			//* 見つかった
			if (it != hierarchyPanel_->GetHierarchyModelResources().end()) {

				// 登録せずサヨナラ
				continue;
			}
		}


		// モデル生成
		Model* model = Model::CreateOBJ(relativePath);


		// 読み込めた
		if (model) {

			/* モデルの管理情報の登録 */
			ModelResource resource;

			resource.pathId_ = relativePath;
			resource.model_ = model;		// モデル登録

			//*  デフォルトでライトセット  *//
			resource.model_->SetLightSets();

			resource.name_ = FileDropManager::GetFileNameWithoutExtension(path);// 表示名 拡張子を抜いたのを設定する

			resource.isView_ = true;
			
			/* すでに同じ名前があったら */
			int count = 2;
			for (const ModelResource& r : hierarchyPanel_->GetHierarchyModelResources()) {

				if (r.name_ == resource.name_) {

					resource.name_ = r.name_ + "_" + std::to_string(count);
					count++;

				}
			}

			// モデルリソース追加
			hierarchyPanel_->AddModelResource(resource);

			/* モデルのヒエラルキ追加 */
			Hierarchy<Model> h;

			h.type_ = HierarchyType::kModel; // タイプ
			h.id_ = resource.pathId_;
			h.name_ = resource.name_;

			h.isView_ = true;

			// WorldTransform初期化
			h.transform_ = std::make_unique<WorldTransform>();
			h.transform_->Initialize();

			/* 初期座標とかあったらその都度っここに　今は０，０，０  */

			hierarchyPanel_->AddHierarchyModel(h);


		}

	}

}