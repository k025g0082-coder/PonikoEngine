#pragma once

#include <imgui.h>
#include "imgui_internal.h"
#include "ImSequencer.h"

#include <vector>

#include "Vector.h"

/*
    UIのタイムライン表示
*/

enum class SequenceItemType {
    kPosition,
    kScale,
    kRotation,
};

enum class EasingType {
    None,
    Sine
};


template <typename T>

// キーフレーム
struct KeyFrame {
    int frame;             // 時間
    T value;                // 値　floatやVector3など
    EasingType easingType; // 補間タイプ

    // 選択されたか
    bool isSelected = false;
};

// シーケンスのアイテム
struct SequenceItem
{
 
    std::vector<KeyFrame<Vector3>> keyFrames;  //　キーたち

    int mFrameStart = 0;    // 開始時間
    int mFrameEnd   = 10;   // 終了時間

};

// シーケンスのトラック
struct SequenceTrack
{

    SequenceItemType mType;       // タイプ？

    int mFrameStart = 0;
    int mFrameEnd = 100;

    std::vector<SequenceItem> items;  //　アイテムたち

};


// これを継承する必要がある
class UISequencer : public ImSequencer::SequenceInterface
{
public:

    // 元ので　＝０がついてるやつは必ず実装
    void Get(int index, int** start, int** end, int* type, unsigned int* color) override;

    const char* GetItemTypeName(int typeIndex) const override {
        return SequencerItemTypeNames[typeIndex];
    }
    const char* GetItemLabel(int index) const override
    {
        static char tmps[512];
        snprintf(tmps, 512, "[%02d] %s", index, SequencerItemTypeNames[(int)tracks_[index].mType]);
        return tmps;
    }


    int GetFrameMin() const override {
        return mFrameMin;
    };
    int GetFrameMax() const override {
        return mFrameMax;
    };
    int GetItemCount() const override {
        return (int)tracks_.size();
    };
    int GetItemTypeCount() const override {
        return sizeof(SequencerItemTypeNames) / sizeof(char*);
    }

    size_t GetCustomHeight(int index) override
    {

        return 32;
    }


    void Add(int type) override {
        tracks_.push_back(
            SequenceTrack{
                (SequenceItemType)type});
    };

    void Del(int index) override {
        tracks_.erase(tracks_.begin() + index);
    }
    void Duplicate(int index) override {
        tracks_.push_back(tracks_[index]);
    }


    // トラック一覧にタイプが存在するか
    bool IsTrackType(SequenceItemType type) {

        // 同じタイプのトラックなら追加しない(座標とか被る
        auto it = std::find_if(
            tracks_.begin(),
            tracks_.end(),
            [type](const SequenceTrack& t) {
                return t.mType == type;
            });
      
        // みつかった
        if (it != tracks_.end())return true;

        return false;
    }

    void AddItem(SequenceItem item)
    {
        if (selectedTrackIndex_ == -1) return;

        tracks_.at(selectedTrackIndex_).items.push_back(
            item
        );
    }

    /*
        キーフレーム追加（Vector3）
    */
    void AddKeyFrameVector3 (KeyFrame<Vector3> key) {

        // 未選択
        if (selectedTrackIndex_ == -1 ||
            selectedItemIndex_ == -1) {
            return;
        }

        // 選択中のアイテムを取得
        auto& item = tracks_
            .at(selectedTrackIndex_)
            .items
            .at(selectedItemIndex_);

        // 選択アイテムの範囲外
        if (item.mFrameStart > key.frame ||
            item.mFrameEnd < key.frame
            ) return;

    
        // フレーム番号順で挿入位置を取得
        auto it = std::lower_bound(
            item.keyFrames.begin(),
            item.keyFrames.end(),
            key.frame,
            [](const KeyFrame<Vector3>& t, int frame) {
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


    // アイテムのタグ名
    const char* GetSequencerItemTypeNames(int index) {
        return SequencerItemTypeNames[index];
    }


    /*
    
        こっから派生

    */

    // 更新
    void UpdateSequencer();

    // 描画
    void DrawSequencer();


    // キーフレームの描画
    void UpdateItems();


    //　今再生中か
    bool isPlaying = false;


    // 現在の再生時間
    int currentFrame = 0;
    float currentFloatFrame = 0.0f;

    // ?
    int selectedItemIndex_ = -1;  // 選択中のアイテム


    int firstFrame = 0;
    bool expanded = true;

    // タイムラインの最大時間？
    int mFrameMin = 0;
    int mFrameMax = 200;


    // シーケンサーウインドウの左上０，０位置
    ImVec2 sequencerStartPosition;

    // 演出/シーケンサー管理クラス内
    int selectedTrackIndex_ = -1;  // 選択中のトラック
    int selectedKeyIndex_ = -1;    // 選択中のキーフレームIndex
  


    // アイテムバーのドラッグ
    bool isDraggingItem_ = false;

    // ドラッグ開始時のマウスX座標
    float dragStartMouseX_ = 0.0f;

    // ドラッグ開始時のフレーム
    int dragStartFrame_ = 0;

    // ドラッグ対象
    int draggingTrackIndex_ = -1;
    int draggingItemIndex_ = -1;
    bool isDragging_ = false;

    // トラックｓ
    std::vector<SequenceTrack> tracks_;

    // タイムラインの一覧
    const char* SequencerItemTypeNames[3] = {
        "座標",
        "スケール",
        "拡大"
    };

    // 
    virtual void CustomDraw(
        int index,
        ImDrawList* drawList,
        const ImRect& rc,
        const ImRect& legendRect,
        const ImRect& clippingRect,
        const ImRect& legendClippingRect
    )override;

    // ひし形描画
    void DrawKeyFrame(
        ImDrawList* drawList,
        ImVec2 center,
        float size,
        ImU32 color
    );
};


