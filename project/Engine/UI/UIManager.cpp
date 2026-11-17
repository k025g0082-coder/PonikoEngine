#include "UIManager.h"

#include "ImGuiManager.h"
#include "Editor.h"

#include "Input.h"

//#include "ImGizmo2D.h"

#include "Timer.h"

#include "SceneComponentLoader.h"

#include <misc/cpp/imgui_stdlib.h>
#include <imgui/IconsFontAwesome.h>

#include "AssetsPanel.h"

SceneUIData* UIManager::currentSceneUIData_;

UIElement* UIManager::selectedElemet_ = nullptr;

std::string UIManager::currentSelectAnimationTag;

UIElement* UIManager::selectedParentElemet_ = nullptr;
UIElement* UIManager::selectedChildElemet_ = nullptr;

HierarchyPanel* UIManager::hierarchyPanel_ = nullptr;


UIManager* UIManager::GetInstance()
{
	static UIManager instance;
	return &instance;
}

void UIManager::Initialize(std::string sceneName)
{
	// 新しく生成して　それをターゲットにする
	currentSceneUIData_ = &sceneUIDatas_[sceneName];

	/*
		ElementData読み込み
	*/
	LoadElementData(sceneName);

}

void UIManager::LoadElementData(std::string sceneName)
{

	/*
		読み込むjsonのパスをここで設定
	*/

	jsonPath = "resources/Scene/" + sceneName + "/UIProperty.json";


	/*
		もし存在しない場合つくる
	*/
	std::filesystem::path path(jsonPath);

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
	if (SceneComponentLoader::GetInstance()->Load(jsonPath)) {

		SceneComponentLoader* loder = SceneComponentLoader::GetInstance();

		//
		loder->BeginGroup("UI");

		// UIData一覧
		loder->Property("data", currentSceneUIData_->elementDatas_);


		loder->EndGroup();

		// 読み込み
		loder->LoadGroup("UI");
	}

	for (UIData& data : currentSceneUIData_->elementDatas_) {
		for (auto& e : data.elements_) {

			// 名前がない場合付ける
			if (e->name_.empty()) {

				e->name_ = "element_" + std::to_string( currentSceneUIData_->nameRegisterCount_ );

				currentSceneUIData_->nameRegisterCount_++;//
			}

			//　各初期化
			e->Initialize();

			/*
				親設定　同じキャンバス内の中から一致する親名のアドレスを設定する
			*/
			if (!e->parentName_.empty()) {

				std::string& parentName = e->parentName_;

				// 一致する親名のエレメント検索
				auto it = std::find_if(
					data.elements_.begin(),
					data.elements_.end(),
					[parentName](auto& element) {
						return parentName == element->name_;
					}
				);

				if (it != data.elements_.end()) {
					e->parent_ = (*it).get();

					// 親側にも追加
					e->parent_->childrens_.push_back(e.get());

					e->GetScreenTransform().parent_ = &e->parent_->GetScreenTransform();// Spriteも親登録
				}

			}
		}
	}
}

void UIManager::SaveElementData()
{
	if (SceneComponentLoader::GetInstance()->Load(jsonPath)) {

		SceneComponentLoader* loder = SceneComponentLoader::GetInstance();

		/* セーブ前のJson切り替え */
		if (loder->SaveSwitchJson(jsonPath)) {

			loder->SaveGroup("UI");

			SceneComponentLoader::GetInstance()->Save(jsonPath);
		}
	}
}

void UIManager::SaveElementPrefabData()
{

	const std::string path = "resources/Prefab/UI/"+ selectedElemet_->name_ + ".json";


	SceneComponentLoader* loder = SceneComponentLoader::GetInstance();


	// JSONデータ化
	json prefabArray = nlohmann::json::array();

	// 子からそのひ孫まで関係者はすべて詰める
	SerializeElementRecursive(selectedElemet_, prefabArray);


	// セーブ
	SceneComponentLoader::GetInstance()->Save(prefabArray,path);
	

	/*
		AssetsにもPrefabのUIEぇめんｔとしてリソース情報（ＪＳＯＮパス）追加
	*/
	// 4. AssetsPanel にも重複しないようにリソース登録
	auto& uiResources = AssetsPanel::GetInstance()->GetHierarchyUIElementResources();
	auto it = std::find_if(
		uiResources.begin(),
		uiResources.end(),
		[&path](const UIElementResource& res) {
			return res.pathId_ == path;
		}
	);

	// まだ未登録のプレハブなら新規追加
	if (it == uiResources.end()) {
		UIElementResource r;
		r.name_ = selectedElemet_->name_;
		r.pathId_ = path;
		r.folderName_ = "Main"; // デフォルトフォルダ
		uiResources.push_back(r);
	}
}

