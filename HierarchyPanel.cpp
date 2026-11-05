#include "HierarchyPanel.h"
#include "AssetsPanel.h"

#include "Editor.h"

#include "Scene.h"
#include "GameObject.h"

#include "ImGuiManager.h"

#include "SceneComponentLoader.h"

#include "FileDropManager.h"

#include "UIManager.h"

#include <misc/cpp/imgui_stdlib.h>
#include <imgui_internal.h>

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

void HierarchyPanel::Initialize(const std::string jsonPath, Grid& g)
{	
	grid = &g;

	/* 初期化で読込んどく */
	HierarchyLoad(jsonPath);
}

void HierarchyPanel::Update()
{
	//// モデル更新
	//for (int i = 0; i < hierarchyModelResouces_.size(); i++)
	//{
	//	hierarchyModelResouces_.at(i).model_->Update();
	//}

	// ヒエラルキの更新
	for (int i = 0; i < hierarchyModels_.size(); i++)
	{
		// キャ・・・キャスト
		if (auto* world = dynamic_cast<WorldTransform*>( hierarchyModels_.at(i)->transform_.get())) {

			/* AABB更新 */
			hierarchyModels_.at(i)->aabb_.UpdatePosition(world->translation_);

			// 更新
			world->UpdateMatrix();
		}

	}
}

void HierarchyPanel::ViewPanel(bool& show)
{

#ifdef USE_IMGUI

	/* もし選択中のヒエラルキある場合 */
	if (selectedModelIndex_.has_value()) {

		/* 選択中のヒエラルキのAABB画面 */
		hierarchyModels_.at(selectedModelIndex_.value()).get()->aabb_.ImGuiView();

		auto* it =hierarchyModels_.at(selectedModelIndex_.value()).get()->gameObject_;

		if (it != nullptr) {

			// それに登録されたIMGUI表示！！
			it->ImGuiView();
		}

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
			if (!hierarchyModelToLayer_.contains(key)) continue;

			// マップから該当レイヤーのモデルリストを取得
			auto& value = hierarchyModelToLayer_[key];


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
					ChangeHierarchyModelLayer(true,key);

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
					ChangeHierarchyModelLayer(false, key);

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

			if (ImGui::Button("Select")) {
				selectedCurrentLayer_ = key;
			}
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

					// 選択中のは色変え
					if (selectedModelIndex_.has_value() && selectedModelIndex_.value() == i) {
						ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 100, 50, 255));
						ImGui::BulletText(hierarchy->name_.c_str());
						ImGui::PopStyleColor();
					}
					else {
						ImGui::BulletText(hierarchy->name_.c_str());
					}


					ImGui::SameLine(ImGui::GetWindowWidth() - 200.0f);

					ImGui::PushID(i);

					// 削除ボタンを右寄りに配置（↑のおかげ
					if (ImGui::Button("Del")) {

						// また検索
						auto it = std::find_if(
							hierarchyModels_.begin(),
							hierarchyModels_.end(),
							[hierarchy](const std::unique_ptr<Hierarchy<Model>>& h) {
								return h.get() == hierarchy;
							}
						);

						if (it != hierarchyModels_.end()) {

							// イテレータから番号を取得 (..神?)
							int index = static_cast<int>(std::distance(hierarchyModels_.begin(), it));

							DeleteHierarchyModel(index);
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



// ヒエラルキー - モデル追加. レイヤー名とヒエラルキ
void HierarchyPanel::AddHierarchyModel(std::string layerName, Hierarchy<Model>& h) {

	if (layerName.empty()) return;

	auto it = hierarchyModelToLayer_.find(layerName);
	if (it == hierarchyModelToLayer_.end()) return;

	// 1. unique_ptr を生成（所有権を移動）
	auto newHierarchy = std::make_unique<Hierarchy<Model>>(std::move(h));

	// 2. 移動する前に、確定した安定したアドレスを取得しておく
	Hierarchy<Model>* rawPtr = newHierarchy.get();

	// 3. vector（unique_ptrの配列）に追加
	hierarchyModels_.push_back(std::move(newHierarchy));

	// 4. メモリ領域が絶対に移動しないポインタをレイヤーマップに登録
	it->second.push_back(rawPtr);
}

void HierarchyPanel::DeleteHierarchyModel(int index)
{
		// 引数無しの場合は、今選択しているヒエラルキモデル番号で
		if (index == -1) {
			if (selectedModelIndex_.has_value()) {
				index = selectedModelIndex_.value();
			}
			else {
				return; // 選択してもない場合はサヨナラ
			}
		}

		// 生ポインタを取得しておく
		Hierarchy<Model>* rawTarget = hierarchyModels_.at(index).get();

		/* GameObjectと紐づいてる場合は消す */
		GameObjectDeleteCheck(rawTarget);


		// 削除対象のレイヤーからポインタを外す
		auto& layer = hierarchyModelToLayer_[rawTarget->layerName_];
		auto it = std::find(layer.begin(), layer.end(), rawTarget);
		if (it != layer.end()) {
			layer.erase(it);
		}

		// vector から unique_ptr ごと削除
		hierarchyModels_.erase(hierarchyModels_.begin() + index);

		selectedModelIndex_ = std::nullopt;
}
void HierarchyPanel::DeleteHierarchyLayer(const std::string& layerName)
{
	if (layerName == "main") return;

	auto it = hierarchyModelToLayer_.find(layerName);
	if (it == hierarchyModelToLayer_.end()) return;

	// 登録されている Hierarchy を hierarchyModels_ から削除
	for (Hierarchy<Model>* target : it->second) {

		auto hierarchyIt = std::find_if(
			hierarchyModels_.begin(),
			hierarchyModels_.end(),
			[target](const std::unique_ptr<Hierarchy<Model>>& hierarchy) {
				// unique_ptr が保持する生ポインタ(get())と target を比較する
				return hierarchy.get() == target;
			}
		);

		/* GameObjectと紐づいてる場合は消す */
		GameObjectDeleteCheck(target);


		if (hierarchyIt != hierarchyModels_.end()) {
			hierarchyModels_.erase(hierarchyIt);
		}
	}

	// レイヤー側のマップ要素を削除
	hierarchyModelToLayer_.erase(it);

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
				*GetHierarchyModels().back().get()
			);

		}
	}

}

