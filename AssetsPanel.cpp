#include "AssetsPanel.h"
#include "HierarchyPanel.h"

#include "ImGuiManager.h"

#include "FileDropManager.h"

#include "WinApp.h"

#include <algorithm>
#include <memory>

#include "SceneComponentLoader.h"

void AssetsPanel::Initialize()
{
	
	/* 全てのアセットをロード */
	AssetsLoad();


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
		for (int i = 0; i < hierarchyModelResouces_.size(); i++)
		{

			const ModelResource& resource = hierarchyModelResouces_.at(i);

			int alpha = 255;

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
				h.layerName_ = hierarchyPanel_->GetSelectedCurrentLayerName();

				h.object = resource.model_;

				// WorldTransform初期化
				h.transform_ = std::make_unique<WorldTransform>();
				h.transform_->Initialize();

				h.isView_ = true;

				/* ヒエラルキ追加 */
				hierarchyPanel_->AddHierarchyModel(hierarchyPanel_->GetSelectedCurrentLayerName(), h);


				/* シーン側のGameObjectに干渉してたら一緒に増やす */
				hierarchyPanel_->GameObjectAddCheck();
				

				//// 該当レイヤー検索
				//auto it = std::find_if(
				//	hierarchyPanel_->GetHierarchyLayers().begin(),
				//	hierarchyPanel_->GetHierarchyLayers().end(),
				//	[&h, this](const HierarchyLayer& layer) {
				//		return layer.layerName_ == hierarchyPanel_->GetSelectedCurrentLayerName();
				//	}
				//);

				////* もし、シーン側で登録しているレイヤーで生成された場合 *//
				//if (it->isAlignGameScene) {

				//	// もし、シーン側で登録しているレイヤーで生成された場合、シーン側のゲームオブジェクトも生成する
				//	if (hierarchyPanel_->gameObjectAdd_ != nullptr) {


				//		// 種類０：サブ種類０を、仮だけど錬成する
				//		hierarchyPanel_->gameObjectAdd_(0, 0, *hierarchyPanel_->GetHierarchyModels().back().get());

				//	}
				//}


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

		/*
			スプライト
		*/
		ImGui::SeparatorText("Sprites");

		for (int i = 0; i < hierarchyTextureResouces_.size(); i++)
		{

			const TextureResource& resource = hierarchyTextureResouces_.at(i);

			float size = 24.0f;

			/* 画像表示 */
			ImGui::Image(
				(ImTextureID)resource.texture_->GetTextureSrvHandleGPU().ptr,
				ImVec2(size, size)
			);

			ImGui::SameLine();

			// 名前
			ImGui::Text(resource.name_.c_str());

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

void AssetsPanel::AssetsLoad()
{

	// よみこむJSONパス（プロジェクト一個で固定）
	jsonPath = "resources/AssetList_.json";


	if (jsonPath.empty()) return;

	// 読込み
	if (SceneComponentLoader::GetInstance()->Load(jsonPath)) {

		SceneComponentLoader* loder = SceneComponentLoader::GetInstance();

		/* 
			モデル
		*/
		loder->BeginGroup("Model");

			// ヒエラルキのモデルのパスリスト 登録
			loder->Property("ModelResources", hierarchyModelResouces_);

		loder->EndGroup();

		/*
			テキスチャ
		*/

		loder->BeginGroup("Texture");

			loder->Property("TextureResources", hierarchyTextureResouces_);

		loder->EndGroup();


			// 読み込み
		loder->LoadGroup("Model");
		loder->LoadGroup("Texture");

	}

}

void AssetsPanel::AssetsSave()
{
	// セーブ
	if (SceneComponentLoader::GetInstance()->Load(jsonPath)) {

		SceneComponentLoader* loder = SceneComponentLoader::GetInstance();

		/* セーブ前のJson切り替え */
		if (loder->SaveSwitchJson(jsonPath)) {

			loder->SaveGroup("Model");
			loder->SaveGroup("Texture");

			SceneComponentLoader::GetInstance()->Save(jsonPath);
		}
	}
}

void AssetsPanel::Update()
{
	// モデル更新(いる？
	for (int i = 0; i < hierarchyModelResouces_.size(); i++)
	{
		hierarchyModelResouces_.at(i).model_->Update();
	}

	// ウインドウ上にドロップされたモデルを追加
	AddDropModel();

	// ウインドウ上にドロップされたスプライトを追加
	AddDropSprite();
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

				hierarchyModelResouces_.begin(),
				hierarchyModelResouces_.end(),

				//* ラムダ式 *//
				// ↓キャプチャ  ↓forのように取り出し
				[&modelId](const auto& h) {
					return h.pathId_ == modelId;
				}
			);

			//* 見つかった
			if (it != hierarchyModelResouces_.end()) {

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
			for (const ModelResource& r : hierarchyModelResouces_) {

				if (r.name_ == resource.name_) {

					resource.name_ = r.name_ + "_" + std::to_string(count);
					count++;

				}
			}

			// モデルリソース追加
			AddModelResource(resource);

			/* モデルのヒエラルキ追加 */
			Hierarchy<Model> h;

			h.type_ = HierarchyType::kModel; // タイプ
			h.id_ = resource.pathId_;
			h.name_ = resource.name_;
			h.layerName_ = hierarchyPanel_->GetSelectedCurrentLayerName();

			h.object = resource.model_;

			h.isView_ = true;

			// WorldTransform初期化
			h.transform_ = std::make_unique<WorldTransform>();
			h.transform_->Initialize();

			/* 初期座標とかあったらその都度っここに　今は０，０，０  */

			hierarchyPanel_->AddHierarchyModel(hierarchyPanel_->GetSelectedCurrentLayerName(), h);


			/* シーン側のGameObjectに干渉してたら一緒に増やす */
			hierarchyPanel_->GameObjectAddCheck();


		}

	}

}

void AssetsPanel::AddDropSprite()
{
	// 新しくドロップされたパスを取得
	auto paths =
		FileDropManager::TakeFilePaths(FileType::kSprite);

	for (const auto& path : paths) {

		// 絶対パス → resources/ 以下の相対パス
		std::string relativePath =
			FileDropManager::GetRelativePath(path);

		// 相対パスが取得できなければスキップ
		if (relativePath.empty()) {
			continue;
		}


		//　スプライトのIDの取得（spriteのパス）
		std::string spriteId = relativePath;

		{// モデルの管理情報から、すでに登録されたIDのがあるかvector検索

			auto it = std::find_if(

				hierarchyTextureResouces_.begin(),
				hierarchyTextureResouces_.end(),

				//* ラムダ式 *//
				// ↓キャプチャ  ↓forのように取り出し
				[&spriteId](const auto& h) {
					return h.pathId_ == spriteId;
				}
			);

			//* 見つかった
			if (it != hierarchyTextureResouces_.end()) {

				// 登録せずサヨナラ
				continue;
			}
		}


		// texture生成
		Texture* texture = new Texture();
		texture->Initialize(relativePath);

		// 読み込めた
		if (texture->IsLoaded()) {

			/* 管理情報の登録 */
			TextureResource resource;

			resource.pathId_ = relativePath;
			resource.texture_ = texture;		// モデル登録

			resource.name_ = FileDropManager::GetFileNameWithoutExtension(path);// 表示名 拡張子を抜いたのを設定する

			/* すでに同じ名前があったら */
			int count = 2;
			for (const ModelResource& r : hierarchyModelResouces_) {

				if (r.name_ == resource.name_) {

					resource.name_ = r.name_ + "_" + std::to_string(count);
					count++;

				}
			}

			// モデルリソース追加
			AddSpriteResource(resource);
		}
	}
}