// 再帰で自分と子孫をすべてJSON配列に詰めるローカル関数
void UIManager::SerializeElementRecursive(UIElement* element, nlohmann::json& outArray) {
	nlohmann::json j;
	element->Serialize(j);// まず自分
	outArray.push_back(j);

	for (UIElement* child : element->childrens_) {
		SerializeElementRecursive(child, outArray);// 子の分だけ再帰するので全部詰めれる
	}
}

void UIManager::SwitchSceneUIData(std::string sceneName)
{



	// もし既に読み込んでいたら
	if (sceneUIDatas_.contains(sceneName)) {

		// 切り替え
		currentSceneUIData_ = &sceneUIDatas_[sceneName];

		/* データを読み書きする、JsonPathを更新 */
		jsonPath = "resources/Scene/" + sceneName + "/UIProperty.json";

		
	}
	else {

		// 空を生成して切り替えしておく
		currentSceneUIData_ = &sceneUIDatas_[sceneName];

		// 新規ロード
		LoadElementData(sceneName);

	}

}

void UIManager::UpdateElements()
{
#ifdef USE_IMGUI

	ImGui::Begin("UIManager");

	// セーブ
	if (ImGui::Button("save")) {

		SaveElementData();
	
	}
	
	ImGui::SameLine();

	if (selectedElemet_) {

		// 外部ファイル化
		if (ImGui::Button("Prefab保存")) {

			SaveElementPrefabData();

		}
	}
	//
	SelectedElementAnimationView();

	//
	currentSceneUIData_->sequencer_.AnimationComboView();


	ImGui::End();

#endif

	// 一度でも各エレメントの矩形内でクリックされたか
	bool anyInside = false;

	//　ElementDataの更新
	for (auto& data : currentSceneUIData_->elementDatas_) {

		//
		deleteElementIndex_ = -1;

		for (int i = 0; i < data.elements_.size(); i++)
		{
			auto& e = data.elements_.at(i);

			// あれば
			e->Update();


			/*
				再生中のアニメの更新
			*/
			UpdateElementAnimation(e.get());


			// スプライトの座標更新
			e->GetScreenTransform().UpdateMatrix();


			// もし現在開いてるレイヤー以外のエレメントは、選択できないようにする
			if (data.name_ != hierarchyPanel_->GetSelectedCurrentLayerName()) {
				continue;
			}

		}
			
#ifdef USE_IMGUI

			/*
			*  いつかやる！！
			*  
			ImGizmo2D::SetDrawList(ImGui::GetWindowDrawList());
			ImGizmo2D::SetViewRect(canvasPos, canvasSize);
			ImGizmo2D::SetViewTransform(0, 0, 1.0f);
			ImGizmo2D::BeginFrame();
			*/

		const auto& ordered = GetOrderedElements(data);

		for (int i = static_cast<int>(ordered.size()) - 1; i >= 0; --i) {

			UIElement* target = ordered[i];
			/*
				マウス座標と大きさをもとにElementを選択したり削除したり
			*/
			JudgeMouseSelectElement(*target, i, anyInside);

			if (anyInside)
				break;
		}
#endif

			

		

		// もし削除対象があったら
		if (deleteElementIndex_ != -1) {

			selectedElemet_ = nullptr;

			// 削除対象の生ポインタを取得しておく
			UIElement* target = data.elements_.at(deleteElementIndex_).get();

			// 親がある場合、その子リストからも削除
			if (target->parent_) {
				std::erase_if(target->parent_->childrens_, [target](UIElement* child) {
					return child == target;
					});
			}

			// 削除対象を親に設定されているエレメントにはnullを
			for (auto& e : data.elements_) {
				if (e->parent_ == target) {
					e->GetScreenTransform().translation_ += target->GetScreenTransform().translation_;
					e->parent_ = nullptr;
					e->GetScreenTransform().parent_ = nullptr;
				}
			}


			// リストから削除
			data.elements_.erase(data.elements_.begin() + deleteElementIndex_);

			// シーケンサーの方のにもnullをセット
			currentSceneUIData_->sequencer_.SetNullSelectElement();
		}
	}
	
#ifdef USE_IMGUI
	
	/* Editorのメインビュー内にマウスがあるか. */
	bool inside = Editor::GetInstance()->IsMainViewInside();

	/*
		もし一度も各エレメントの矩形内でクリックされていない & Editorのメインビュー内である場合リセット
	*/
	if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !anyInside && inside) {
		

		// モーダルが開いていない時のみリセット
		if (!ImGui::IsPopupOpen("ParentElementPopup")) {
			selectedElemet_ = nullptr;
			selectedParentElemet_ = nullptr;
			selectedChildElemet_ = nullptr;
		}
	}


	//
	ParentSelectPopupWindow();

	//
	ElementNamePopupWindow();


