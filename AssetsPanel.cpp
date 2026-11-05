#include "AssetsPanel.h"
#include "HierarchyPanel.h"

#include "ImGuiManager.h"

#include "FileDropManager.h"

#include "WinApp.h"

#include <imgui/IconsFontAwesome.h>

#include <algorithm>
#include <memory>

#include "SceneComponentLoader.h"


void AssetsPanel::Initialize()
{
	
	/* 全てのアセットをロード */
	AssetsLoad();

	/* JSONをもとにモデル生成 */
	for (int i = 0; i < hierarchyModelResouces_.size(); i++)
	{
		ModelResource& r = hierarchyModelResouces_.at(i);

		r.model_ = Model::CreateOBJ(r.pathId_);

		/* ライティングはデフォルトでON */
		r.model_->SetLightSets();

		// モデルのポインタ登録
		hierarchyModelResourceList_[r.pathId_] = r.model_;
		r.name_ = FileDropManager::GetFileNameWithoutExtension(FileDropManager::ToWide(r.pathId_));// 表示名 拡張子を抜いたのを設定する
	
	}

	/* JSONをもとにテクスチャー生成 */
	for (auto& resource : hierarchyTextureResouces_) {

		Texture* texture = new Texture();
		texture->Initialize(resource.pathId_);

		// 読み込めたら
		if (texture->IsLoaded()) {

			resource.texture_ = texture;
			resource.name_ = FileDropManager::GetFileNameWithoutExtension(FileDropManager::ToWide(resource.pathId_));// 表示名 拡張子を抜いたのを設定す

			hierarchyTextureResourceList_[resource.pathId_] = texture;
		}
	}

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
		
		ViewModelTreeNode();

		/*
			テキスチャー
		*/
		ImGui::SeparatorText("Textures");

		/* テキスチャーの一覧表示 */
		ViewTextureTreeNode();



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
			AddTextureResource(resource);
		}
	}
}

//
//void AssetsPanel::ViewModelTreeNode()
//{
//	for (int i = 0; i < hierarchyModelResouces_.size(); i++)
//	{
//
//		const ModelResource& resource = hierarchyModelResouces_.at(i);
//
//		int alpha = 255;
//
//		/* 選択中は色変え */
//		if (selectedHierarchyResource_.pathId_ == resource.pathId_)
//			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 100, 50, alpha));
//
//		//
//		ImGui::Text(resource.name_.c_str());
//
//
//		//if (ImGui::TreeNode(resource.name_.c_str()),) {
//
//		//	// ここに何か書く
//
//		//	
//		//	ImGui::TreePop();
//		//}
//
//
//		if (selectedHierarchyResource_.pathId_ == resource.pathId_)
//			ImGui::PopStyleColor();
//
//		//ImGui::SameLine(ImGui::GetWindowWidth() - 100.0f);
//
//		/* 見えるか 20261105 これもれいやーできたんで没 */
//		ImGui::PushID(i);
//		/*
//		if (!resource.isView_) {
//			if (ImGui::Button("ON")) {
//
//				// 紐づけられたモデルの表示非表示切り替え
//				hierarchyPanel_->ChangeHierarchyModelView(true, i);
//
//				ImGui::PopID();
//				break;
//			}
//		}
//		else {
//			if (ImGui::Button("OFF")) {
//
//				// 紐づけられたモデルの表示非表示切り替え
//				hierarchyPanel_->ChangeHierarchyModelView(false, i);
//
//				ImGui::PopID();
//				break;
//			}
//		}
//		*/
//		ImGui::PopID();
//
//		//ImGui::SameLine(ImGui::GetWindowWidth() - 75.0f);
//
//		ImGui::PushID(i);
//
//		/* モデルのリソース選択 */
//		/* 20261105　べつにレイヤー機能できたので没 */
//
//		/*
//		if (ImGui::Button("Select")) {
//
//			//
//			selectedHierarchyResource_ = resource;
//
//			// ビューポートのモデル選びなおし
//			selectedModelViewport_.ClearModel();
//			selectedModelViewport_.AddModel(resource.model_);
//
//
//			ImGui::PopID();
//			break;
//		}
//		*/
//
//		ImGui::PopID();
//
//		ImGui::SameLine(ImGui::GetWindowWidth() - 110.0f);
//
//
//		// ボタンのスタイル設定
//		PushButtonStyle();
//
//		ImGui::PushID(i);
//		// 選択されたヒエラルキモデルで追加ボタン
//		if (ImGui::Button((char*)ICON_FA_SQUARE_PLUS)) {
//
//			//
//			AddHierarchyModel(resource);
//
//
//			//// 該当レイヤー検索 
//			//auto it = std::find_if(
//			//	hierarchyPanel_->GetHierarchyLayers().begin(),
//			//	hierarchyPanel_->GetHierarchyLayers().end(),
//			//	[&h, this](const HierarchyLayer& layer) {
//			//		return layer.layerName_ == hierarchyPanel_->GetSelectedCurrentLayerName();
//			//	}
//			//);
//
//			////* もし、シーン側で登録しているレイヤーで生成された場合 *//
//			//if (it->isAlignGameScene) {
//
//			//	// もし、シーン側で登録しているレイヤーで生成された場合、シーン側のゲームオブジェクトも生成する
//			//	if (hierarchyPanel_->gameObjectAdd_ != nullptr) {
//
//
//			//		// 種類０：サブ種類０を、仮だけど錬成する
//			//		hierarchyPanel_->gameObjectAdd_(0, 0, *hierarchyPanel_->GetHierarchyModels().back().get());
//
//			//	}
//			//}
//
//			// ボタンのスタイルおわり
//			PopButtonStyle();
//
//
//			ImGui::PopID();
//			break;
//		}
//		ImGui::PopID();
//
//		ImGui::SameLine();
//
//		ImGui::PushID(i);
//
//		/*
//			モデルのリソース削除
//		*/
//		if (ImGui::Button((char*)ICON_FA_XMARK)) {
//
//			// 削除
//			hierarchyPanel_->DeleteHierarchyResouce(i);
//
//			// ボタンのスタイルおわり
//			PopButtonStyle();
//
//			ImGui::PopID();
//			break;
//		}
//
//		// ボタンのスタイルおわり
//		PopButtonStyle();
//
//
//		ImGui::PopID();
//	}
//}