void HierarchyPanel::GameObjectDeleteCheck(Hierarchy<Model>* hierarchy)
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
			gameObjectDelete_(*hierarchy);

		}
	}
}

WorldTransform* HierarchyPanel::PasteHierarchyModel()
{

	///* モデルのヒエラルキ追加 */
	Hierarchy<Model> h;

	// セレクト中のから各コピー
	h.type_ = hierarchyModels_.at(selectedModelIndex_.value())->type_;
	h.id_ = hierarchyModels_.at(selectedModelIndex_.value())->id_;
	h.name_ = hierarchyModels_.at(selectedModelIndex_.value())->name_;

	h.object = hierarchyModels_.at(selectedModelIndex_.value())->object;

	h.aabb_ = hierarchyModels_.at(selectedModelIndex_.value())->aabb_;

	h.layerName_ = hierarchyModels_.at(selectedModelIndex_.value())->layerName_;

	h.isView_ = hierarchyModels_.at(selectedModelIndex_.value())->isView_;

	// WorldTransform座標決定
	h.transform_ = std::make_unique<WorldTransform>();

	h.transform_->Initialize();
	h.transform_->translation_ = hierarchyModels_.at(selectedModelIndex_.value())->transform_->translation_;
	h.transform_->rotation_ = hierarchyModels_.at(selectedModelIndex_.value())->transform_->rotation_;
	h.transform_->scale_ = hierarchyModels_.at(selectedModelIndex_.value())->transform_->scale_;

	WorldTransform* world = static_cast<WorldTransform*>(h.transform_.get());

	// 位置更新
	world->UpdateMatrix();

	/* ヒエラルキ-モデル - 追加 */
	AddHierarchyModel(selectedCurrentLayer_, h);


	/* シーン側のGameObjectに干渉してたら一緒に増やす */
	GameObjectAddCheck();


	// セレクト中のインデックスを末尾のに変更
	selectedModelIndex_.value() = (int)hierarchyModels_.size() - 1;


	//*  BaseTransform -> WorldTransform
	WorldTransform* rworld = dynamic_cast<WorldTransform*>(hierarchyModels_.back()->transform_.get());

	// ついかしたヒエラルキのワールド
	return rworld;
}


