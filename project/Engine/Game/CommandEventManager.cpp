#include "CommandEventManager.h"

void CommandEventManager::Update()
{
	if (!isPlaying_) return;// 帰る
	if (!currentEvent_)return;//保険

	bool b = ExecuteCommand(currentEvent_->commands_[currentIndex_]);// 条件

	if (b) {
		currentIndex_++;

		if (currentIndex_ >= currentEvent_->commands_.size()) {
			// 終了
			isPlaying_ = false;
			currentIndex_ = 0;
		}
	}

	triggeredEvents_.clear();// 毎回クリアする
}

void CommandEventManager::PlayEvent(std::string eventName)
{
	/* もしすでにイベント中ならリターン */
	if (isPlaying_)return;

	// あるか
	auto it = std::find_if(
		eventList_.begin(),
		eventList_.end(),
		[eventName](const EventCommandAsset& e) {
			return e.eventName_ == eventName;
		}
	);

	if (it != eventList_.end()) {

		// 再生開始
		isPlaying_ = true;
		currentEvent_ = &(*it);

	}

}

void CommandEventManager::RegisterEvent(EventCommandAsset e)
{
	if (e.eventName_.empty()) return;

	std::string eventName = e.eventName_;

	// あるか
	auto it = std::find_if(
		eventList_.begin(),
		eventList_.end(),
		[eventName](const EventCommandAsset& event) {
			return eventName == event.eventName_;
		}
	);


	// なかったら
	if (it == eventList_.end()) {

		eventList_.push_back(e);

	}

}

void CommandEventManager::NotifyTrigger(const std::string & triggerName)
{
	// 入れる
	triggeredEvents_.insert(triggerName);
}

bool CommandEventManager::ExecuteCommand(EventCommand & cmd)
{
	switch (cmd.type_) {

	// トリガー待ち
	case EventCommandType::kWaitTrigger:

		/*
			もしトリガーリストにあったら(あるかをtrue/false)
		*/
		return triggeredEvents_.contains(cmd.TriggerName_);

		break;

	// 指定エレメント再生
	case EventCommandType::kPlayElementAnimation:

		if (cmd.element_) {

			// もし再生中でなければ
			//if (cmd.element_->animationDatas_.currentTag_.empty()) {
				cmd.element_->PlayAnimation(cmd.elementAnimationName_);
				return true;
			//}
		}

		return false;
		break;

	// 指定エレメント内のテキストエレメントの文字変
	case EventCommandType::kSetElementText:

		if (cmd.element_) {

			// 中の子要素（テキスト）を取得して文字を動的に書き換える
			if (auto* elem = cmd.element_->FindChild(cmd.elementTextName_)) {
				elem->SetText(cmd.elementTextChangeTexts_);//
				return true;
			}
			else {
				return false;
			}

		}
		else {
			return false;
		}

		break;
	}



	return false;
}
