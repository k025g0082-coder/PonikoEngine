
#include "SequencerData.h"

/*
	SequenceItemのJson化　・・やり取りするのはキー情報のみ
*/
void to_json(nlohmann::json& j, const SequenceItem& v)
{
	j = nlohmann::json::object();

	// 空じゃなけりゃ
	if (!v.keyFrames.empty()) {

		// 各キーをJSON化
		for (auto& item : v.keyFrames) {
			j["keyFrames"].push_back(KeyFrame<Vector3>::ToJson(item));
		}
	}
}

/*
	SequenceItemのJsonから復元　・・やり取りするのはキー情報のみ
*/
void from_json(const nlohmann::json& j, SequenceItem& v)
{
	v.keyFrames.clear(); //

	if (j.contains("keyFrames")) {

		// JSONから各キー復元
		for (const auto& item : j.at("keyFrames"))
		{
			v.keyFrames.push_back(
				KeyFrame<Vector3>::FromJson(item)
			);
		}

	}
}


/*
	SequenceTrackのJson化　・・やり取りするのはキー情報のみ
*/
void to_json(nlohmann::json& j, const SequenceTrack& v)
{
	j = nlohmann::json::object();

	// 空じゃなけりゃ
	if (!v.items.empty()) {

		// 各トラック内アイテムをJSON化
		for (const auto& item : v.items)
		{
			j["items"].push_back(item);
		}
	}
}

/*
	SequenceTrackのJsonから復元　・・やり取りするのはキー情報のみ
*/
void from_json(const nlohmann::json& j, SequenceTrack& v)
{

	if (j.contains("items")) {

		// JSONから各アイテム復元
		const auto& itemsJson = j.at("items");

		// インデックスで直接代入するだけ！
		for (size_t i = 0; i < itemsJson.size() && i < v.items.size(); ++i) {

			// 上書き
			itemsJson[i].get_to(v.items[i]);

		}
	}

	//for (const auto& item : j.at("items"))
	//{
	//	SequenceItem s = item.get<SequenceItem>();

	//	// 同じItemTypeが存在するか検索
	//	auto it = std::find_if(
	//		v.items.begin(),
	//		v.items.end(),
	//		[&s](const SequenceItem& t)
	//		{
	//			return t.mType == s.mType;
	//		}
	//	);

	//	// 存在するなら上書き
	//	if (it != v.items.end())
	//	{
	//		*it = s;
	//	}
	//	// 存在しないなら追加
	//	else
	//	{
	//		v.items.push_back(s);
	//	}
	//}
}


