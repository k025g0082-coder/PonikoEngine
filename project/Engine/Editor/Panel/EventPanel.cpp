#include "EventPanel.h"
#include "ImGuiManager.h"

#include "UIManager.h"

#include "SceneComponentLoader.h"


#include <nlohmann/json.hpp>
#include <misc/cpp/imgui_stdlib.h>
#include <string>
#include <vector>

#include <memory>

#include <utility>
#include <optional>

#include <fstream>


EventPanel* EventPanel::GetInstance()
{
    static EventPanel instance;
    return &instance;
}


void EventPanel::Initialize()
{
    /*
		コマンドイベントの、タイプごとのUI描画を登録
	*/ 

	commandEventDrawers_[EventCommandType::kWaitTrigger] = [this](EventCommand& cmd) {
		DrawInputRow("TriggerName", cmd.TriggerName_);
	};
	commandEventDrawers_[EventCommandType::kPlayElementAnimation] = [this](EventCommand& cmd) {
		DrawUIElementDropRow(cmd);
		DrawInputRow("Animation Name", cmd.elementAnimationName_);
	};
	commandEventDrawers_[EventCommandType::kSetElementText] = [this](EventCommand& cmd) {
		DrawUIElementDropRow(cmd);
		DrawInputRow("TextUIElement Name", cmd.elementTextName_);
		DrawInputRow("Text", cmd.elementTextChangeTexts_);
	};

}


/*
* 
	メインのImGuiパネル部分
*
*/
void EventPanel::PanelView()
{
#ifdef USE_IMGUI

    ImGui::Begin("EventPanel");

   
	if (!currentSceneName_.empty()) {
		
		// コマンド系
		ShowCommandEventView();

		// ...
	}


	// save
	if (ImGui::Button("save")) {
		Save();
	}

	ImGui::SameLine();

	if (selectedCommandEvent_) {
		if (ImGui::Button("テスト実行")) {

			//sceneEvents_[currentSceneName_].commandEventManager.Reset();
			sceneEvents_[currentSceneName_].commandEventManager.PlayEvent(selectedCommandEvent_->eventName_);

		}
	}

	// 改名window
	EventNamePopupWindow();


    ImGui::End();

#endif
}

void EventPanel::Update()
{
	// メインパネル表示
	PanelView();


	if (!currentSceneName_.empty()) {
		
		// コマンド系の場合の更新
		sceneEvents_[currentSceneName_].commandEventManager.Update();

		//...
	}
}

void EventPanel::SwitchScene(std::string sceneName)
{
    if (sceneName.empty()) return;

	/*
		現在のSceneNameに設定
	*/ 
	currentSceneName_ = sceneName;

   // 既にあるか
    if (sceneEvents_.contains(sceneName)) {

		// あった場合切り替え。
		// ・・・すでにcurrentSceneNameに設定しているので専用の処理は要らないか

    }
	else {
		/*
			なかったばあい新規読み込み＆登録
		*/
		Load();
	}


	/*
	　  シーン読み込み前 - uiPropertyに配置されたUI配置用
	 
		1. キャンバス内の子無しUIElementを全て取得
		2. jsonにある一致した名前のがあれば、コマンドイベントのアドレスに格納する（実行時に必要な為）
	*/
	auto uiDatas = UIManager::GetInstance()->GetCurrentSceneUIData();

	// 現在のシーン名のCommandイベントリスト
	auto& list = sceneEvents_[currentSceneName_].commandEventManager.GetEventList();


	// 全キャンバス
	for (auto& uiData : *uiDatas) {

		// 全エレメント
		for (auto& element : uiData.elements_) {

			/*
				他Elementの子であればスキップ
			*/
			if (element->parent_) continue;


			//　現在のシーン名のCommandイベントリストから、全イベントを詮索
			for (auto& event : list) {
				
				// 全コマンド
				for (auto& cmd : event.commands_) {

					// エレメント名が一致
					if (cmd.elementRootName_ == element->name_) {

						// ポインタを設定!!!!！！
						cmd.element_ = element.get();
					}
				}
			}

		}
	}
}