void AssetsPanel::ViewModelTreeNode()
{
	for (int folderIndex = 0; folderIndex < modelFolderNames_.size(); folderIndex++)
	{

		const auto& folder = modelFolderNames_.at(folderIndex);


		// Models用の文字列ID（または独自のポインタ）でID空間を区別する
		ImGui::PushID(("Model_" + folder).c_str());


		// フォルダ表示
		bool isOpen = ImGui::TreeNodeEx(
			folder.c_str(),
			ImGuiTreeNodeFlags_SpanAvailWidth
		);

		/*   -ドラッグ終了地点   */
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload =
				ImGui::AcceptDragDropPayload("MODEL"))
			{
				// 受け取ったモデルの配列番号
				int index = *(const int*)payload->Data;

				// モデルの所属フォルダを変更
				hierarchyModelResouces_[index].folderName_ = folder;
			}

			ImGui::EndDragDropTarget();
		}

		// フォルダの中身 - 表示,一般の処理
		if (isOpen)
		{
			for (int i = 0; i < hierarchyModelResouces_.size(); i++)
			{
				//
				const auto& resource = hierarchyModelResouces_.at(i);

				if (resource.folderName_ == folder)
				{
					//ImGui::Text("%s", resource.name_.c_str());

					float size = 24.0f;

					ImGui::PushID(i);

					/*
						葉っぱ（Leaf
						の処理（意味深）（隠語）
					*/

					ViewModelLeafTreeNode(i, resource);

					//bool isSelected = (selectedHierarchyResource_.pathId_ == resource.pathId_);
					//if (ImGui::Selectable(resource.name_.c_str(), isSelected))
					//{
					//	// クリックされた時の処理
					//	//selectedHierarchyResource_ = resource;
					//	//selectedModelViewport_.ClearModel();
					//	//selectedModelViewport_.AddModel(resource.model_);
					//}

					/*   -ドラッグ開始地点-    */
					if (ImGui::BeginDragDropSource())
					{
						//  ドラッグ開始した配列のindexを渡す  //
						ImGui::SetDragDropPayload(
							"MODEL",
							&i,
							sizeof(int)
						);

						// 名前
						ImGui::Text(resource.name_.c_str());

						ImGui::EndDragDropSource();
					}

					ImGui::PopID();
				}
			}

			ImGui::TreePop();
		}

		// 選択時の場合すこし下のレイヤーとに余白入れる
		if (isOpen)
			ImGui::Dummy(ImVec2(0.0f, 10.0f)); // 縦に 10px の余白

		// スコープ終了
		ImGui::PopID();
	}
}

