#pragma once
#include "EventCommand.h"

#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>

/*
	シーン別で持たせる、コマンド系のイベントのマネージャー
*/
class CommandEventManager
{
public:

	void Update();


	void PlayEvent(std::string eventName);

	void RegisterEvent(EventCommandAsset e);

	void NotifyTrigger(const std::string& triggerName);


	// イベントリストの参照
	std::vector<EventCommandAsset>& GetEventList() { return eventList_; }

	void Reset() {
		if (!currentEvent_)return;
		
		// コマンドに関連しているUIElement全てのを、アニメーションリセット
		for (auto& cmd : currentEvent_->commands_) {
			if (cmd.element_ == nullptr) continue;

			cmd.element_->ResetAnimation();

		}

		isPlaying_ = false;
		currentEvent_ = nullptr;
	}

private:
	// イベントの終了条件を見る
	bool ExecuteCommand(EventCommand& cmd);

	EventCommandAsset* currentEvent_;// 現在実行中のイベント

	// 飛んできた通知の一時補完用
	std::unordered_set<std::string> triggeredEvents_;


	// イベントリスト
	std::vector<EventCommandAsset> eventList_;


	uint32_t currentIndex_;// イベント何行目か
	bool isPlaying_;
};

