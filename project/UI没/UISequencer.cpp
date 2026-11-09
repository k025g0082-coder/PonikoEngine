#include "UISequencer.h"

#include <imgui/IconsFontAwesome.h>

#include "ImGuiManager.h"
#include "Timer.h"


void UISequencer::Get(int index, int** start, int** end, int* type, unsigned int* color)
{
    SequenceTrack& item = tracks_[index];
    if (color)
        *color = 0xFFAA8080; // same color for everyone, return color based on type
    if (start)
        *start = &item.mFrameStart;
    if (end)
        *end = &item.mFrameEnd;
    if (type)
        *type = (int)item.mType;
}


void UISequencer::UpdateSequencer()
{
	// 再生中なら
	if (isPlaying) {
		
		const float kRate = 60.0f;// 60fps

		currentFloatFrame += Timer::GetDeltaTime() * kRate;
		currentFrame = static_cast<int>(currentFloatFrame);
	}
	else {

		// マウス操作で変更されたフレームを同期
		currentFloatFrame = static_cast<float>(currentFrame);

	}

	//
	UpdateItems();
    tracks_;

}

void UISequencer::DrawSequencer()
{

#ifdef USE_IMGUI

	/* 色 */
	ImGui::PushStyleColor(
		ImGuiCol_WindowBg,
		ImVec4(0.10f, 0.14f, 0.20f, 1.0f)
	);

	ImGui::Begin("ElementView");


	ImGui::PushItemWidth(130);

	/* 再生ボタン */
	const char* icon;
	if (isPlaying) {
		icon = (char*) ICON_FA_PLAY;
	}
	else {
		icon = (char*)ICON_FA_STOP;
	}

	if (ImGui::Button(icon)) {
		isPlaying = !isPlaying;
	}

	ImGui::InputInt("Frame ", &currentFrame);
	ImGui::SameLine();

	ImGui::InputInt("Frame Min", &mFrameMin);
	ImGui::SameLine();
	ImGui::InputInt("Frame Max", &mFrameMax);

	ImGui::PopItemWidth();
	

	// シーケンサーの描きだし位置を得る
	sequencerStartPosition = ImGui::GetCursorScreenPos();

	Sequencer(
		this,
		&currentFrame,
		&expanded,
		&selectedTrackIndex_,
		&firstFrame,
		ImSequencer::SEQUENCER_EDIT_ALL | ImSequencer::SEQUENCER_CHANGE_FRAME
	);

	// add a UI to edit that particular item
	//if (selectedEntry != -1)
	//{
	//	const UISequencer::SequenceItem& item = sequencer_.items_[selectedEntry];
	//	ImGui::Text("I am a %s, please edit me", sequencer_.GetSequencerItemTypeNames(item.mType));
	//	// switch (type) ....
	//}

	ImGui::End();

	// iro
	ImGui::PopStyleColor();
#endif
}

void UISequencer::UpdateItems()
{
#ifdef USE_IMGUI

#endif
}