void EventPanel::NotifyTrigger(std::string triggerTag)
{
	if (currentSceneName_.empty()) return;

	// コマンドイベント用
	sceneEvents_[currentSceneName_].commandEventManager.NotifyTrigger(triggerTag);

}

void EventPanel::RegionUIElementAddress(UIElement* element)
{
	

	/*
	　　シーン読み込み後 - ゲームのシーン内で個別でAddElementされたの用

	    1. キャンバス内の子無しUIElementを全て取得
		2. jsonにある一致した名前のがあれば、コマンドイベントのアドレスに格納する（実行時に必要な為）
	*/

	// 現在のシーン名のCommandイベントリスト
	auto& list = sceneEvents_[currentSceneName_].commandEventManager.GetEventList();

	/*
		他Elementの子であれば
	*/
	if (element->parent_) return;


	//　現在のシーン名のCommandイベントリストから、全イベントを詮索
	for (auto& event : list) {
				
		// 全コマンド
		for (auto& cmd : event.commands_) {

			// エレメント名が一致
			if (cmd.elementRootName_ == element->name_) {

				// ポインタを設定!!!!！！
				cmd.element_ = element;
			}
		}
	}

}

void EventPanel::Load()
{
	if (currentSceneName_.empty()) return;

	// 新規読み込み。jsonから取得
	const std::string jsonPath_ = "resources/Scene/" + currentSceneName_ + "/events.json";


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

		loder->BeginGroup("Events");

		/*
			シーンにあったコマンドイベントリストを読み込む
		*/
		loder->Property("CommandEvents", sceneEvents_[currentSceneName_].commandEventManager.GetEventList());



		loder->EndGroup();
		loder->LoadGroup("Events");// 読込

	}

}

/*
	コマンドイベントのビュー
*/
void EventPanel::ShowCommandEventView()
{
#ifdef USE_IMGUI

	/*
		左パネル - 名前一覧表示など
	*/

	// ウィンドウ内の残り高さを取得（下の全セーブボタン用に一ボタン分開けた）
	float availHeight = ImGui::GetContentRegionAvail().y - 60.0f;

	// --- 左ペイン（幅 140px 固定
	ImGui::BeginChild("LeftPane", ImVec2(140.0f, availHeight), true);
	{
		ShowCommandEventLeftView();
	}
	ImGui::EndChild();

	//
	ImGui::SameLine();

	// --- 右ペイン（幅 0.0f にすると「残りの幅すべて」になる）
	ImGui::BeginChild("RightPane", ImVec2(0.0f, availHeight), true);
	{
		ShowCommandEventRightView();
	}
	ImGui::EndChild();

	
#endif
}

// 左ビュー
void EventPanel::ShowCommandEventLeftView()
{
#ifdef USE_IMGUI

	auto& events = sceneEvents_[currentSceneName_];

	// スクロールしない下部のYを差し引いた高さに設定
	float listHeight = ImGui::GetContentRegionAvail().y - 60.0f;

	ImGui::BeginChild("EventListScroll", ImVec2(0.0f, listHeight), false);
	{

		// コマンドイベント一覧表示
		for (auto& e : events.commandEventManager.GetEventList()) {

			// 今選択されているか(ポインタの比較)
			bool isSelected = (selectedCommandEvent_ == &e);

			// 1. 左クリックで選択切り替え
			if (ImGui::Selectable(e.eventName_.c_str(), isSelected)) {
				selectedCommandEvent_ = &e;
			}

			// 2. ダブルクリック判定（例：名前編集モードに入る、または即テスト再生）
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {

				// ダブルクリックされた時の処理(名前変更ビューなど？)

			}

			// 3. 右クリックメニュー（削除や複製など）
			if (ImGui::BeginPopupContextItem()) { // 直前のアイテム（Selectable）の右クリックを自動検知

				if (ImGui::MenuItem("名前変更")) {
					isShowNameWindow_ = true;
				}
				if (ImGui::MenuItem("Duplicate")) {
					// 複製処理
				}
				if (ImGui::MenuItem("Delete")) {
					// 削除処理（ループ後に行う）
				}
				ImGui::EndPopup();
			}
		}

	}
	ImGui::EndChild();


	/* こっからはスクロールしない */


	/*
		新規追加
	*/
	if (ImGui::Button("New")) {

		EventCommandAsset e;
		e.eventName_ = "NewEvent";

		events.commandEventManager.RegisterEvent(e);

	}
#endif
}

