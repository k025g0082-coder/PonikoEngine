#include "UIManager.h"

#include "ImGuiManager.h"
#include "Editor.h"

#include "Input.h"
#include "SceneComponentLoader.h"

#include "AssetsPanel.h"

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

	/* Spriteの一斉読み込み */
	for (UIData& data : elementDatas_) {
		for (UIElement& e : data.elements_) {

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

	//　ElementData内の表示
	for (UIData& data : elementDatas_) {

		ImGui::Text(data.name_.c_str());

		//
		for (UIElement& e : data.elements_) {

			std::string text = "   " + e.pathID_;

			ImGui::Text(text.c_str());
		}
	}


	ImGui::End();

#endif

	//　ElementDataの更新
	for (UIData& data : elementDatas_) {

	
		//
		deleteElementIndex_ = -1;

		for (int i = 0; i < data.elements_.size(); i++)
		{
			UIElement& e = data.elements_.at(i);

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

			// 当たったか
			//ImGui::Text(" inside: %s", inside ? "true":"false" );

#endif

			

		}

		// もし削除対象があったら
		if (deleteElementIndex_ != -1) {
			data.elements_.erase(data.elements_.begin() + deleteElementIndex_);
		}
	}

#ifdef USE_IMGUI

	if (selectedElemet_) {

		/*
			選択されているElementを移動
		*/
		MoveSelectElement();

	}
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
			selectedElemet_ = &e; // 対象を選択
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

	// 左マウスが離されたら選択解除
	if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
		selectedElemet_ = nullptr;
		return;
	}

	// ドラッグ中：ImGuiの前フレームからのマウス移動量（ピクセル）を取得
	ImVec2 mouseDelta = ImGui::GetIO().MouseDelta;
	float radio = Editor::GetInstance()->GetMainViewRatio();

	if (radio > 0.0f) {
		// マウスの移動量を画面スケールで割って、1280x720のゲーム座標系に変換して加算
		selectedElemet_->sprite_.GetScreenTransform().translation_.x += mouseDelta.x / radio;
		selectedElemet_->sprite_.GetScreenTransform().translation_.y += mouseDelta.y / radio;
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

}