void UISequencer::CustomDraw(
    int index,
    ImDrawList* drawList,
    const ImRect& rc,
    const ImRect& legendRect,
    const ImRect& clippingRect,
    const ImRect& legendClippingRect
)
{
    // トラックの範囲チェック
    if (index < 0 || index >= static_cast<int>(tracks_.size()))
        return;

    auto& track = tracks_[index];

    // マウス情報
    ImVec2 mousePos = ImGui::GetMousePos();

    bool isLeftClicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
    bool isLeftDown = ImGui::IsMouseDown(ImGuiMouseButton_Left);

    // トラックの中央Y座標
    float centerY = rc.Min.y + rc.GetHeight() * 0.5f;

    // タイムライン全体のフレーム範囲
    int frameRange = mFrameMax - mFrameMin;

    if (frameRange <= 0)
        frameRange = 1;

    // フレーム番号をX座標に変換
    auto FrameToX = [&](int frame) -> float
        {
            float ratio = static_cast<float>(frame - mFrameMin)
                / static_cast<float>(frameRange);

            return rc.Min.x + ratio * rc.GetWidth();
        };

    // X座標をフレーム番号に変換
    auto XToFrame = [&](float x) -> int
        {
            float ratio = (x - rc.Min.x) / rc.GetWidth();

            int frame = mFrameMin
                + static_cast<int>(ratio * frameRange);

            return ImClamp(frame, mFrameMin, mFrameMax);
        };

    // =========================================================
    // アイテムごとの描画
    // =========================================================

    for (int itemIdx = 0;
        itemIdx < static_cast<int>(track.items.size());
        ++itemIdx)
    {
        auto& item = track.items[itemIdx];

        // -----------------------------------------------------
 // 1. アイテムバー
 // -----------------------------------------------------

        float barMinX = FrameToX(item.mFrameStart);
        float barMaxX = FrameToX(item.mFrameEnd);

        ImVec2 barMin(
            ImMax(barMinX, rc.Min.x),
            rc.Min.y + 4.0f
        );

        ImVec2 barMax(
            ImMin(barMaxX, rc.Max.x),
            rc.Max.y - 4.0f
        );

        // マウスがバーの中にあるか
        bool isHoveredBar =
            mousePos.x >= barMin.x &&
            mousePos.x <= barMax.x &&
            mousePos.y >= barMin.y &&
            mousePos.y <= barMax.y;

        // ドラッグ開始
        if (isHoveredBar &&
            isLeftClicked &&
            !isDragging_)
        {
            isDraggingItem_ = true;

            draggingTrackIndex_ = index;
            draggingItemIndex_ = itemIdx;

            dragStartMouseX_ = mousePos.x;
            dragStartFrame_ = item.mFrameStart;
        }

        // ドラッグ中
        if (isDraggingItem_ &&
            draggingTrackIndex_ == index &&
            draggingItemIndex_ == itemIdx)
        {
            if (isLeftDown)
            {
                // マウス移動量をフレームに変換
                float deltaX = mousePos.x - dragStartMouseX_;

                int deltaFrame = static_cast<int>(
                    deltaX / rc.GetWidth() * frameRange
                    );

                // 元の長さ
                int duration = item.mFrameEnd - item.mFrameStart;

                // 新しい開始フレーム
                int newStart = dragStartFrame_ + deltaFrame;

                // タイムライン範囲内に制限
                newStart = ImClamp(
                    newStart,
                    mFrameMin,
                    mFrameMax - duration
                );

                int newEnd = newStart + duration;

                // 移動量
                int actualDelta = newStart - item.mFrameStart;

                item.mFrameStart = newStart;
                item.mFrameEnd = newEnd;

                // キーフレームも一緒に移動
                for (auto& key : item.keyFrames)
                {
                    key.frame += actualDelta;
                }
            }
            else
            {
                // マウスを離した
                isDraggingItem_ = false;

                draggingTrackIndex_ = -1;
                draggingItemIndex_ = -1;
            }
        }

        // バーの色
        ImU32 barColor = isHoveredBar
            ? IM_COL32(70, 160, 240, 180)
            : IM_COL32(50, 120, 200, 100);

        // 背景
        drawList->AddRectFilled(
            barMin,
            barMax,
            barColor,
            4.0f
        );

        // 枠線
        drawList->AddRect(
            barMin,
            barMax,
            IM_COL32(100, 180, 255, 255),
            4.0f,
            0,
            1.5f
        );

        // -----------------------------------------------------
        // 2. キーフレーム
        // -----------------------------------------------------

        for (int k = 0;
            k < static_cast<int>(item.keyFrames.size());
            ++k)
        {
            auto& keyFrame = item.keyFrames[k];

            float centerX = FrameToX(keyFrame.frame);

            // 描画範囲外ならスキップ
            if (centerX < rc.Min.x || centerX > rc.Max.x)
                continue;

            ImVec2 centerPos(centerX, centerY);

            float keySize = 6.0f;

            // -------------------------------------------------
            // ホバー判定
            // -------------------------------------------------

            float dx = mousePos.x - centerPos.x;
            float dy = mousePos.y - centerPos.y;

            float distSq = dx * dx + dy * dy;

            bool isHovered =
                distSq <= (keySize + 3.0f) * (keySize + 3.0f);

            // -------------------------------------------------
            // 選択処理
            // -------------------------------------------------

            if (isHovered && isLeftClicked)
            {
                // 全キーフレームの選択解除
                for (auto& t : tracks_)
                {
                    for (auto& itm : t.items)
                    {
                        for (auto& kf : itm.keyFrames)
                        {
                            kf.isSelected = false;
                        }
                    }
                }

                // 選択したキーフレーム
                keyFrame.isSelected = true;

                selectedTrackIndex_ = index;
                selectedItemIndex_ = itemIdx;
                selectedKeyIndex_ = k;

                isDragging_ = true;
            }

            // -------------------------------------------------
            // ドラッグ処理
            // -------------------------------------------------

            if (isDragging_ &&
                selectedTrackIndex_ == index &&
                selectedItemIndex_ == itemIdx &&
                selectedKeyIndex_ == k)
            {
                if (isLeftDown)
                {
                    int newFrame = XToFrame(mousePos.x);

                    // アイテムの範囲内に制限
                    newFrame = ImClamp(
                        newFrame,
                        item.mFrameStart,
                        item.mFrameEnd
                    );

                    // 前後のキーフレームとの重複を防止
                    int minFrame = item.mFrameStart;
                    int maxFrame = item.mFrameEnd;

                    if (k > 0)
                    {
                        minFrame = item.keyFrames[k - 1].frame + 1;
                    }

                    if (k + 1 < static_cast<int>(item.keyFrames.size()))
                    {
                        maxFrame = item.keyFrames[k + 1].frame - 1;
                    }

                    keyFrame.frame = ImClamp(
                        newFrame,
                        minFrame,
                        maxFrame
                    );
                }
                else
                {
                    isDragging_ = false;
                }
            }

            // -------------------------------------------------
            // キーフレームの色
            // -------------------------------------------------

            ImU32 color =
                keyFrame.isSelected
                ? IM_COL32(255, 220, 0, 255)
                : (isHovered
                    ? IM_COL32(255, 255, 255, 255)
                    : IM_COL32(0, 200, 255, 255));

            // 描画
            DrawKeyFrame(
                drawList,
                centerPos,
                keySize,
                color
            );
        }
    }
}
void UISequencer::DrawKeyFrame(
    ImDrawList* drawList,
    ImVec2 center,
    float size,
    ImU32 color
) {
    if (!drawList) return;

    // ひし形の4頂点（上・右・下・左）
    ImVec2 points[4] = {
        ImVec2(center.x,        center.y - size), // 上
        ImVec2(center.x + size, center.y),        // 右
        ImVec2(center.x,        center.y + size), // 下
        ImVec2(center.x - size, center.y)         // 左
    };

    // 1. 中身の塗りつぶし描画
    drawList->AddConvexPolyFilled(
        points,
        4,
        color
    );

    // 2. くっきり見せるための外枠線描画（白色）
    drawList->AddPolyline(
        points,
        4,
        IM_COL32(255, 255, 255, 255), // 枠線の色（白）
        ImDrawFlags_Closed,           // 閉じた多角形として描画
        1.5f                          // 線の太さ
    );
}