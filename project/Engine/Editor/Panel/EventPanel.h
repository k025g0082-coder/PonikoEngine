#pragma once
#include "CommandEventManager.h"

#include <unordered_map>

/*
	イベントを設定出来たりするパネル（の予定）
*/

struct EventManagers {

	CommandEventManager commandEventManager;

};

class EventPanel {
public:

	// ｼﾝｸﾞﾙﾄﾝ
	static EventPanel* GetInstance();

	// コピー禁止
	EventPanel() = default;
	~EventPanel() = default;
	EventPanel(EventPanel&) = delete;
	EventPanel& operator=(EventPanel&) = delete;

	void Initialize();
	void PanelView();

	void Update();


	/// <summary>
	/// シーン切り替え時に実行
	/// </summary>
	/// <param name="sceneName">シーン名</param>
	void SwitchScene(std::string sceneName);


	/// <summary>
	/// イベント用のトリガーを発行する
	/// </summary>
	/// <param name="triggerTag"></param>
	void NotifyTrigger(std::string triggerTag);

	// 新しく追加されたりしたUIElement、イベントのアドレス参照用のにセットする
	void RegionUIElementAddress(UIElement* element);

private:

	/*
		コマンドイベント
	*/
	void ShowCommandEventView();

	void ShowCommandEventLeftView();
	void ShowCommandEventRightView();
	EventCommandAsset* selectedCommandEvent_ = nullptr;

	// コマンドタイプー＞必要な入力欄のラムダ式          ↓　【重要】引数！！
	std::unordered_map< EventCommandType, std::function<void (EventCommand&)> > commandEventDrawers_;


	void Save();
	void Load();


	std::string currentSceneName_;

	/*
		シーン名ー＞イベントデータ
	*/
	std::unordered_map< std::string, EventManagers > sceneEvents_;

	//
	bool isShowNameWindow_ = false;
	void EventNamePopupWindow();


	// 共通ヘルパー：テーブルの1行（左ラベル・右InputText）を描く
	void DrawInputRow(const char* label, std::string& targetStr);

	// 共通ヘルパー：UIElementのアドレスの、ドロップ欄
	void DrawUIElementDropRow(EventCommand& cmd);
};

