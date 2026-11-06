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
	if (ImGui::BeginNeoSequencer("Sequencer", &currentFrame, &startFrame, &endFrame, { 0, 0 },
		ImGuiNeoSequencerFlags_EnableSelection |
		ImGuiNeoSequencerFlags_Selection_EnableDragging |
		ImGuiNeoSequencerFlags_Selection_EnableDeletion))
	{
		if (ImGui::BeginNeoGroup("Transform", &transformOpen))
		{
			/* 各トラック */
			for (int i = 0; i < items_.size(); ++i)
			{

				if (ImGui::BeginNeoTimelineEx(items_.at(i).name.c_str()))
				{
					/* 現在このトラックが選択されているか判定 */
					if (ImGui::IsNeoTimelineSelected())
					{
						currentSelectItemIndex = i; // 選択中のインデックスを更新
					}

					bool lerped = false;

					// 全てのキー
					for (int j = 0; j < items_.at(i).keyFrames.size(); ++j) {

						KeyFrame<Vector3>& key = items_.at(i).keyFrames.at(j);

						ImGui::NeoKeyframe(&key.frame);

						if (ImGui::IsNeoKeyframeSelected()) {
							currentSelectItemKeyIndex = j; // 選択中のインデックスを更新
						}


						if (lerped) continue;
						if (!isPlaying) continue;

						if (j == items_.at(i).keyFrames.size() - 1) continue;

						// 現在のフレーム数から補完するキーフレームを探す
						KeyFrame<Vector3>& nextKey = items_.at(i).keyFrames.at(j + 1);// 次のキー

						if (currentFrame >= key.frame && currentFrame <= nextKey.frame) {

							float frameRange = static_cast<float>(nextKey.frame - key.frame);

							if (frameRange != 0.0f) {
								// t = 現在 / 全体
								float t = (currentFloatFrame - static_cast<float>(key.frame)) / frameRange;

								// 範囲外参照を防ぐために 0.0f 〜 1.0f にクランプ（安全策）
								t = std::clamp(t, 0.0f, 1.0f);

								// 補間
								UIManager::GetInstance()->GetSelectedElement()->sprite_.GetScreenTransform().translation_ =
									Lerp(key.value, nextKey.value, Easing::Get(t, EasingType::None));

								lerped = true;
							}
							else {

								// 終点と位置が一緒なら動かない
								UIManager::GetInstance()->GetSelectedElement()->sprite_.GetScreenTransform().translation_ = key.value;
								lerped = true;
							}
						}
					}

	
					//ImGui::Text("frame :%d  %f,  %f,   %f",key.frame, key.value.x, key.value.y, key.value.z);
					ImGui::EndNeoTimeLine();


					// 右クリ - 削除
					if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
					{
						if (currentSelectItemKeyIndex >= 0 && currentSelectItemKeyIndex < items_.at(i).keyFrames.size()) {

							items_.at(i).keyFrames.erase(items_.at(i).keyFrames.begin() + currentSelectItemKeyIndex);

							ImGui::EndNeoGroup();
							ImGui::EndNeoSequencer();
							return;
						}
					}
					
				}
			}

			ImGui::EndNeoGroup();
		}

		ImGui::EndNeoSequencer();
	}

#endif
}
void UISequencer::Draw()
{

}
