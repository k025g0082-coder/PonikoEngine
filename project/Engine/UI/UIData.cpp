
#include "UIData.h"
#include "UISequencer.h"

/*
	UIElementAnimationのJson化
*/
void to_json(nlohmann::json& j, const UIElementAnimation& v)
{
	j = nlohmann::json::object();

	j["animationName"] = v.animationName;
	j["durationFrames"] = v.durationFrames;

	// 空じゃなけりゃ
	if (!v.tracks.empty()) {

		// 各トラックをJSON化
		for (const auto& item : v.tracks)
		{
			j["tracks"].push_back(item);
		}
	}
}

/*
	UIElementAnimationのJsonから復元
*/
void from_json(const nlohmann::json& j, UIElementAnimation& v)
{
	if (j.contains("animationName"))
		v.animationName = j["animationName"].get<std::string>();

	if (j.contains("durationFrames"))
		v.durationFrames = j["durationFrames"].get<float>();


	/*
		Track情報は、デフォルトデータをセットする（Transformの文字列とかは固定のため・・）
	*/
	UISequencer::SetDefaultTrackDatas(v);


	if (j.contains("tracks")) {

		const auto& tracksJson = j.at("tracks");

		// インデックスで直接代入するだけ！
		for (size_t i = 0; i < tracksJson.size() && i < v.tracks.size(); ++i) {

			// 上書き
			tracksJson[i].get_to(v.tracks[i]);
		}
	}

}


/*
	UIElementAnimationDataのJson化
*/
void to_json(nlohmann::json& j, const UIElementAnimationDatas& v)
{

	j = nlohmann::json::object();

	j["currentTag"] = v.currentTag_;
	j["animations"] = nlohmann::json::array();

	// 空じゃなけりゃ
	if (!v.animations_.empty()) {

		// 各アイテムをJSON化
		for (const auto& [name, anime] : v.animations_) {
			j["animations"].push_back(anime);
		}
	}
}

/*
	UIElementAnimationDataのJsonから復元
*/
void from_json(const nlohmann::json& j, UIElementAnimationDatas& v)
{
	if (j.contains("currentTag")) {
		v.currentTag_ = j.at("currentTag").get<std::string>();
	}

	if (j.contains("animations")) {

		const auto& animationsJson = j.at("animations");

		// アニメーションを新規登録する
		for (const auto& animeJson : animationsJson) {

			// 復元
			UIElementAnimation anime = animeJson.get<UIElementAnimation>();

			// アニメ名で登録
			v.animations_[anime.animationName] = anime;
			v.currentTag_ = anime.animationName;
		}

		// もし currentTag_ が空、またはマップに存在しない場合は最初のアニメーションをタグにする
		if (v.currentTag_.empty() || v.animations_.find(v.currentTag_) == v.animations_.end()) {
			if (!v.animations_.empty()) {
				v.currentTag_ = v.animations_.begin()->first;
			}
		}
	}
}

//void UIElement::AddChild(std::unique_ptr<UIElement> child)
//{
//	if (!child) return;
//
//	// 子側に自分のアドレスをセット
//	child->parent_ = this;
//
//	// 所有権を移動 子リストに追加
//	children_.push_back(std::move(child));
//
//}