bool HierarchyPanel::DeleteHierarchyResouce(int index)
{
	/*
		GameObjectを含んでるか探索
	*/
	for (int i = 0; i < hierarchyModels_.size(); i++)
	{
		std::unique_ptr<Hierarchy<Model>>& model = hierarchyModels_.at(i);
		
		if (model->gameObject_ != nullptr)
			return false;// 見つかった場合 失敗.  False
	}


	// リソースに紐づけられたIDのモデルを全て削除
	for (int i = 0; i < hierarchyModels_.size(); i++)
	{

		std::unique_ptr<Hierarchy<Model>>& model = hierarchyModels_.at(i);

		if (model->id_ == assetsPanel_->GetHierarchyModelResources().at(index).pathId_) {
			hierarchyModels_.erase(hierarchyModels_.begin() + i);
			i--;
		}
	}

	//　アセットの方も削除
	assetsPanel_->DeleteModelResource(index);
	
	// 先にモデルと紐づけたマップを削除
	//hierarchyModelResourceList_.erase(hierarchyModelResouces_.at(index).pathId_);

	//// モデルのリソース情報削除
	//hierarchyModelResouces_.erase(hierarchyModelResouces_.begin() + index);
	return true;
}

void HierarchyPanel::ChangeHierarchyModelView(bool b, int assetIndex)
{

	assetsPanel_->GetHierarchyModelResources().at(assetIndex).isView_ = b;

	// 対応したアセットが紐づけられたヒエラルキモデル表示・非表示切り替え
	for (int i = 0; i < hierarchyModels_.size(); i++)
	{
		std::unique_ptr<Hierarchy<Model>>& model = hierarchyModels_.at(i);

		if (model->id_ == assetsPanel_->GetHierarchyModelResources().at(assetIndex).pathId_) {
			model->isView_ = b;
		}
	}
}

void HierarchyPanel::ChangeHierarchyModelLayer(bool b, std::string layerName)
{
	for (Hierarchy<Model>* model : hierarchyModelToLayer_[layerName]) {
		
		model->isView_ = b;

		if (model->gameObject_) {
			model->gameObject_->SetView(b);
		}
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
				if (!hierarchyModelToLayer_.contains(inputLayerName)) {

					hierarchyModelToLayer_.try_emplace(inputLayerName);

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


		// ヒエラルキのレイヤー名一覧
		loder->Property("Layers", hierarchyLayers_);

		//// ヒエラルキのモデルのパスリスト 登録
		//loder->Property("ModelResources", hierarchyModelResouces_);

		// ヒエラルキのモデル情報リスト 登録
		loder->Property("ModelProperties", hierarchyModels_);



		loder->EndGroup();
		loder->LoadGroup("Hierarchy");
	}

	grid->Update();


	/* メインのレイヤー */
	selectedCurrentLayer_ = "main";// 選択中にしとく


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

		hierarchyModelToLayer_.try_emplace("main");

		hierarchyLayers_.push_back(HierarchyLayer{
			.layerName_ = "main"
			});
	}


	// 読み込んだリソースのモデルを表示/非表示設定
	for (int i = 0; i < assetsPanel_->GetHierarchyModelResources().size(); i++)
	{
		
		ModelResource& r = assetsPanel_->GetHierarchyModelResources().at(i);

		/* 読み込んだ表示非表示の切り替え */
		ChangeHierarchyModelView(r.isView_,i);
	}


	// 読み込んだレイヤー生成
	for (int i = 0; i < hierarchyLayers_.size(); i++){

		auto name = hierarchyLayers_.at(i).layerName_;

		hierarchyModelToLayer_.try_emplace(name);

	}
	
	// 読み込んだレイヤーに、モデルのアドレス追加
	for (std::unique_ptr<Hierarchy<Model>>& model : hierarchyModels_) {

		hierarchyModelToLayer_[model->layerName_].push_back(model.get());

		// ついでにアドレスも追加
		model->object = assetsPanel_->GethierarchyModelResourceList()[model->id_];

	}
	//
	for (int i = 0; i < hierarchyLayers_.size(); i++) {

		ChangeHierarchyModelLayer(hierarchyLayers_.at(i).isView_, hierarchyLayers_.at(i).layerName_);
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