#endif

	// もしキャンバスレイヤーじゃなきゃここで抜ける
	if (hierarchyPanel_->GetSelectedCurrentLayerType() != HierarchyLayerType::kCanvasLayer) return;

#ifdef USE_IMGUI

	if (selectedElemet_) {

		/*
			選択されているElementを移動
		*/
		MoveSelectElement();

	}

	// タイムライン
	UIEditorTimeLine();

#endif
}

void UIManager::DrawElements()
{

	//　ElementDataの描画
	for (UIData& data : currentSceneUIData_->elementDatas_) {

		if (!data.isView_)continue;

		/*
			子が親より前になるよう、再帰的に並び変え 
		*/
		const auto& list = GetOrderedElements(data);

		// 描画
		for (auto* e : list) {
			e->Draw();
		}

	}

}

void UIManager::UpdateElementAnimation(UIElement* element)
{
	// 現在設定されている、タグのアニメ
	auto& anime_ = element->animationDatas_.animations_[element->animationDatas_.currentTag_];

	/*
		アニメーション時間を加算
	*/ 
	if (anime_.isPlaying) {

		const float kRate = 60.0f;// 60fps
		anime_.frame += Timer::GetDeltaTime() * kRate;

	}
	else {
		return;// 再生されてなければ抜ける
	}


	// UIElementのアニメデータのトラックたち
	auto& tracks_ = anime_.tracks;

	for (int track = 0; track < tracks_.size(); ++track)
	{

		// トラックのデータのアイテム
		auto& items_ = tracks_.at(track).items;

		/*
			各アイテム　　Transform:座標,拡大,回転　Color:R,G,B,アルファ・・
		*/
		for (int i = 0; i < items_.size(); ++i)
		{
			// 全てのキー
			for (int j = 0; j < items_.at(i).keyFrames.size(); ++j) {

				KeyFrame<Vector3>& key = items_.at(i).keyFrames.at(j);

				/// 補間
				UISequencer::LerpKeyFrame(
					element,
					element->animationDatas_.animations_[element->animationDatas_.currentTag_].frame,// 現在の経過時間。
					track,
					i,
					j
				);

			}
			
		}
	}
}

// ゴミ　じゃあ何？登録時にマップとかでアクセスできた方がいいんか？？？？？？？
void UIManager::PlayAnimationElement(std::string canvasName, std::string elementName, std::string elementAnimationTag)
{

	//該当レイヤー検索
	auto it = std::find_if(
		currentSceneUIData_->elementDatas_.begin(),
		currentSceneUIData_->elementDatas_.end(),
		[canvasName](const UIData& data_) {
			return data_.name_ == canvasName;
		}
	);
	if (it != currentSceneUIData_->elementDatas_.end()) {

		// 該当エレメント検索
		auto itt = std::find_if(
			it->elements_.begin(),
			it->elements_.end(),
			[elementName](auto& data_) {
				return data_->name_ == elementName;
			}
		);

		if (itt != it->elements_.end()) {

			// 現在のアニメを指定
			(*itt)->animationDatas_.currentTag_ = elementAnimationTag;

			// 再生時間リセット
			(*itt)->animationDatas_.animations_[elementAnimationTag].frame = 0.0f;

			// 再生開始
			(*itt)->animationDatas_.animations_[elementAnimationTag].isPlaying = true;

		}
	}

}

