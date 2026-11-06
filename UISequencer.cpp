#include "UISequencer.h"
#include "Timer.h"

#include "Matrix.h"
#include "Easing.h"

#include "UIManager.h"

#include <im-neo-sequencer/imgui_neo_sequencer.h>
#include "ImGuiManager.h"

#include <algorithm>
#include <imgui/IconsFontAwesome.h>

void UISequencer::Update()
{

	/* もし選択されてない場合はreturn */
	if (selectElementData == nullptr)
		return;


#ifdef USE_IMGUI


	// 再生
	if (isPlaying) {

		const float kRate = 60.0f;// 60fps

		currentFloatFrame += Timer::GetDeltaTime() * kRate;
		currentFrame = static_cast<int>(currentFloatFrame);
	}

	/* 再生ボタン */
	const char* icon;
	if (isPlaying) {
		icon = (char*)ICON_FA_PLAY;
	}
	else {
		icon = (char*)ICON_FA_STOP;
	}

	if (ImGui::Button(icon)) {
		isPlaying = !isPlaying;
		currentFrame = 0;
		currentFloatFrame = 0;
	}

	// ざひょうキー追加
	if (ImGui::Button(icon = (char*)ICON_FA_PLUS)) {

		if (UIManager::GetInstance()->GetSelectedElement()) {

			//座標
			Vector3 vec = UIManager::GetInstance()->GetSelectedElement()->sprite_.GetScreenTransform().translation_;

			AddKeyFrame(SequenceItemType::kPosition, KeyFrame<Vector3>{
				currentFrame,
					vec,
					EasingType::None
			});
		}
	}


	ImGui::Text("currentSelectItemIndex: %d", currentSelectItemIndex);
	ImGui::Text("currentSelectItemKeyIndex: %d", currentSelectItemKeyIndex);


	if (ImGui::BeginNeoSequencer("Sequencer", &currentFrame, &startFrame, &endFrame, { 0, 0 },
		ImGuiNeoSequencerFlags_EnableSelection |
		ImGuiNeoSequencerFlags_Selection_EnableDragging |
		ImGuiNeoSequencerFlags_Selection_EnableDeletion))
	{
		//
		if (ImGui::BeginNeoGroup("Transform", &transformOpen))
		{

			// 現在選択されているデータのアイテム
			auto& items_ = selectElementData->items;


			/* 各トラック */
			for (int i = 0; i < items_.size(); ++i)
			{

				if (ImGui::BeginNeoTimelineEx(kItemTypeTags_[(int)items_.at(i).mType]))
				{
					/* 現在このトラックが選択されているか判定 */
					if (ImGui::IsNeoTimelineSelected())
					{
						currentSelectItemIndex = i; // 選択中のインデックスを更新
					}

					// 全てのキー
					for (int j = 0; j < items_.at(i).keyFrames.size(); ++j) {

						KeyFrame<Vector3>& key = items_.at(i).keyFrames.at(j);

						ImGui::NeoKeyframe(&key.frame);

						if (ImGui::IsNeoKeyframeSelected()) {
							currentSelectItemKeyIndex = j; // 選択中のインデックスを更新
						}

						if (!isPlaying && previousFrame == currentFrame) 
							continue;



						/// 補間
						LerpKeyFrame(i, j);
						


					}

	
					//ImGui::Text("frame :%d  %f,  %f,   %f",key.frame, key.value.x, key.value.y, key.value.z);
					ImGui::EndNeoTimeLine();


					// 右クリ - 削除
					if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
					{
						// ＊未選択チェック
						if (currentSelectItemIndex >= 0 && currentSelectItemIndex < items_.size()) {
							if (currentSelectItemKeyIndex >= 0 && currentSelectItemKeyIndex < items_.at(i).keyFrames.size()) {

								items_.at(currentSelectItemIndex).
									keyFrames.erase(
										items_.at(currentSelectItemIndex).keyFrames.begin() + currentSelectItemKeyIndex);

								ImGui::EndNeoGroup();
								ImGui::EndNeoSequencer();
								return;
							}
						}
					}
					
				}
			}

			ImGui::EndNeoGroup();
		}

		ImGui::EndNeoSequencer();
	}

	//// バーに補間を同期
	//if (!isPlaying && previousFrame != currentFrame) {

	//	LerpKeyFrame(currentSelectItemIndex, currentSelectItemIndex);

	//}

	// 同期
	if (previousFrame != currentFrame) {

		currentFloatFrame = static_cast<float>(currentFrame);

	}
	previousFrame = currentFrame;
#endif
}
void UISequencer::Draw()
{

}


void UISequencer:: AddItem(SequenceItemType type)
{

	if (!selectElementData) return;

	auto& items_ = selectElementData->items;

	/* すでに同じタイプがある場合は追加しない */
	auto it = std::find_if(
		items_.begin(),
		items_.end(),
		[type](const SequenceItem& t) {
			return t.mType == type;
		}
	);

	if (it != items_.end())return;

	// 追加
	items_.push_back(SequenceItem{
		.mType = type
		});
}