// 右
void EventPanel::ShowCommandEventRightView()
{
	if (selectedCommandEvent_ == nullptr) return;

#ifdef USE_IMGUI

	// スクロールしない下部のYを差し引いた高さに設定
	float listHeight = ImGui::GetContentRegionAvail().y - 30.0f;

	int swapIndexA  = -1;
	int swapIndexB  = -1;
	int deleteIndex = -1;

	ImGui::BeginChild("EventListScroll", ImVec2(0.0f, listHeight), false);
	{
		int i = 0;
		/*
			すべてのコマンドリストを回す
		*/
		for (auto& cmd : selectedCommandEvent_->commands_)
		{

			ImGui::PushID(i);

			/*
				1. タイプのComboBox
			*/
				
			int currentType = static_cast<int>(cmd.type_);

			// 連番の表示
			ImGui::Text((std::to_string(i) + ".").c_str());
				
			ImGui::SameLine();

			ImGui::SetNextItemWidth(200.0f);
			if (ImGui::Combo("##Type", &currentType, kEventCommandTypeNames, IM_ARRAYSIZE(kEventCommandTypeNames))) {
				cmd.type_ = static_cast<EventCommandType>(currentType);
			}


			/*
				2. Up,Down系のボタン
			*/
			float buttonsWidth = 145.0f; // ボタン3つ分の合計幅の目安
			ImGui::SameLine(ImGui::GetContentRegionMax().x - buttonsWidth);

			if (ImGui::Button("Up", ImVec2(35.0f, 0.0f)) && i > 0) {
				swapIndexA = i;
				swapIndexB = i - 1;
			}
			ImGui::SameLine();
			if (ImGui::Button("Down", ImVec2(45.0f, 0.0f)) && i < selectedCommandEvent_->commands_.size() - 1) {
				swapIndexA = i;
				swapIndexB = i + 1;
			}
			ImGui::SameLine();
			if (ImGui::Button("Delete", ImVec2(55.0f, 0.0f))) {
				deleteIndex = i;
			}


			/*
				3. タイプ別の異なる入力欄
			*/
			// 表の開始（ID名, 列数 = 2列, オプション = 枠線なし）
			if (ImGui::BeginTable("ParamTable", 2, ImGuiTableFlags_None)) {

				// 1列目（Label）: 幅を 160px に固定する
				ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 160.0f);

				// 2列目（Input）: ウィンドウの残り幅いっぱいに自動で伸縮させる
				ImGui::TableSetupColumn("Input", ImGuiTableColumnFlags_WidthStretch);

				// 実際の「行（Row）」と「セル（Column）」を追加する処理
				// （DrawInputRow の中で TableNextRow() と TableSetColumnIndex() を呼んでいる！）
				if (auto it = commandEventDrawers_.find(cmd.type_); it != commandEventDrawers_.end()) {

					it->second(cmd);// 表示

				}

				// 4. 表の終了（BeginTable が true を返したときだけ必ず呼ぶ）
				ImGui::EndTable();
			}

			ImGui::PopID();

				
			ImGui::Dummy(ImVec2{ 0,4.0f });
			ImGui::Separator();
			ImGui::Dummy(ImVec2{ 0,4.0f });

			i++;
		}

	}
	ImGui::EndChild();

	/*
		もし入れ替えとかしてた場合入れ替える
	*/
	if (swapIndexA != -1 && swapIndexB != -1) {

		std::swap(
			selectedCommandEvent_->commands_[swapIndexA],
			selectedCommandEvent_->commands_[swapIndexB]
		);

	}

	// 削除
	if (deleteIndex != -1) {
		selectedCommandEvent_->commands_.erase(
			selectedCommandEvent_->commands_.begin() + deleteIndex
		);
	}


	/* こっからはスクロールしない */


	/*
		新規追加
	*/
	if (ImGui::Button("New")) {

		EventCommand cmd;
		
		selectedCommandEvent_->commands_.push_back(cmd);

	}
