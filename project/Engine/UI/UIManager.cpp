#include "UIManager.h"

#include "ImGuiManager.h"
#include "Editor.h"

#include "Input.h"

#include "Timer.h"

#include "SceneComponentLoader.h"

#include <imgui/IconsFontAwesome.h>

#include "AssetsPanel.h"

SceneUIData* UIManager::currentSceneUIData_;


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
		for (UIElement& e : data.elements_) {

			// 名前がない場合付ける
			if (e.name_.empty()) {

				e.name_ = "element_" + std::to_string( currentSceneUIData_->nameRegisterCount_ );

				currentSceneUIData_->nameRegisterCount_++;//
			}

			/*
				Spriteの一斉読み込み
			*/

			// AssetPanelからテキスチャを貰う
			e.sprite_.Initialize(AssetsPanel::GetInstance()->GethierarchyTextureResourceList()[e.pathID_]);
			e.sprite_.SetAnchor(Sprite::anchor::Center);

			// SRT
			e.sprite_.GetScreenTransform().translation_ = e.position_;
			e.sprite_.GetScreenTransform().scale_ = e.scale_;
			e.sprite_.GetScreenTransform().rotation_ = e.rotation_;
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
	
	//
	SelectedElementAnimationView();

	//
	currentSceneUIData_->sequencer_.AnimationComboView();


	ImGui::End();

#endif

	// 一度でも各エレメントの矩形内でクリックされたか
	bool anyInside = false;

	//　ElementDataの更新
	for (UIData& data : currentSceneUIData_->elementDatas_) {

		//
		deleteElementIndex_ = -1;

		for (int i = 0; i < data.elements_.size(); i++)
		{
			UIElement& e = data.elements_.at(i);

			/*
				再生中のアニメの更新
			*/
			UpdateElementAnimation(&e);


			// もし現在開いてるレイヤー以外のエレメントは、選択できないようにする
			if (data.name_ != hierarchyPanel_->GetSelectedCurrentLayerName()) {
				e.sprite_.UpdateScreenTransform();// 行列だけ更新
				continue;
			}

			// スプライトの座標更新
			e.sprite_.UpdateScreenTransform();

#ifdef USE_IMGUI

			/*
				マウス座標と大きさをもとにElementを選択したり削除したり
			*/
			JudgeMouseSelectElement(e,i,anyInside);

#endif

			

		}

		// もし削除対象があったら
		if (deleteElementIndex_ != -1) {
			data.elements_.erase(data.elements_.begin() + deleteElementIndex_);
		}
	}
	
	/*
		もし一度も各エレメントの矩形内でクリックされていない場合リセット
	*/
	if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !anyInside) {

		// モーダルが開いていない時のみリセット
		if (!ImGui::IsPopupOpen("ParentElementPopup")) {
			selectedElemet_ = nullptr;
			selectedParentElemet_ = nullptr;
			selectedChildElemet_ = nullptr;
		}
	}


	//
	ParentSelectPopupWindow();


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

		// 描画
		for (UIElement& e : data.elements_) {
			e.sprite_.Draw();
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
			[elementName](const UIElement& data_) {
				return data_.name_ == elementName;
			}
		);

		if (itt != it->elements_.end()) {

			// 現在のアニメを指定
			itt->animationDatas_.currentTag_ = elementAnimationTag;

			// 再生時間リセット
			itt->animationDatas_.animations_[elementAnimationTag].frame = 0.0f;

			// 再生開始
			itt->animationDatas_.animations_[elementAnimationTag].isPlaying = true;

		}
	}

}