void UIManager::SelectedElementAnimationView()
{
#ifdef USE_IMGUI

	/*
		なんかUI用のエレメント追加ボタン
	*/
	ImGui::SameLine(ImGui::GetWindowWidth() - 100.0f);
	if (ImGui::Button("ﾃｷｽﾄ追加")) {

		//
		AddUIElement(UIType::kText, Editor::GetInstance()->GetHierarchyPanel().GetSelectedCurrentLayerName());

	}

	if (selectedParentElemet_) {
		ImGui::Text("親選択：%s", selectedParentElemet_->name_.c_str());
	}
	else {
		ImGui::Text("親選択：なし");
	}


	if (selectedChildElemet_) {
		ImGui::Text("子選択：%s", selectedChildElemet_->name_.c_str());
	}
	else {
		ImGui::Text("子選択：なし");
	}


	if (selectedElemet_) {

		//
		ImGui::AlignTextToFramePadding();

		ImGui::Text("エレメント名：%s", selectedElemet_->name_.c_str());

		ImGui::SameLine(ImGui::GetWindowWidth() - 100.0f);
		if (ImGui::Button("名前変更")) {

			//f
			isShowNameWindow_ = true;
		}


		if (selectedElemet_->parent_) {
			
			ImGui::Text("親：%s", selectedElemet_->parent_->name_.c_str());

			/* 親解除 */

			if (ImGui::Button("解除")) {

				// 親の相対座標ー＞スクリーン上の座標に戻す（座標を足すだけ）
				selectedElemet_->GetScreenTransform().translation_ += selectedElemet_->parent_->GetScreenTransform().translation_;

				// 親の子リストからも削除

				std::erase_if(
					selectedElemet_->parent_->childrens_,
					[this](UIElement* child) {
						return child == selectedElemet_;
					}
				);

				selectedElemet_->parent_ = nullptr;
				selectedElemet_->GetScreenTransform().parent_ = nullptr;// ScreenTransformも

			}
		}
		else {
			ImGui::Text("親：なし");
		}

		ImGui::SeparatorText("BaseTransform");
		ImGui::Dummy(ImVec2{ 0,4.0f });

		if (ImGui::DragFloat3("座標", &selectedElemet_->GetScreenTransform().translation_.x, 0.1f)) {
			selectedElemet_->position_ = selectedElemet_->GetScreenTransform().translation_;// ベース座標も同期
		}


		ImGui::SameLine(ImGui::GetWindowWidth() - 50.0f);

		ImGui::PushID("b1");
		if (ImGui::Button((char*)ICON_FA_KEY)) {

			//
			if (selectedElemet_) {

				//座標
				Vector3 vec = selectedElemet_->GetScreenTransform().translation_;

				// キーフレーム追加
				currentSceneUIData_->sequencer_.AddKeyFrame(SequenceItemType::kPosition, KeyFrame<Vector3>{
					currentSceneUIData_->sequencer_.GetCurrentFrame(),
						vec,
						EasingType::None
				});
			}

		}
		ImGui::PopID();


		if (ImGui::DragFloat3("拡大", &selectedElemet_->GetScreenTransform().scale_.x, 0.1f)) {
			selectedElemet_->scale_ = selectedElemet_->GetScreenTransform().scale_;// ベース大きさも同期
		}

		ImGui::SameLine(ImGui::GetWindowWidth() - 50.0f);

		ImGui::PushID("b2");
		if (ImGui::Button((char*)ICON_FA_KEY)) {

			if (selectedElemet_) {

				//　スケール
				Vector3	scale = selectedElemet_->GetScreenTransform().scale_;

				// キーフレーム追加
				currentSceneUIData_->sequencer_.AddKeyFrame(SequenceItemType::kScale, KeyFrame<Vector3>{
					currentSceneUIData_->sequencer_.GetCurrentFrame(),
						scale,
						EasingType::None
				});
			}
		}
		ImGui::PopID();


		if (ImGui::DragFloat3("回転", &selectedElemet_->GetScreenTransform().rotation_.x, 0.1f)) {
			selectedElemet_->rotation_ = selectedElemet_->GetScreenTransform().rotation_;// ベース回転も同期
		}


		ImGui::SameLine(ImGui::GetWindowWidth() - 50.0f);

		ImGui::PushID("b3");
		if (ImGui::Button((char*)ICON_FA_KEY)) {
			//
			if (selectedElemet_) {

				//回転
				Vector3 rot = selectedElemet_->GetScreenTransform().rotation_;

				// キーフレーム追加
				currentSceneUIData_->sequencer_.AddKeyFrame(SequenceItemType::kRotation, KeyFrame<Vector3>{
					currentSceneUIData_->sequencer_.GetCurrentFrame(),
						rot,
						EasingType::None
				});
			}
		}
		ImGui::PopID();

		ImGui::Dummy(ImVec2{ 0,4.0f });
		ImGui::SeparatorText("Animations");
		ImGui::Dummy(ImVec2{ 0,4.0f });

		/*
		    それぞれにあわせたImGui表示
		*/
		selectedElemet_->ImGuiView();

		ImGui::Dummy(ImVec2{ 0,.0f });
	}

#endif

}