#endif
}


void EventPanel::Save()
{
	if (currentSceneName_.empty()) return;

	// セーブ時のパス
	const std::string jsonPath_ = "resources/Scene/" + currentSceneName_ + "/events.json";


	auto loder = SceneComponentLoader::GetInstance();

	//　切り替え
	if (loder->SaveSwitchJson(jsonPath_)) {

		loder->SaveGroup("Events");

		SceneComponentLoader::GetInstance()->Save(jsonPath_);
	}

}

void EventPanel::DrawInputRow(const char* label, std::string& targetStr)
{
#ifdef USE_IMGUI

	/* 一つ下の段へ */
	ImGui::TableNextRow();

	// 左の段へ - Label
	ImGui::TableSetColumnIndex(0);
	ImGui::AlignTextToFramePadding();
	ImGui::Indent(22.0f);
	ImGui::Text("%s", label);
	ImGui::Unindent(22.0f);

	// 右の段へ - Input欄
	ImGui::TableSetColumnIndex(1);
	ImGui::SetNextItemWidth(-1.0f);
	char buf[256];
	strcpy_s(buf, targetStr.c_str());
	std::string hiddenId = std::string("##") + label;
	if (ImGui::InputText(hiddenId.c_str(), buf, sizeof(buf))) {
		targetStr = buf;
	}
#endif
}

void EventPanel::DrawUIElementDropRow(EventCommand& cmd)
{
#ifdef USE_IMGUI

	/* 一つ下の段へ */
	ImGui::TableNextRow();

	// 左の段へ - Label
	ImGui::TableSetColumnIndex(0);
	ImGui::AlignTextToFramePadding();
	ImGui::Indent(22.0f);
	ImGui::Text("UIElement名：");
	ImGui::Unindent(22.0f);

	// 右の段へ - Input欄
	ImGui::TableSetColumnIndex(1);
	ImGui::SetNextItemWidth(-1.0f);
	

	// == 名前表示
	if (cmd.element_ == nullptr) {
		ImGui::Selectable("- ここにドロップ -");
	}
	else {

		// 登録されたエレメントの名前表示
		ImGui::Selectable(cmd.element_->name_.c_str());
	}

	/*
		ドロップされるターゲット
	*/
	if (ImGui::BeginDragDropTarget())
	{
		// 送り側と同じタグ名を指定
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("UI_ELEMENT_PTR"))
		{
			// サイズが一致しているか安全確認
			IM_ASSERT(payload->DataSize == sizeof(UIElement*));

			// void* から UIElement** にキャストして中身（UIElement*）を取り出す
			UIElement* droppedElement = *static_cast<UIElement* const*>(payload->Data);

			if (droppedElement) {

				/*
					アドレスを設定！！！
				*/
				cmd.element_ = droppedElement;
				cmd.elementRootName_ = droppedElement->name_;//
			}
		}
		ImGui::EndDragDropTarget();
	}

#endif
}


void EventPanel::EventNamePopupWindow()
{
	if (currentSceneName_.empty())return;
	if (selectedCommandEvent_ == nullptr) return;

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

			auto& list = sceneEvents_[currentSceneName_].commandEventManager.GetEventList();

			// 該当イベント名検索
			auto it = std::find_if(
				list.begin(),
				list.end(),
				[this](const EventCommandAsset& e) {
					return e.eventName_ == inputName;
				}
			);

			// なかった
			if (it == list.end() && !inputName.empty()) {

				// 改名処理
				selectedCommandEvent_->eventName_ = inputName;

				// 閉じる
				ImGui::CloseCurrentPopup();

			}

		}

		ImGui::EndPopup();
	}
#endif
}