void UIManager::SelectedElementAnimationView()
{
#ifdef USE_IMGUI

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

		if (selectedElemet_->parent_) {
			ImGui::Text("親：%s", selectedElemet_->parent_->name_.c_str());
		}
		else {
			ImGui::Text("親：なし");
		}

		ImGui::SeparatorText("BaseTransform");
		ImGui::Dummy(ImVec2{ 0,4.0f });

		if (ImGui::DragFloat3("座標", &selectedElemet_->sprite_.GetScreenTransform().translation_.x, 0.1f)) {
			selectedElemet_->position_ = selectedElemet_->sprite_.GetScreenTransform().translation_;// ベース座標も同期
		}


		ImGui::SameLine(ImGui::GetWindowWidth() - 50.0f);

		ImGui::PushID("b1");
		if (ImGui::Button((char*)ICON_FA_KEY)) {

			//
			if (selectedElemet_) {

				//座標
				Vector3 vec = selectedElemet_->sprite_.GetScreenTransform().translation_;

				// キーフレーム追加
				currentSceneUIData_->sequencer_.AddKeyFrame(SequenceItemType::kPosition, KeyFrame<Vector3>{
					currentSceneUIData_->sequencer_.GetCurrentFrame(),
						vec,
						EasingType::None
				});
			}

		}
		ImGui::PopID();


		if (ImGui::DragFloat3("拡大", &selectedElemet_->sprite_.GetScreenTransform().scale_.x, 0.1f)) {
			selectedElemet_->scale_ = selectedElemet_->sprite_.GetScreenTransform().scale_;// ベース大きさも同期
		}

		ImGui::SameLine(ImGui::GetWindowWidth() - 50.0f);

		ImGui::PushID("b2");
		if (ImGui::Button((char*)ICON_FA_KEY)) {

			if (selectedElemet_) {

				//　スケール
				Vector3	scale = selectedElemet_->sprite_.GetScreenTransform().scale_;

				// キーフレーム追加
				currentSceneUIData_->sequencer_.AddKeyFrame(SequenceItemType::kScale, KeyFrame<Vector3>{
					currentSceneUIData_->sequencer_.GetCurrentFrame(),
						scale,
						EasingType::None
				});
			}
		}
		ImGui::PopID();


		if (ImGui::DragFloat3("回転", &selectedElemet_->sprite_.GetScreenTransform().rotation_.x, 0.1f)) {
			selectedElemet_->rotation_ = selectedElemet_->sprite_.GetScreenTransform().rotation_;// ベース回転も同期
		}


		ImGui::SameLine(ImGui::GetWindowWidth() - 50.0f);

		ImGui::PushID("b3");
		if (ImGui::Button((char*)ICON_FA_KEY)) {
			//
			if (selectedElemet_) {

				//回転
				Vector3 rot = selectedElemet_->sprite_.GetScreenTransform().rotation_;

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
			キー補間タイプ
		*/
		static int type = (int)EasingType::None;

		if (ImGui::BeginCombo("Easing", Easing::GetEasingTag()[type]))
		{
			for (int i = 0; i < IM_ARRAYSIZE(Easing::GetEasingTag()); i++)
			{
				bool isSelected = (type == i);

				if (ImGui::Selectable(Easing::GetEasingTag()[i], isSelected))
				{
					type = i;
				}

				if (isSelected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}

			ImGui::EndCombo();
		}
	}

#endif

}

void UIManager::JudgeMouseSelectElement(UIElement& e,int index,bool& anyInside)
{
	// 現在のEditorのMainViewの比率を得る
	float radio = Editor::GetInstance()->GetMainViewRatio();

	// 比率掛けた座標
	Vector2 position = {
		.x = e.sprite_.GetScreenTransform().translation_.x * radio,
		.y = e.sprite_.GetScreenTransform().translation_.y * radio
	};

	// 比率掛けた大きさ
	Vector2 size = {
		.x = static_cast<float>(e.sprite_.GetImageWidth()) * radio,
		.y = static_cast<float>(e.sprite_.GetImageHeight()) * radio,
	};


	Vector2 min = {
		position.x - size.x / 2.0f,
		position.y - size.y / 2.0f
	};

	Vector2 max = {
		position.x + size.x / 2.0f,
		position.y + size.y / 2.0f
	};

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

}

void UIManager::ParentSelectPopupWindow()
{
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

				/* 親設定 */
				selectedChildElemet_->parent_ = selectedParentElemet_;

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
}


void UIManager::MoveSelectElement()
{
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

		// マウスの移動量を画面スケールで割って、1280x720のゲーム座標系に変換して加算
		selectedElemet_->sprite_.GetScreenTransform().translation_.x += mouseDelta.x / radio;
		selectedElemet_->sprite_.GetScreenTransform().translation_.y += mouseDelta.y / radio;

		// 座標も同期
		selectedElemet_->position_ = selectedElemet_->sprite_.GetScreenTransform().translation_;

	}
}




void UIManager::AddUIElement(std::string canvasLayerName, std::string pathId)
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


	if (itt == layers.end()) {
		return;
	}

	HierarchyLayerType type = itt->layerType_;

	if (type != HierarchyLayerType::kCanvasLayer)
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
	UIElement& element = currentSceneUIData_->elementDatas_.at(index).elements_.emplace_back();


	// 名前はデフォルトとする
	element.name_ = "element_" + std::to_string(currentSceneUIData_->nameRegisterCount_);

	element.groupName_ = "Main";

	// 
	element.pathID_ = pathId;

	/* Assetパネルからテクスチャのリソースを受けとり、スプライトを初期化 */
	element.sprite_.Initialize(AssetsPanel::GetInstance()->GethierarchyTextureResourceList()[pathId]);
	element.sprite_.SetAnchor(Sprite::anchor::Center);

	// とりあえずスケール初期化？
	element.scale_ = { 1.0f,1.0f,1.0f };

	//
	UIElementAnimation newAnime;
	newAnime.animationName = "NewAnime";
	UISequencer::SetDefaultTrackDatas(newAnime);

	// アニメ情報もセットしておく
	element.animationDatas_.animations_["NewAnime"] = newAnime;

	currentSelectAnimationTag = "NewAnime";
	element.animationDatas_.currentTag_ = "NewAnime"; //

	/* 選択中にしとく */
	selectedElemet_ = &element;

	// シーケンサー側の参照も最新の安全な参照に更新する
	currentSceneUIData_->sequencer_.SetUIElementAnimationData(*selectedElemet_);
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