void UIManager::JudgeMouseSelectElement(UIElement& e,int index,bool& anyInside)
{
#ifdef USE_IMGUI

	// 現在のEditorのMainViewの比率を得る
	float radio = Editor::GetInstance()->GetMainViewRatio();


	// 親のスケールを考慮した最終スケールを取得
	Vector2 worldScale = e.GetScale();
	UIElement* p = e.parent_;
	while (p) {
		worldScale.x *= p->GetScale().x;
		worldScale.y *= p->GetScale().y;
		p = p->parent_;
	}

	// 比率を掛けた中心座標
	Vector2 center = {
		.x = e.GetCenterPosition().x * radio,
		.y = e.GetCenterPosition().y * radio
	};

	// 比率とスケールを掛けた実際の見た目サイズ（Textなら文字全体の幅・高さが返る）
	Vector2 size = {
		.x = e.GetWidth()  * worldScale.x * radio,
		.y = e.GetHeight() * worldScale.y * radio
	};

	Vector2 min = { center.x - size.x * 0.5f, center.y - size.y * 0.5f };
	Vector2 max = { center.x + size.x * 0.5f, center.y + size.y * 0.5f };



	/* かくエレメントのスプライトの矩形内にまうすがあるか. */
	inside = Editor::GetInstance()->IsMainViewInsideMouse(min, max);


	/*
		左マウス - 選択
	*/
	if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {

		/*  
			コントロールキーが押されていた場合 親エレメントの選択とする
		*/
		if (ImGui::IsKeyDown(ImGuiKey_ReservedForModCtrl)) {

			if (inside) {
				// 矩形内

				anyInside = true;

				if (!selectedParentElemet_) {
					// 1回目のCtrl+クリック：親に指定
					selectedParentElemet_ = &e;
				}
				// ※もし親が選択元の自分でなかったら
				else if (selectedParentElemet_->name_ != e.name_) {

					if (!isShowParentWindow_) {

						isShowParentWindow_ = true;

						// 子を設定
						selectedChildElemet_ = &e;

							

					}

				}

			}

		}
		else {

			/*  
				ドラッグの対象の選択
			*/
			if (inside) {

				anyInside = true;

				// 対象を選択
				selectedElemet_ = &e;
				isDragSelectedElement_ = true; // ドラッグ中

				/*
					新たに選択されたので、アニメーションのデータを受け渡す
				*/
				currentSceneUIData_->sequencer_.SetUIElementAnimationData(*selectedElemet_);

			}
		}
	}
	/*
		右マウス - 削除
	*/
	if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
		if (inside) {
			deleteElementIndex_ = index;
		}
	}
#endif
}

