#include "HierarchyPanel.h"
#include "AssetsPanel.h"

#include "Editor.h"


#include "GameObject.h"

#include "ImGuiManager.h"

#include "SceneComponentLoader.h"

#include "UIManager.h"

#include <misc/cpp/imgui_stdlib.h>


#include <imgui/IconsFontAwesome.h>

#include <iterator>
#include <algorithm>
#include <memory>

const char* kLayerTypeIcons[] = {
	(char*)ICON_FA_CUBE,
	(char*)ICON_FA_PEN
};

const char* kLayerTypeNames[] = {
	(char*)ICON_FA_CUBE  "  ModelLayer",
	(char*)ICON_FA_PEN  "  CanvasLayer"
};

void HierarchyPanel::Initialize(const std::string sceneName, EditorGrid& g)
{	
	grid = &g;

	/* 初期化で読込んどく */
	HierarchyLoad(sceneName);
}

void HierarchyPanel::Update()
{

	// ヒエラルキの更新
	for (auto& hierarchy : hierarchys_) {

		hierarchy.get()->Update();

	}
}

void HierarchyPanel::ViewPanel(bool& show)
{

#ifdef USE_IMGUI

	/*
		選択ヒエラルキーのImGui表示
	*/
	if (selectedHierarchyNode_.has_value()) {

		selectedHierarchyNode_.value()->ImGuiView();

	}


	/* 色 */
	ImGui::PushStyleColor(
		ImGuiCol_WindowBg,
		ImVec4(0.10f, 0.14f, 0.20f, 1.0f)
	);

	if (ImGui::Begin("Hierarchy", &show)) {

		

		/* レイヤー追加ウインドウ表示 */
		if (ImGui::Button("レイヤー追加")) {
			ImGui::OpenPopup("Popup");
		}


		AddLayerPopupWindow();


		// === ヒエラルキー
		ImGui::SeparatorText("HierarchyLayers");

		std::string deleteLayter = "";
		int layerCount = 0;



		/* ヒエラルキーレイヤーによって分ける */
		for (int layerCount = 0; layerCount < hierarchyLayers_.size(); layerCount++) {

			auto& layerInfo = hierarchyLayers_.at(layerCount);
			const std::string& key = layerInfo.layerName_;

			// マップ内に存在しない場合はスキップ
			if (!hierarchyToLayer_.contains(key)) continue;

			// マップから該当レイヤーのモデルリストを取得
			auto& value = hierarchyToLayer_[key];


			/* 選択中は色変え */
			if (selectedCurrentLayer_ == key) {
				ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 100, 50, 255));
			}


			int layerType = (int)hierarchyLayers_.at(layerCount).layerType_;

			std::string icon = kLayerTypeIcons[layerType];
			std::string layerName = key.c_str();

			ImGui::SetWindowFontScale(1.1f);// 一時的に大きく

			//
			ImGui::AlignTextToFramePadding();


			// レイヤー名
			bool isOpen = ImGui::TreeNodeEx((icon + "  " + layerName).c_str(), ImGuiTreeNodeFlags_SpanLabelWidth);

			//if (previousLayerIsOpen.contains(layerName)) {

			//	// 以前開いていたか
			//	bool& wasOpen = previousLayerIsOpen[layerName];

			//	/* 開かれた瞬間 */
			//	if (isOpen && !wasOpen) {

			//		// キャンバスレイヤーだった場合
			//		if (layerType == (int)HierarchyLayerType::kCanvasLayer) {
			//			UIManager::GetInstance()->SetViewTimeline(true);// 表示
			//		}

			//	}

			//	/* 開じた瞬間 */
			//	if (!isOpen && wasOpen) {
			//	
			//		// キャンバスレイヤーだった場合
			//		if (layerType == (int)HierarchyLayerType::kCanvasLayer) {
			//			UIManager::GetInstance()->SetViewTimeline(false);// 非表示
			//		}

			//	}
			//}

			////
			//previousLayerIsOpen[layerName] = isOpen;


			ImGui::SetWindowFontScale(1.0f);//

			// 色とじ
			if (selectedCurrentLayer_ == key) {
				ImGui::PopStyleColor();
			}

			ImGui::SameLine(ImGui::GetWindowWidth() - 150.0f);

			ImGui::PushID(layerCount);

			// レイヤー非表示切り替え
			if (!hierarchyLayers_.at(layerCount).isView_) {

				if (ImGui::Button("ON")) {

					hierarchyLayers_.at(layerCount).isView_ = true;
					ChangeHierarchyLayer(true,key);

					ImGui::PopID();
					// ツリーが開いていた場合は TreePop を呼んでから抜ける
					if (isOpen) {
						ImGui::TreePop();
					}
					break;
				}
			}
			else {
				if (ImGui::Button("OFF")) {

					hierarchyLayers_.at(layerCount).isView_ = false;
					ChangeHierarchyLayer(false, key);

					ImGui::PopID();
					// ツリーが開いていた場合は TreePop を呼んでから抜ける
					if (isOpen) {
						ImGui::TreePop();
					}
					break;
				}
			}
			ImGui::SameLine();

			/* 選択中レイヤー変更 */
			ImGui::PushID(layerCount);

			/*
				レイヤー変更
			*/
			if (ImGui::Button("Select")) {
				selectedCurrentLayer_ = key;
				selectedCurrentLayerType_ = layerInfo.layerType_;
			}


			if (!previousLayer_.empty()) {

				// レイヤー変更の瞬間
				if (previousLayer_ != selectedCurrentLayer_) {

				}
			}

			//
			previousLayer_ = selectedCurrentLayer_;



			ImGui::PopID();

			ImGui::SameLine(0,-4.0f);

			// レイヤー削除
			if (ImGui::Button("Del")) {

				deleteLayter = key;

				ImGui::PopID();

				// ツリーが開いていた場合は TreePop を呼んでから抜ける
				if (isOpen) {
					ImGui::TreePop();
				}

				break;
			}
			ImGui::PopID();

			
			
			if (isOpen){

				/* ヒエラルキー小分け */
				for (int i = 0; i < value.size(); i++) {
					
					auto& hierarchy = value.at(i);

					if (selectedHierarchyIndex_.has_value()) {

						// 選択中のは色変え
						if (selectedHierarchyIndex_.value() == i)
						{

							ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 100, 50, 255));
							ImGui::BulletText(hierarchy->name_.c_str());
							ImGui::PopStyleColor();
						}
						else {
							ImGui::BulletText(hierarchy->name_.c_str());
						}

					}


					ImGui::SameLine(ImGui::GetWindowWidth() - 200.0f);

					ImGui::PushID(i);

					// 削除ボタンを右寄りに配置（↑のおかげ
					if (ImGui::Button("Del")) {

						// また検索
						auto it = std::find_if(
							hierarchys_.begin(),
							hierarchys_.end(),
							[hierarchy](const auto& h) {
								return h.get() == hierarchy;
							}
						);

						if (it != hierarchys_.end()) {

							// イテレータから番号を取得 (..神?)
							int index = static_cast<int>(std::distance(hierarchys_.begin(), it));

							//DeleteHierarchy(index);
						}

						ImGui::PopID();
						break;
					}
					

					ImGui::PopID();

				}

				

				ImGui::TreePop();
			}
		}

		// ループ抜けた後削除
		if (!deleteLayter.empty()) {
			DeleteHierarchyLayer(deleteLayter);
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



// ヒエラルキー - 追加. レイヤー名とヒエラルキ
void HierarchyPanel::AddHierarchy(std::string layerName, std::unique_ptr<HierarchyNode> h) {

	if (layerName.empty()) return;

	auto it = hierarchyToLayer_.find(layerName);
	if (it == hierarchyToLayer_.end()) return;

	// 1. unique_ptr を生成（所有権を移動）
	auto newHierarchy = std::move(h);

	// 2. 移動する前に、確定した安定したアドレスを取得しておく
	HierarchyNode* rawPtr = newHierarchy.get();

	// 3. vector（unique_ptrの配列）に追加
	hierarchys_.push_back(std::move(newHierarchy));

	// 4. メモリ領域が絶対に移動しないポインタをレイヤーマップに登録
	it->second.push_back(rawPtr);
}

//
//
//// ヒエラルキ　削除
//void HierarchyPanel::DeleteHierarchy(int index)
//{
//	// 引数無しの場合は、今選択しているヒエラルキ番号で
//	if (index == -1) {
//		if (selectedHierarchyNode_.has_value()) {
//			index = selectedHierarchyIndex_.value();
//		}
//		else {
//			return; // 選択してもない場合はサヨナラ
//		}
//	}
//
//	// 生ポインタを取得しておく
//	HierarchyNode* rawTarget = hierarchys_.at(index).get();
//
//
//	/* ModelHierarchyで、なおかつGameObjectと紐づいてる場合は消す */
//	if (rawTarget->type_ == HierarchyType::kModel) {
//
//		GameObjectDeleteCheck(dynamic_cast<HierarchyNode*>(rawTarget));
//
//	}
//
//
//	// 削除対象のレイヤーからポインタを外す
//	auto& layer = hierarchyToLayer_[rawTarget->layerName_];
//	auto it = std::find(layer.begin(), layer.end(), rawTarget);
//	if (it != layer.end()) {
//		layer.erase(it);
//	}
//
//	// vector から unique_ptr ごと削除
//	hierarchys_.erase(hierarchys_.begin() + index);
//
//	selectedHierarchyNode_ = std::nullopt;
//}



/* レイヤーごと消した処理 */
void HierarchyPanel::DeleteHierarchyLayer(const std::string& layerName)
{
	if (layerName == "main") return;

	auto it = hierarchyToLayer_.find(layerName);
	if (it == hierarchyToLayer_.end()) return;

	// 登録されている Hierarchy を hierarchyModels_ から削除
	for (HierarchyNode* target : it->second) {

		auto hierarchyIt = std::find_if(
			hierarchys_.begin(),
			hierarchys_.end(),
			[target](const std::unique_ptr<HierarchyNode>& hierarchy) {

				// unique_ptr が保持する生ポインタ(get())と target を比較する
				return hierarchy.get() == target;
			}
		);

		/* GameObjectと紐づいてる場合は消す */
		if (target->type_ == HierarchyType::kModel) {

			GameObjectDeleteCheck(dynamic_cast<HierarchyNode*>(target));

		}

		if (hierarchyIt != hierarchys_.end()) {
			hierarchys_.erase(hierarchyIt);
		}
	}

	// レイヤー側のマップ要素を削除
	hierarchyToLayer_.erase(it);

	// hierarchyLayers_（レイヤー情報リスト）からの削除
	auto itt = std::find_if(
		hierarchyLayers_.begin(),
		hierarchyLayers_.end(),
		[&layerName](const HierarchyLayer& l) {
			return l.layerName_ == layerName;
		}
	);

	// あったら名前一覧から削除
	if (itt != hierarchyLayers_.end()) {
		hierarchyLayers_.erase(itt);
	}

	auto ittt = std::find(
		alignHierarchyLayerNames_.begin(),
		alignHierarchyLayerNames_.end(),
		layerName
	);

	// あったら名前一覧から削除
	if (ittt != alignHierarchyLayerNames_.end()) {
		alignHierarchyLayerNames_.erase(ittt);
	}

}


void HierarchyPanel::GameObjectAddCheck()
{
	// 今開いてるレイヤー検索
	auto it = std::find_if(
		hierarchyLayers_.begin(),
		hierarchyLayers_.end(),
		[this](const HierarchyLayer& layer) {
			return layer.layerName_ == selectedCurrentLayer_;
		}
	);

	//* もし、シーン側で登録しているレイヤーで生成された場合 *//
	if (it->isAlignGameScene) {

		// もし、シーン側で登録しているレイヤーで生成された場合、シーン側のゲームオブジェクトも生成する
		if (gameObjectAdd_ != nullptr) {

			/* 
				種類２：サブ種類０を、仮だけど錬成する
			*/

			gameObjectAdd_(
				2,
				0,
				*dynamic_cast<ModelNode*>(GetHierarchys().back().get())
			);

		}
	}

}


/* ゲームオブジェクト */
void HierarchyPanel::GameObjectDeleteCheck(HierarchyNode* hierarchy)
{
	// 今開いてるレイヤー検索
	auto it = std::find_if(
		hierarchyLayers_.begin(),
		hierarchyLayers_.end(),
		[this](const HierarchyLayer& layer) {
			return layer.layerName_ == selectedCurrentLayer_;
		}
	);

	//* もし、シーン側で登録しているレイヤーで消された場合 *//
	if (it->isAlignGameScene) {

		if (gameObjectDelete_ != nullptr) {

			// kesu
			gameObjectDelete_(*dynamic_cast<ModelNode*>(hierarchy));

		}
	}
}

WorldTransform* HierarchyPanel::PasteHierarchyModel()
{

	/* 
		ペースト元のヒエラルキの型に合わせたクローンを生成 
	*/
	auto h = selectedHierarchyNode_.value()->Clone();

	/* ヒエラルキ- 追加 */
	AddHierarchy(selectedCurrentLayer_, std::move(h));


	/* シーン側のGameObjectに干渉してたら一緒に増やす */
	GameObjectAddCheck();


	// セレクト中のインデックスを末尾のに変更
	selectedHierarchyIndex_.value() = (int)hierarchys_.size() - 1;


	//*  BaseTransform -> WorldTransform
	WorldTransform* rworld = dynamic_cast<WorldTransform*>(hierarchys_.back()->transform_.get());

	// ついかしたヒエラルキのワールド
	return rworld;
}

bool HierarchyPanel::DeleteHierarchyNode(HierarchyNode* target)
{
	if (!target) return false;

	// 1. GameObjectと紐づいている場合の削除チェック
	if (target->type_ == HierarchyType::kModel) {
		GameObjectDeleteCheck(target);
	}

	// 2. hierarchyToLayer_ (参照用マップ) からポインタを除去
	auto layerIt = hierarchyToLayer_.find(target->layerName_);
	if (layerIt != hierarchyToLayer_.end()) {
		auto& layerVector = layerIt->second;
		auto it = std::find(layerVector.begin(), layerVector.end(), target);
		if (it != layerVector.end()) {
			layerVector.erase(it);
		}
	}

	// 3. hierarchys_ (本体のunique_ptr配列) から削除
	auto it = std::find_if(
		hierarchys_.begin(),
		hierarchys_.end(),
		[target](const std::unique_ptr<HierarchyNode>& h) {
			return h.get() == target;
		}
	);

	if (it != hierarchys_.end()) {
		hierarchys_.erase(it);
		return true;
	}

	return false;
}


void HierarchyPanel::ChangeHierarchyModelView(bool b, int assetIndex)
{

	AssetsPanel::GetInstance()->GetHierarchyModelResources().at(assetIndex).isView_ = b;

	/* 
		対応したアセットが紐づけられたヒエラルキモデル表示・非表示切り替え。
		..もしモデルで、かつ対応するIDなら非表示表示きりかえ
	*/
	for (int i = 0; i < hierarchys_.size(); i++)
	{
		
		if (hierarchys_.at(i).get()->type_ != HierarchyType::kModel)
			continue;

		ModelNode* model = dynamic_cast<ModelNode*>(hierarchys_.at(i).get());

		if (model->modelId_ == AssetsPanel::GetInstance()->GetHierarchyModelResources().at(assetIndex).pathId_) {
			model->isView_ = b;
		}
	}
}

void HierarchyPanel::ChangeHierarchyLayer(bool b, std::string layerName)
{
	// レイヤー名のヒエラルキをリストから全て取得
	for (HierarchyNode* h: hierarchyToLayer_[layerName]) {

		// 切り替え
		h->ChangeHierarchyLayer(b);

	}
}

void HierarchyPanel::AddLayerPopupWindow()
{
	
	// ポップアップ本体
	if (ImGui::BeginPopup("Popup")) {

		ImGui::Text("レイヤー追加");

		ImGui::Text("レイヤー名：");
		ImGui::SameLine();


		static std::string inputLayerName;
		ImGui::InputText(" ", &inputLayerName);


		ImGui::Text("レイヤータイプ：");
		ImGui::SameLine();

		/* レイヤータイプを選択 */
		ImGui::Combo("type", &currentSelectedLayerType_, kLayerTypeNames, IM_ARRAYSIZE(kLayerTypeNames));


		/* レイヤー操作 */
		if (ImGui::Button("Add")) {

			if (!inputLayerName.empty()) {

				// 追加
				if (!hierarchyToLayer_.contains(inputLayerName)) {

					hierarchyToLayer_.try_emplace(inputLayerName);

					// レイヤー
					hierarchyLayers_.push_back(HierarchyLayer{
						.layerType_ = (HierarchyLayerType)currentSelectedLayerType_,
						.layerName_ = inputLayerName
						});

					// 閉じる
					ImGui::CloseCurrentPopup();

				}
			}
		}

		ImGui::EndPopup();
	}

}

void HierarchyPanel::HierarchyClear()
{
	hierarchys_.clear();
}



void HierarchyPanel::HierarchyLoad(const std::string sceneName) {

	if (sceneName.empty()) return;

	/* 
		読み込むjsonのパスをここで設定
	*/
	
	jsonPath_ = "resources/Scene/" + sceneName + "/hierarchy.json";
	

	/* 
		もし存在しない場合つくる
	*/  
	std::filesystem::path path(jsonPath_);

	if (!std::filesystem::exists(path)) {

		// 親ディレクトリを作成
		std::filesystem::create_directories(path.parent_path());

		// JSONファイルを新規作成
		std::ofstream file(path);

		if (!file.is_open()) return;

		file << "{}";
		file.close();
	}


	// 読込み
	if (SceneComponentLoader::GetInstance()->Load(jsonPath_)) {

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


		// ヒエラルキのレイヤー名一覧
		loder->Property("Layers", hierarchyLayers_);

		//// ヒエラルキのモデルのパスリスト 登録
		//loder->Property("ModelResources", hierarchyModelResouces_);

		// ヒエラルキのモデル情報リスト 登録
		loder->Property("ModelProperties", hierarchys_);



		loder->EndGroup();
		loder->LoadGroup("Hierarchy");
	}


	grid->Update();


	/* メインのレイヤー */
	selectedCurrentLayer_ = "main";// 選択中にしとく
	selectedCurrentLayerType_ = HierarchyLayerType::kModelLayer;

	// == Main探し
	auto it = std::find_if(
		hierarchyLayers_.begin(),
		hierarchyLayers_.end(),
		[](const HierarchyLayer& l) {
			return l.layerName_ == "main";
		}
	);

	// 無ければ
	if ( it== hierarchyLayers_.end() ) {

		hierarchyToLayer_.try_emplace("main");

		hierarchyLayers_.push_back(HierarchyLayer{
			.layerName_ = "main"
			});
	}


	// 読み込んだリソースのモデルを表示/非表示設定
	for (int i = 0; i < AssetsPanel::GetInstance()->GetHierarchyModelResources().size(); i++)
	{
		
		ModelResource& r = AssetsPanel::GetInstance()->GetHierarchyModelResources().at(i);

		/* 読み込んだ表示非表示の切り替え */
		ChangeHierarchyModelView(r.isView_,i);
	}


	// 読み込んだレイヤー生成
	for (int i = 0; i < hierarchyLayers_.size(); i++){

		auto name = hierarchyLayers_.at(i).layerName_;

		hierarchyToLayer_.try_emplace(name);

	}
	
	// 読み込んだレイヤーに、ヒエラルキのアドレス追加
	for (std::unique_ptr<HierarchyNode>& h : hierarchys_) {

		hierarchyToLayer_[h->layerName_].push_back(h.get());

		if (h.get()->type_ != HierarchyType::kModel) 
			continue;

		/* 
			もでる専用処理
		*/
		auto* model = dynamic_cast<ModelNode*>(h.get());

		// ついでにアドレスも追加
		model->modelResource_ = AssetsPanel::GetInstance()->GethierarchyModelResourceList()[model->modelId_];

	}
	//
	for (int i = 0; i < hierarchyLayers_.size(); i++) {

		ChangeHierarchyLayer(hierarchyLayers_.at(i).isView_, hierarchyLayers_.at(i).layerName_);
	}

}

void HierarchyPanel::HierarchySave() {

	if (jsonPath_.empty()) return;

	// ※注　ポインタ
	auto loder = SceneComponentLoader::GetInstance();

	if (loder->SaveSwitchJson(jsonPath_)) {

		// ライト配置のセーブ
		SceneComponentLoader::GetInstance()->LightSave();

		loder->SaveGroup("Grid");
		loder->SaveGroup("Hierarchy");

		SceneComponentLoader::GetInstance()->Save(jsonPath_);
	}
}
