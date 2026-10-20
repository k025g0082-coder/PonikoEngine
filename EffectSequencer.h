#pragma once
#include "ImSequencer.h"
#include <vector>

/* 
    エフェクトのタイムライン表示
*/


// これを継承する必要がある
class EffectSequencer : public ImSequencer::SequenceInterface
{
public:
    // シーケンスのアイテム
    struct SequenceItem
    {
        int mType;       // タイプ？
        int mFrameStart; // 開始時間
        int mFrameEnd;   // 終了時間
    };

    // 元ので　＝０がついてるやつは必ず実装
    void Get(int index, int** start, int** end, int* type, unsigned int* color) override;

    const char* GetItemTypeName(int typeIndex) const override{ 
        return SequencerItemTypeNames[typeIndex];
    }
    const char* GetItemLabel(int index) const override
    {
        static char tmps[512];
        snprintf(tmps, 512, "[%02d] %s", index, SequencerItemTypeNames[items_[index].mType]);
        return tmps;
    }


    int GetFrameMin() const override {
        return mFrameMin;
    };
    int GetFrameMax() const override {
        return mFrameMax;
    };
    int GetItemCount() const override {
        return (int)items_.size();
    };
    int GetItemTypeCount() const override {
        return sizeof(SequencerItemTypeNames) / sizeof(char*); 
    }

    void Add(int type) override {
        items_.push_back(SequenceItem{ type, 0, 10 });
    };
    void Del(int index) override {
        items_.erase(items_.begin() + index); 
    }
    void Duplicate(int index) override {
        items_.push_back(items_[index]);
    }

    // アイテムのタグ名
    const char* GetSequencerItemTypeNames(int index) {
        return SequencerItemTypeNames[index];
    }

    // タイムラインの最大時間？
    int mFrameMin = 0;
    int mFrameMax = 0;

    // アイテムｓ
    std::vector<SequenceItem> items_;

    // タイムラインの一覧
    const char* SequencerItemTypeNames[5] = {
        "Camera","Music", "ScreenEffect", "FadeIn", "Animation"
    };

};