void UIManager::ParentSelectPopupWindow()
{
#ifdef USE_IMGUI

	// ポップアップ開く要求フラグが立っていたら1度だけ実行
	if (isShowParentWindow_) {
		ImGui::OpenPopup("ParentElementPopup");
		isShowParentWindow_ = false; // 処理したので戻す
	}

	// ポップアップ描画
	if (ImGui::BeginPopupModal("ParentElementPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {

		if (selectedParentElemet_ && selectedChildElemet_) {
			ImGui::Text("指定エレメントを子要素として親子関係にします");
			ImGui::Text("よろしいですか？");
			ImGui::Dummy(ImVec2{ 0, 4.0f });

			ImGui::Text("親：%s", selectedParentElemet_->name_.c_str());
			ImGui::Text("子：%s", selectedChildElemet_->name_.c_str());
			ImGui::Dummy(ImVec2{ 0, 4.0f });

			if (ImGui::Button("OK", ImVec2(80, 0))) {

				/* 新しい親設定 */
				selectedChildElemet_->SetParent(selectedParentElemet_);

				// ポインタクリア
				selectedParentElemet_ = nullptr;
				selectedChildElemet_ = nullptr;

				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine();

			if (ImGui::Button("キャンセル", ImVec2(80, 0))) {
				// 親選択状態もリセットしたい場合はクリアする
				selectedChildElemet_ = nullptr;
				ImGui::CloseCurrentPopup();
			}
		}
		else {
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
#endif
}


void UIManager::MoveSelectElement()
{
#ifdef USE_IMGUI

	if (!selectedElemet_)return;

	// 左マウスが離されたら選択中のドラッグ解除
	if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {

		isDragSelectedElement_ = false;

		return;
	}

	// ドラッグ中：ImGuiの前フレームからのマウス移動量（ピクセル）を取得
	ImVec2 mouseDelta = ImGui::GetIO().MouseDelta;
	float radio = Editor::GetInstance()->GetMainViewRatio();

	if (isDragSelectedElement_ && radio > 0.0f) {

		Vector2 parentScale = { 1.0f, 1.0f };

		// 親がいる場合
		if (selectedElemet_->parent_) {
			parentScale = selectedElemet_->parent_->GetScale();
		}

		// マウスの移動量を画面スケールで割って、1280x720のゲーム座標系に変換して加算
		selectedElemet_->SetElapsedDragVelocity(
			Vector2{ 
				(mouseDelta.x / radio) / parentScale.x,
				(mouseDelta.y / radio) / parentScale.y
			});


		// ?

		// 座標も同期
		selectedElemet_->position_ = selectedElemet_->GetScreenTransform().translation_;

	}
#endif
}

/*
	要は今のシーンの中からelement_XXをみてって空き
	番号とする
*/
std::string UIManager::EmptyNameRegister()
{
	int c = 0;

	const std::string base = "element_";


	for (UIData& data : currentSceneUIData_->elementDatas_) {
		for (auto& e : data.elements_) {

			// 名前中から検索
			if (e->name_.find(base) != 0) continue;

			// 末尾の数字を取り出す
			std::string numberStr = e->name_.substr(base.size());

			try {
				int number = std::stoi(numberStr);// 数字に

				// 大きければ設定？
				c = (std::max)(c, number);

			}
			catch (...) {
				// 数字に変換できなければ無視
			}

		}
	}

	// 
	return  base + std::to_string(c + 1);
}




UIElement* UIManager::AddUIElement(std::string canvasLayerName, std::string prefabName)
{
	const auto& layers = hierarchyPanel_->GetHierarchyLayers();

	// 該当レイヤー検索
	auto itt = std::find_if(
		layers.begin(),
		layers.end(),
		[canvasLayerName](const HierarchyLayer& data_) {
			return data_.layerName_ == canvasLayerName;
		}
	);

	// もしレイヤー名がなかったorキャンバスレイヤーではない
	if (itt == layers.end() || itt->layerType_ != HierarchyLayerType::kCanvasLayer) {
		return nullptr;
	}

	// elementData内に同じレイヤー名があるかチェック
	auto it = std::find_if(
		currentSceneUIData_->elementDatas_.begin(),
		currentSceneUIData_->elementDatas_.end(),
		[canvasLayerName](const UIData& data_) {
			return data_.name_ == canvasLayerName;
		}
	);

	int index = 0;
	if (it == currentSceneUIData_->elementDatas_.end()) {
		currentSceneUIData_->elementDatas_.push_back(UIData{ .name_ = canvasLayerName });
		index = static_cast<int>(currentSceneUIData_->elementDatas_.size()) - 1;
	}
	else {
		index = static_cast<int>(std::distance(currentSceneUIData_->elementDatas_.begin(), it));
	}


	/*
		Assetsマネージャーから一致するPrefabあるか検索
	*/
	auto& list = AssetsPanel::GetInstance()->GetHierarchyUIElementResources();

	// 
	auto ittttt = std::find_if(
		list.begin(),
		list.end(),
		[prefabName](const UIElementResource& r) {
			return r.name_ == prefabName;
		}
	);
	
	// みつからなかった
	if (ittttt == list.end()) {
		return nullptr;// null
	}

	/*
		prefabのパスを取得
	*/
	std::string pathId = ittttt->pathId_;


	auto& targetElements = currentSceneUIData_->elementDatas_.at(index).elements_;

	// Elementの一斉読み込み
	if (SceneComponentLoader::GetInstance()->Load(pathId)) {

		json& j = SceneComponentLoader::GetInstance()->GetCurrentJson();
		if (!j.is_array()) return nullptr;

		// 今回新しく生成した要素だけを一時保持するリスト
		std::vector<std::unique_ptr<UIElement>> createdElements;
		std::unordered_map<std::string, std::string> renamedMap; // 旧名 -> 新名（重複回避用）

		for (auto& item : j) {
			UIType type = UIType::kTexture;
			if (item.contains("type")) {
				type = item.at("type").get<UIType>();
			}

			std::unique_ptr<UIElement> element;
			if (type == UIType::kTexture) {
				element = std::make_unique<UIElementTexture>();
			}
			else if (type == UIType::kText) {
				element = std::make_unique<UIElementText>();
			}
			if (!element) continue;

			// 1. JSONから復元
			element->Deserialize(item);

			// 2. 既存要素と名前が被る場合は連番を振って衝突回避
			std::string originalName = element->name_;
			std::string uniqueName = originalName;
			int suffix = 1;
			while (std::any_of(targetElements.begin(), targetElements.end(),
				[&uniqueName](const auto& existing) { return existing->name_ == uniqueName; })) {
				uniqueName = originalName + "_" + std::to_string(suffix++);
			}
			element->name_ = uniqueName;
			renamedMap[originalName] = uniqueName;

			// 3. アセット等の初期化
			element->Initialize();

			// 4. アニメーションが空のときだけデフォルトを入れる（保存済みアニメを消さない）
			if (element->animationDatas_.animations_.empty()) {
				UIElementAnimation newAnime;
				newAnime.animationName = "NewAnime";
				UISequencer::SetDefaultTrackDatas(newAnime);
				element->animationDatas_.animations_["NewAnime"] = newAnime;
				element->animationDatas_.currentTag_ = "NewAnime";
			}

			createdElements.push_back(std::move(element));
		}

		// 5. 今回生成したグループ内で親子関係を復元する
		UIElement* rootElement = nullptr;
		for (auto& e : createdElements) {
			if (!e->parentName_.empty()) {
				// 親の名前もリネームされていたら更新
				if (renamedMap.contains(e->parentName_)) {
					e->parentName_ = renamedMap[e->parentName_];
				}

				auto parentIt = std::find_if(
					createdElements.begin(),
					createdElements.end(),
					[&e](const auto& candidate) { return candidate->name_ == e->parentName_; }
				);

				if (parentIt != createdElements.end()) {
					e->parent_ = parentIt->get();
					e->parent_->childrens_.push_back(e.get());
					e->GetScreenTransform().parent_ = &e->parent_->GetScreenTransform();
				}
			}
			else if (!rootElement) {
				rootElement = e.get(); // ルート（親）を記録
			}
		}

		// 6. ルート要素を選択状態にしてシーケンサーへ渡す
		selectedParentElemet_ = nullptr;
		selectedChildElemet_ = nullptr;
		selectedElemet_ = rootElement;
		if (selectedElemet_) {
			currentSelectAnimationTag = selectedElemet_->animationDatas_.currentTag_;
			currentSceneUIData_->sequencer_.SetUIElementAnimationData(*selectedElemet_);
		}

		// 7. キャンバスのリストへ所有権を移動
		for (auto& e : createdElements) {
			targetElements.push_back(std::move(e));
		}

		// ルートのアドレスを返す
		return rootElement;
	}

	// ここまできたということはload失敗ってこと
	return nullptr;
}

void UIManager::AddUIElement(UIType type,std::string canvasLayerName, std::string pathId)
{

	const auto& layers = hierarchyPanel_->GetHierarchyLayers();

	//該当レイヤー検索
	auto itt = std::find_if(
		layers.begin(),
		layers.end(),
		[canvasLayerName](const HierarchyLayer& data_) {
			return data_.layerName_ == canvasLayerName;
		}
	);

	// なかった
	if (itt == layers.end()) {
		return;
	}

	HierarchyLayerType t = itt->layerType_;

	// キャンバスではない
	if (t != HierarchyLayerType::kCanvasLayer)
		return;


	// elementData内に同じレイヤー名があるかチェック
	auto it = std::find_if(
		currentSceneUIData_->elementDatas_.begin(),
		currentSceneUIData_->elementDatas_.end(),
		[canvasLayerName](const UIData& data_) {
			return data_.name_ == canvasLayerName;
		}

	);

	// 見つからなかった
	int index = 0;

	if (it == currentSceneUIData_->elementDatas_.end()) {

		currentSceneUIData_->elementDatas_.push_back(UIData{
			.name_ = canvasLayerName
			});

		index = static_cast<int>(currentSceneUIData_->elementDatas_.size()) - 1;

	}
	else {

		index = static_cast<int>(
			std::distance(currentSceneUIData_->elementDatas_.begin(), it)
			);
	}

	selectedElemet_ = nullptr;
	selectedParentElemet_ = nullptr;

	/* emplace_back で末尾に直接要素を作成して参照を受け取る */
	std::unique_ptr<UIElement> element;// =

	if (type == UIType::kTexture) {
		element = std::make_unique<UIElementTexture>();
	}
	else if (type == UIType::kText) {
		element = std::make_unique<UIElementText>();
	}

	// 
	element->type_ = type;

	// 名前はデフォルトとする
	element->name_ = EmptyNameRegister();

	element->groupName_ = "Main";

	// 
	element->pathID_ = pathId;


	/*
		ここでアセット読み取り等行う
	*/
	element->Initialize();


	//
	UIElementAnimation newAnime;
	newAnime.animationName = "NewAnime";
	UISequencer::SetDefaultTrackDatas(newAnime);

	// アニメ情報もセットしておく
	element->animationDatas_.animations_["NewAnime"] = newAnime;

	currentSelectAnimationTag = "NewAnime";
	element->animationDatas_.currentTag_ = "NewAnime"; //

	/* 選択中にしとく */
	selectedElemet_ = element.get();

	// シーケンサー側の参照も最新の安全な参照に更新する
	currentSceneUIData_->sequencer_.SetUIElementAnimationData(*selectedElemet_);


	/*
		所有権を移動
	*/
	currentSceneUIData_->elementDatas_.at(index).elements_.push_back(std::move(element));

}


void UIManager::UIEditorTimeLine()
{

#ifdef USE_IMGUI
	ImGui::Begin("UIView");

	// シーケンサー更新
	currentSceneUIData_->sequencer_.Update();

	ImGui::End();
#endif
	

}


// 再帰で「親 → 子 → 孫」の順にリストへ詰める関数
void UIManager::CollectDrawOrder(UIElement* element, std::vector<UIElement*>& outList) {

	outList.push_back(element); // まず自分（親）

	// その直後に自分の子たちを順に詰める（＝親より手前に描画される）
	for (UIElement* child : element->childrens_) {
		CollectDrawOrder(child, outList);
	}
}

// キャンバス全体の描画順リストを取得
std::vector<UIElement*> UIManager::GetOrderedElements(UIData& data) {

	std::vector<UIElement*> ordered;

	for (auto& e : data.elements_) {
		// ルート（親なし）だけを起点にする
		if (e->parent_ == nullptr) {
			CollectDrawOrder(e.get(), ordered);
		}
	}

	return ordered;
}



void UIManager::ElementNamePopupWindow()
{
#ifdef USE_IMGUI

	// ここでpopを呼ぶ
	if (isShowNameWindow_) {

		ImGui::OpenPopup("namePopup");
		isShowNameWindow_ = false;

	}


	// ポップアップ本体
	if (ImGui::BeginPopup("namePopup")) {

		ImGui::Text("名前変更：");
		ImGui::SameLine();


		static std::string inputName;
		ImGui::InputText(" ", &inputName);

		if (ImGui::Button("閉じる")) {
			// 閉じる
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		/* レイヤー操作 */
		if (ImGui::Button("OK")) {


			for (UIData& data : currentSceneUIData_->elementDatas_) {

				// 今開いてるレイヤーはどこかしら♪
				if (data.name_ == Editor::GetInstance()->GetHierarchyPanel().GetSelectedCurrentLayerName()) {

					// 該当エレメント検索
					auto it = std::find_if(
						data.elements_.begin(),
						data.elements_.end(),
						[this](auto& data_) {
							return data_->name_ == inputName;
						}
					);

					// なかった
					if (it == data.elements_.end() && !inputName.empty()) {

						selectedElemet_->name_ = inputName;

						// 閉じる
						ImGui::CloseCurrentPopup();

						break;
					}

				}

			}
			
		}

		ImGui::EndPopup();
	}
#endif
}