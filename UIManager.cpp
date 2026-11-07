#include "UIManager.h"

#include "ImGuiManager.h"
#include "Editor.h"

#include "Input.h"

#include "Timer.h"

#include "SceneComponentLoader.h"

#include <imgui/IconsFontAwesome.h>

#include "AssetsPanel.h"


std::vector<UIData> UIManager::elementDatas_;


UIManager* UIManager::GetInstance()
{
	static UIManager instance;
	return &instance;
}

void UIManager::Initialize()
{
	/*
		ElementData読み込み
	*/
	LoadElementData();


	/*sequencer_.mFrameMin = -100;
	sequencer_.mFrameMax = 1000;*/
	//sequencer_.items_.push_back(UISequencer::SequenceItem{ 0, 10, 30 });
	//sequencer_.items_.push_back(UISequencer::SequenceItem{ 1, 20, 30 });
	//sequencer_.items_.push_back(UISequencer::SequenceItem{ 2, 12, 60 });
	//sequencer_.items_.push_back(UISequencer::SequenceItem{ 1, 61, 90 });
	//sequencer_.items_.push_back(UISequencer::SequenceItem{ 2, 90, 99 });

}

void UIManager::LoadElementData()
{
	jsonPath = "resources/Scene/uiSample.json";

	// 読込み
	if (SceneComponentLoader::GetInstance()->Load(jsonPath)) {

		SceneComponentLoader* loder = SceneComponentLoader::GetInstance();

		//
		loder->BeginGroup("UI");

		// UIData一覧
		loder->Property("data", elementDatas_);


		loder->EndGroup();

		// 読み込み
		loder->LoadGroup("UI");
	}

	int count = 0;
	for (UIData& data : elementDatas_) {
		for (UIElement& e : data.elements_) {

			// 名前がない場合付ける
			if (e.name_.empty()) {
				e.name_ = "element_" + std::to_string(count);
				count++;//
			}

			/*
				Spriteの一斉読み込み
			*/

			// AssetPanelからテキスチャを貰う
			e.sprite_.Initialize(assetPanel_->GethierarchyTextureResourceList()[e.pathID_]);
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
	sequencer_.AnimationComboView();


	ImGui::End();

#endif

	//　ElementDataの更新
	for (UIData& data : elementDatas_) {

		//
		deleteElementIndex_ = -1;

		for (int i = 0; i < data.elements_.size(); i++)
		{
			UIElement& e = data.elements_.at(i);

			/*
				再生中のアニメの更新
			*/
			UpdateElementAnimation(e.animationDatas_);


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
			JudgeMouseSelectElement(e,i);

#endif

			

		}

		// もし削除対象があったら
		if (deleteElementIndex_ != -1) {
			data.elements_.erase(data.elements_.begin() + deleteElementIndex_);
		}
	}

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
	for (UIData& data : elementDatas_) {

		if (!data.isView_)continue;

		// 描画
		for (UIElement& e : data.elements_) {
			e.sprite_.Draw();
		}

	}

}

void UIManager::UpdateElementAnimation(UIElementAnimationDatas& animeDatas)
{
	// 現在設定されている、タグのアニメ
	auto& anime_ = animeDatas.animations_[animeDatas.currentTag_];

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
					&animeDatas,
					animeDatas.animations_[animeDatas.currentTag_].frame,// 現在の経過時間。
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
		elementDatas_.begin(),
		elementDatas_.end(),
		[canvasName](const UIData& data_) {
			return data_.name_ == canvasName;
		}
	);
	if (it != elementDatas_.end()) {

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


	if (selectedElemet_) {

		//
		ImGui::AlignTextToFramePadding();

		ImGui::Text("エレメント名：%s", selectedElemet_->name_.c_str());


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
				sequencer_.AddKeyFrame(SequenceItemType::kPosition, KeyFrame<Vector3>{
					sequencer_.GetCurrentFrame(),
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
				sequencer_.AddKeyFrame(SequenceItemType::kScale, KeyFrame<Vector3>{
					sequencer_.GetCurrentFrame(),
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
				sequencer_.AddKeyFrame(SequenceItemType::kRotation, KeyFrame<Vector3>{
					sequencer_.GetCurrentFrame(),
						rot,
						EasingType::None
				});
			}
		}
		ImGui::PopID();

		ImGui::Dummy(ImVec2{ 0,4.0f });
		ImGui::SeparatorText("Animations");
		ImGui::Dummy(ImVec2{ 0,4.0f });

		

	}

#endif

}

void UIManager::JudgeMouseSelectElement(UIElement& e,int index)
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

		if (inside) {

			// 対象を選択
			selectedElemet_ = &e; 
			isDragSelectedElement_ = true; // ドラッグ中

			/*
				新たに選択されたので、アニメーションのデータを受け渡す
			*/
			sequencer_.SetUIElementAnimationData(selectedElemet_->animationDatas_);

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
		elementDatas_.begin(),
		elementDatas_.end(),
		[canvasLayerName](const UIData& data_) {
			return data_.name_ == canvasLayerName;
		}

	);

	// 見つからなかった
	int index = 0;

	if (it == elementDatas_.end()) {

		elementDatas_.push_back(UIData{
			.name_ = canvasLayerName
			});

		index = static_cast<int>(elementDatas_.size()) - 1;

	}
	else {

		index = static_cast<int>(
			std::distance(elementDatas_.begin(), it)
			);
	}

	/* emplace_back で末尾に直接要素を作成して参照を受け取る */
	UIElement& element = elementDatas_.at(index).elements_.emplace_back();

	// 
	element.pathID_ = pathId;

	/* Assetパネルからテクスチャのリソースを受けとり、スプライトを初期化 */
	element.sprite_.Initialize(assetPanel_->GethierarchyTextureResourceList()[pathId]);
	element.sprite_.SetAnchor(Sprite::anchor::Center);

	// とりあえずスケール初期化？
	element.scale_ = { 1.0f,1.0f,1.0f };
}


void UIManager::UIEditorTimeLine()
{

#ifdef USE_IMGUI
	ImGui::Begin("UIView");

	// シーケンサー更新
	sequencer_.Update();

	ImGui::End();
#endif
	

}