void UISequencer::AddKeyFrame(SequenceItemType type, KeyFrame<Vector3> key)
{

	if (!selectElementData) return;

	auto& items_ = selectElementData->items;


	/*
		もしitems_にそのタイプのアイテムが存在しない場合、新しくアイテムを追加する
	*/
	//auto itt = std::find_if(
	//	items_.begin(),
	//	items_.end(),
	//	[type](const SequenceItem& t) {
	//		return t.mType == type;
	//	}
	//);

	//if (itt == items_.end()) AddItem(type);



	// タイプに合った選択されているアイテムの変更
	currentSelectItemIndex = (int)type;

	// たいぷにあったアイテムを取得
	auto& item = items_.at((int)type);

	// フレーム番号順で挿入位置を取得
	auto it = std::lower_bound(
		item.keyFrames.begin(),
		item.keyFrames.end(),
		key.frame,
		[](const KeyFrame<Vector3>& t, int32_t frame) {
			return t.frame < frame;
		}
	);

	// 同じフレームに存在する場合は追加しない
	if (it != item.keyFrames.end() &&
		it->frame == key.frame) {
		return;
	}

	// ソート順を維持して挿入
	item.keyFrames.insert(it, key);

}

/*


	キー補間


*/
void UISequencer::LerpKeyFrame(int itemIndex, int keyIndex)
{
	if (!selectElementData) return;

	auto& items_ = selectElementData->items;

	if (itemIndex < 0 || itemIndex > items_.size())return;
	if (keyIndex < 0  || keyIndex > items_.at(itemIndex).keyFrames.size()) return;


	// キー
	KeyFrame<Vector3>& key = items_.at(itemIndex).keyFrames.at(keyIndex);


	if (keyIndex == items_.at(itemIndex).keyFrames.size() - 1) return;



	// 現在のフレーム数から補完するキーフレームを探す
	KeyFrame<Vector3>& nextKey = items_.at(itemIndex).keyFrames.at(keyIndex + 1);// 次のキー

	if (currentFrame >= key.frame && currentFrame <= nextKey.frame) {

		float frameRange = static_cast<float>(nextKey.frame - key.frame);

		if (frameRange != 0.0f) {

			// t = 現在 / 全体
			float t = (currentFloatFrame - static_cast<float>(key.frame)) / frameRange;

			// 範囲外参照を防ぐために 0.0f 〜 1.0f にクランプ（安全策）
			t = std::clamp(t, 0.0f, 1.0f);


			switch ((SequenceItemType)itemIndex) {

				/* 　座標　 */
			case SequenceItemType::kPosition:

				// 補間
				UIManager::GetInstance()->GetSelectedElement()->sprite_.GetScreenTransform().translation_ =
					Lerp(key.value, nextKey.value, Easing::Get(t, EasingType::None));

				break;

				/* 　拡大　 */
			case SequenceItemType::kScale:

				// 補間
				UIManager::GetInstance()->GetSelectedElement()->sprite_.GetScreenTransform().scale_ =
					Lerp(key.value, nextKey.value, Easing::Get(t, EasingType::None));

				break;

				/* 　回転　 */
			case SequenceItemType::kRotation:

				// 補間
				UIManager::GetInstance()->GetSelectedElement()->sprite_.GetScreenTransform().rotation_ =
					Lerp(key.value, nextKey.value, Easing::Get(t, EasingType::None));

				break;
			}

			//lerped = true;
		}
		else {

			switch ((SequenceItemType)itemIndex) {

				// 終点と位置が一緒なら動かない
			case SequenceItemType::kPosition:
				UIManager::GetInstance()->GetSelectedElement()->sprite_.GetScreenTransform().translation_ = key.value;
				break;
			case SequenceItemType::kScale:
				UIManager::GetInstance()->GetSelectedElement()->sprite_.GetScreenTransform().scale_ = key.value;
				break;
			case SequenceItemType::kRotation:
				UIManager::GetInstance()->GetSelectedElement()->sprite_.GetScreenTransform().rotation_ = key.value;
				break;
			}


			//lerped = true;
		}
	}

}



/*
	SequenceItemのJson化
*/
void to_json(nlohmann::json& j, const SequenceItem& v)
{
	// タイプ
	j["mType"] = (int)v.mType;

	j["keyFrames"] = nlohmann::json::array();// 空でも保存

	// 各キーをJSON化
	for (auto& item : v.keyFrames) {
		j["keyFrames"].push_back(KeyFrame<Vector3>::ToJson(item));
	}
}

/*
	SequenceItemのJsonから復元
*/
void from_json(const nlohmann::json& j, SequenceItem& v)
{

	// タイプ
	v.mType = (SequenceItemType)j["mType"].get<int>();


	v.keyFrames.clear();

	// JSONから各キー復元
	for (const auto& item : j.at("keyFrames"))
	{
		v.keyFrames.push_back(
			KeyFrame<Vector3>::FromJson(item)
		);
	}
}


/*
	UIElementAnimationDataのJson化
*/
void to_json(nlohmann::json& j, const UIElementAnimationData& v)
{

	j = nlohmann::json::object();

	j["items"] = nlohmann::json::array();

	// 各アイテムをJSON化
	for (const auto& item : v.items)
	{
		j["items"].push_back(item);
	}
}

/*
	UIElementAnimationDataのJsonから復元
*/
void from_json(const nlohmann::json & j, UIElementAnimationData & v)
{

	// JSONから各アイテム復元
	for (const auto& item : j.at("items"))
	{
		SequenceItem s = item.get<SequenceItem>();

		// 同じItemTypeが存在するか検索
		auto it = std::find_if(
			v.items.begin(),
			v.items.end(),
			[&s](const SequenceItem& t)
			{
				return t.mType == s.mType;
			}
		);

		// 存在するなら上書き
		if (it != v.items.end())
		{
			*it = s;
		}
		// 存在しないなら追加
		else
		{
			v.items.push_back(s);
		}
	}
}