/*
	これはツリー構造でいうの葉っぱのところにあたいするもの
*/
void AssetsPanel::ViewModelLeafTreeNode(int index, const ModelResource& resource)
{

	int alpha = 255;

	/* 選択中は色変え */
	bool isSelected = (selectedHierarchyResource_.pathId_ == resource.pathId_);

	if (isSelected) {
		ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 100, 50, alpha));
	}

	ImGui::PushID(index);


	/* 文字の高さ（上下位置）をボタンの高さにピッタリ合わせる */
	ImGui::AlignTextToFramePadding();


	/* --- テキスト（Selectable化して選択＆ドラッグ対象にする） --- */
	std::string label = std::string(ICON_FA_CUBE) + "  " + resource.name_;

	if (ImGui::Selectable(label.c_str(), isSelected, ImGuiSelectableFlags_None, ImVec2(120.0f, 0.0f))) {
		// クリックされた時しょり（あれば
	}

	/* --- ドラッグ開始処理 --- */
	if (ImGui::BeginDragDropSource()) {
		// 配列番号（index）をペイロードとして渡す
		ImGui::SetDragDropPayload("MODEL", &index, sizeof(int));

		// ドラッグ中にカーソル横に表示するプレビューテキスト
		ImGui::Text("%s", resource.name_.c_str());

		ImGui::EndDragDropSource();
	}

	ImGui::PopID();

	if (isSelected) {
		ImGui::PopStyleColor();
	}
	//ImGui::SameLine(ImGui::GetWindowWidth() - 100.0f);

	/* 見えるか 20261105 これもれいやーできたんで没 */
	ImGui::PushID(index);
	/*
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
	*/
	ImGui::PopID();

	//ImGui::SameLine(ImGui::GetWindowWidth() - 75.0f);

	ImGui::PushID(index);

	/* モデルのリソース選択 */
	/* 20261105　べつにレイヤー機能できたので没 */

	/*
	if (ImGui::Button("Select")) {

		//
		selectedHierarchyResource_ = resource;

		// ビューポートのモデル選びなおし
		selectedModelViewport_.ClearModel();
		selectedModelViewport_.AddModel(resource.model_);


		ImGui::PopID();
		break;
	}
	*/

	ImGui::PopID();

	ImGui::SameLine(ImGui::GetWindowWidth() - 110.0f);


	// ボタンのスタイル設定
	PushButtonStyle();

	ImGui::PushID(index);
	// 選択されたヒエラルキモデルで追加ボタン
	if (ImGui::Button((char*)ICON_FA_SQUARE_PLUS)) {

		//
		AddHierarchyModel(resource);


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

		// ボタンのスタイルおわり
		PopButtonStyle();


		ImGui::PopID();
		return;
	}
	ImGui::PopID();

	ImGui::SameLine();

	ImGui::PushID(index);

	/*
		モデルのリソース削除
	*/
	if (ImGui::Button((char*)ICON_FA_XMARK)) {

		// 削除
		hierarchyPanel_->DeleteHierarchyResouce(index);

		// ボタンのスタイルおわり
		PopButtonStyle();

		ImGui::PopID();
		return;
	}

	// ボタンのスタイルおわり
	PopButtonStyle();


	ImGui::PopID();

}


void AssetsPanel::ViewTextureTreeNode()
{
	for (int folderIndex = 0; folderIndex < textureFolderNames_.size(); folderIndex++)
	{

		const auto& folder = textureFolderNames_.at(folderIndex);


		// Models用の文字列ID（または独自のポインタ）でID空間を区別する
		ImGui::PushID(("Texture_" + folder).c_str());


		// フォルダ表示
		bool isOpen = ImGui::TreeNodeEx(
			folder.c_str(),
			ImGuiTreeNodeFlags_SpanAvailWidth
		);

		/*   -ドラッグ終了地点   */
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload =
				ImGui::AcceptDragDropPayload("TEXTURE"))
			{
				// 受け取ったモデルの配列番号
				int index = *(const int*)payload->Data;

				// モデルの所属フォルダを変更
				hierarchyTextureResouces_[index].folderName_ = folder;
			}

			ImGui::EndDragDropTarget();
		}

		// フォルダの中身 - 表示,一般の処理
		if (isOpen)
		{
			for (int i = 0; i < hierarchyTextureResouces_.size(); i++)
			{
				//
				const auto& resource = hierarchyTextureResouces_.at(i);

				if (resource.folderName_ == folder)
				{
					//ImGui::Text("%s", resource.name_.c_str());

					float size = 24.0f;

					ImGui::PushID(i);

					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));// 枠無くし

					/* 画像表示 - これがドロップ対象になる */
					ImGui::ImageButton(
						"##tex_btn",
						(ImTextureID)resource.texture_->GetTextureSrvHandleGPU().ptr,
						ImVec2(size, size)
					);

					ImGui::PopStyleColor();


					/*   -ドラッグ開始地点-    */
					if (ImGui::BeginDragDropSource())
					{
						//  ドラッグ開始した配列のindexを渡す  //
						ImGui::SetDragDropPayload(
							"TEXTURE",
							&i,
							sizeof(int)
						);

						// 名前
						ImGui::Text(resource.name_.c_str());

						ImGui::EndDragDropSource();
					}

					ImGui::PopID();


					ImGui::SameLine();

					// 名前
					ImGui::Text(resource.name_.c_str());
				}
			}

			ImGui::TreePop();
		}

		// スコープ終了
		ImGui::PopID();
	}
}

void AssetsPanel::AddHierarchyModel(const ModelResource& resource)
{

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

}


void AssetsPanel::PushButtonStyle()
{
	//ImGui::SetWindowFontScale(1.2f);// 一時的に大きく
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);// 枠無くし
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.6f, 1));// カラー

}
void AssetsPanel::PopButtonStyle()
{

	//ImGui::SetWindowFontScale(1.0f);
	ImGui::PopStyleVar();
	ImGui::PopStyleColor();

}