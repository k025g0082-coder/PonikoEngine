#pragma once
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "UIElement.h"

/*
	一行単位ですすんでいくコマンドタイプのイベント
*/

// イベント一行分のEventタイプ
enum class EventCommandType {

	kWaitTrigger,		    // トリガー待ち

	kPlayElementAnimation,  // 指定エレメント名のアニメを再生
	kSetElementText,		// 指定エレメントのテキストを変更する

	Count
};

inline constexpr const char* kEventCommandTypeNames[] =
{ 
	"トリガー待ち",
	"UIアニメーション",
	"UIテキスト変更"
};

// イベント一行分のデータ
struct EventCommand {

	EventCommandType type_;

	UIElement* element_ = nullptr;// UIエレメントのアドレス

	std::string elementRootName_;		  // 指定されたエレメントの名前

	std::string elementAnimationName_;    // 再生されるアニメ名
	std::string elementTextName_;         // 指定されたテキストの名前
	std::string elementTextChangeTexts_;  // 指定されたテキストの変更後のテキストｓ

	std::string TriggerName_;// トリガーの際のタグ名

};

	// 保存 / 読み込み
	void to_json(nlohmann::json& j, const EventCommand& v);
	void from_json(const nlohmann::json& j, EventCommand& v);


// イベント一個分
struct EventCommandAsset {

	std::string eventName_;
	std::vector<EventCommand> commands_;

};

	// 保存 / 読み込み
	void to_json(nlohmann::json& j, const EventCommandAsset& v);
	void from_json(const nlohmann::json& j, EventCommandAsset& v);
