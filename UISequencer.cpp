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
	if (selectElement == nullptr)
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
		icon = (char*)ICON_FA_STOP;
	}
	else {
		icon = (char*)ICON_FA_PLAY;
	}

	if (ImGui::Button(icon)) {
		isPlaying = !isPlaying;
		currentFrame = 0;
		currentFloatFrame = 0;
	}


	ImGui::Text("currentSelectTrackIndex: %d", currentSelectTrackIndex);
	ImGui::Text("currentSelectItemIndex: %d", currentSelectItemIndex);
	ImGui::Text("currentSelectItemKeyIndex: %d", currentSelectItemKeyIndex);


	if (ImGui::BeginNeoSequencer("Sequencer", &currentFrame, &startFrame, &endFrame, { 0, 0 },
		ImGuiNeoSequencerFlags_EnableSelection |
		ImGuiNeoSequencerFlags_Selection_EnableDragging |
		ImGuiNeoSequencerFlags_Selection_EnableDeletion))
	{


		//currentSelectAnimationTag = "NewAnime";


		//if (currentSelectAnimationTag.empty())



		// 現在選択されているアニメデータのトラックたち
		auto& tracks_ = selectElement->animationDatas_.animations_[selectElement->animationDatas_.currentTag_].tracks;

		for (int track = 0; track < tracks_.size(); ++track)
		{

			/*
				各トラック　　Transform・・Color・・
			*/
			if (ImGui::BeginNeoGroup(tracks_.at(track).trackName.c_str(), &tracks_.at(track).isOpen))
			{

				// 現在選択されているトラックのデータのアイテム
				auto& items_ = tracks_.at(track).items;

				if (ImGui::IsNeoTimelineSelected())
				{
					currentSelectTrackIndex = track; // 選択されたGroupのインデックス
				}

				/* 
					各アイテム　　Transform:座標,拡大,回転　Color:R,G,B,アルファ・・
				*/
				for (int i = 0; i < items_.size(); ++i)
				{

					if (ImGui::BeginNeoTimelineEx(items_.at(i).name.c_str()))
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
							LerpKeyFrame(selectElement,currentFloatFrame,track,i, j);



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

//
//void UISequencer:: AddItem(SequenceItemType type)
//{
//
//	if (!selectElementData) return;
//
//	auto& items_ = selectElementData->items;
//
//	/* すでに同じタイプがある場合は追加しない */
//	auto it = std::find_if(
//		items_.begin(),
//		items_.end(),
//		[type](const SequenceItem& t) {
//			return t.mType == type;
//		}
//	);
//
//	if (it != items_.end())return;
//
//	// 追加
//	items_.push_back(SequenceItem{
//		.mType = type
//		});
//}

void UISequencer::AddKeyFrame(SequenceItemType type, KeyFrame<Vector3> key)
{

	if (!selectElement) return;

	//if (currentSelectAnimationTag.empty()) return;
	if (currentSelectTrackIndex == -1) return;


	// 現在選択中のアニメーションの、現在選んでいるIndexの、アイテムｓ
	auto& items_ = 
		selectElement->animationDatas_.animations_[selectElement->animationDatas_.currentTag_].tracks.at(currentSelectTrackIndex).items;



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

void UISequencer::SetDefaultTrackDatas(UIElementAnimation& anime)
{

	// トラックのみ操作
	anime.tracks = {

		/*
			Transform
		*/
		SequenceTrack{
			.trackName = "Transform",
			.items =
			{
				{ SequenceItemType::kPosition, "座標" },
				{ SequenceItemType::kScale,    "拡大" },
				{ SequenceItemType::kRotation, "回転" }
			}
		},

		/*
			Alpha
		*/
		SequenceTrack{
			.trackName = "透明度",
			.items =
			{
				{ SequenceItemType::kAlpha, "Alpha" },
			}
		}
	};
	
}

void UISequencer::AnimationComboView()
{
#ifdef USE_IMGUI
	
	if (!selectElement) return;

	/*
		現在選択されているエレメントのアニメデータｓ
	*/
	auto& animations_ = selectElement->animationDatas_.animations_;


		// 1. 現在選択されているタグが空、または存在しない場合はデフォルトを設定
	if (currentSelectAnimationTag.empty() ||
		animations_.find(currentSelectAnimationTag) == animations_.end())
	{
		if (!animations_.empty()) {
			currentSelectAnimationTag = animations_.begin()->first;
		}
	}

	// 2. コンボボックスの表示開始
	if (ImGui::BeginCombo("Animation Clip", selectElement->animationDatas_.currentTag_.c_str()))
	{
		// Map内の全アニメーションをループして選択肢を作る
		for (const auto& [name, anime] : animations_)
		{
			const bool isSelected = (currentSelectAnimationTag == name);

			// 項目を描画（選択されたら currentSelectAnimationTag を更新）
			if (ImGui::Selectable(name.c_str(), isSelected))
			{
				selectElement->animationDatas_.currentTag_ = name; // タグを直接更新

				currentFrame = 0;
				currentFloatFrame = 0.0f;
				currentSelectTrackIndex = -1;
				currentSelectItemIndex = -1;
				currentSelectItemKeyIndex = -1;
			}

			// 現在選択中の項目に初期フォーカスをあてる
			if (isSelected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}

		ImGui::EndCombo();
	}

	// 新規アニメーション作成ボタン
	if (ImGui::Button("New Animation"))
	{
		std::string newName = "Anime_" + std::to_string(animations_.size());

		UIElementAnimation newAnime;
		newAnime.animationName = newName;
		UISequencer::SetDefaultTrackDatas(newAnime);

		animations_[newName] = newAnime;
		currentSelectAnimationTag = newName;
		selectElement->animationDatas_.currentTag_ = newName; //
	}

#endif
}

/*


	キー補間


*/
void UISequencer::LerpKeyFrame(UIElement* element, float animationFrame, int trackIndex ,int itemIndex, int keyIndex)
{

	auto& items_ = element->animationDatas_.animations_[element->animationDatas_.currentTag_].tracks[trackIndex].items;


	if (itemIndex < 0 || itemIndex > items_.size())return;
	if (keyIndex < 0  || keyIndex > items_.at(itemIndex).keyFrames.size()) return;


	// キー
	KeyFrame<Vector3>& key = items_.at(itemIndex).keyFrames.at(keyIndex);


	if (keyIndex == items_.at(itemIndex).keyFrames.size() - 1) return;



	// 現在のフレーム数から補完するキーフレームを探す
	KeyFrame<Vector3>& nextKey = items_.at(itemIndex).keyFrames.at(keyIndex + 1);// 次のキー

	//if (currentFrame >= key.frame && currentFrame <= nextKey.frame) {
	if (animationFrame >= static_cast<float>(key.frame) && animationFrame <= static_cast<float>(nextKey.frame)) {

		float frameRange = static_cast<float>(nextKey.frame - key.frame);

		if (frameRange != 0.0f) {

			// t = 現在 / 全体
	//		float t = (currentFloatFrame - static_cast<float>(key.frame)) / frameRange;
			float t = (animationFrame - static_cast<float>(key.frame)) / frameRange;

			// 範囲外参照を防ぐために 0.0f 〜 1.0f にクランプ（安全策）
			t = std::clamp(t, 0.0f, 1.0f);


			switch ((SequenceItemType)itemIndex) {

				/* 　座標　 */
			case SequenceItemType::kPosition:

				// 補間
				element->sprite_.GetScreenTransform().translation_ =
					Lerp(key.value, nextKey.value, Easing::Get(t, EasingType::None));

				break;

				/* 　拡大　 */
			case SequenceItemType::kScale:

				// 補間
				element->sprite_.GetScreenTransform().scale_ =
					Lerp(key.value, nextKey.value, Easing::Get(t, EasingType::None));

				break;

				/* 　回転　 */
			case SequenceItemType::kRotation:

				// 補間
				element->sprite_.GetScreenTransform().rotation_ =
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

