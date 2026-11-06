#pragma once
#include <cstdint>

#include <vector>
#include <string>

#include "Vector.h"
#include "Easing.h"

/*
    UIのタイムライン表示
*/

enum class SequenceItemType {
    kPosition,
    kScale,
    kRotation,
};


template <typename T>

// キーフレーム
struct KeyFrame {
    int32_t frame;             // 時間
    T value;                // 値　floatやVector3など
    EasingType easingType; // 補間タイプ

    // 選択されたか
    bool isSelected = false;
};

// シーケンスのアイテム
struct SequenceItem
{
    SequenceItemType mType;       // タイプ？

    std::string name;

    std::vector<KeyFrame<Vector3>> keyFrames;  //　キーたち

    int mFrameStart = 0;    // 開始時間
    int mFrameEnd = 10;   // 終了時間

};

// シーケンスのトラック
//struct SequenceTrack
//{
//    std::vector<SequenceItem> items;  //　アイテムたち
//
//};


class UISequencer
{
public:
	void Update();
	void Draw();

    //void AddTrack()

    /* キーフレーム挿入 */
    void AddKeyFrame(SequenceItemType type, KeyFrame<Vector3> key) {

        if (currentSelectItemIndex == -1) return;

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

private:

	bool isPlaying = false;

	int32_t currentSelectItemIndex = -1;
    int32_t currentSelectItemKeyIndex = -1;

	int32_t currentFrame = 0;
	float currentFloatFrame = 0;

	int32_t startFrame = -10;
	int32_t endFrame = 64;

    std::vector<SequenceItem> items_ = {
        { SequenceItemType::kPosition,"座標" },
        { SequenceItemType::kScale,"拡大" },
        { SequenceItemType::kRotation,"回転" },
    };

	bool transformOpen = false;
	std::vector<int32_t> keys = { 0, 10, 24 };
	bool doDelete = false;

};

