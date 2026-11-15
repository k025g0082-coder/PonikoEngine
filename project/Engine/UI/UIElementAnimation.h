#pragma once
#include "Sprite.h"
#include "Vector.h"
#include "Easing.h"

#include "SequencerData.h"

#include "Text.h"

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

/*
*
*
    アニメ
*
*
*/
struct UIElementAnimation {

    // アニメ名
    std::string animationName = "NewAnime";

    // アニメ時間
    float durationFrames = 60.0f;

    // 再生中か
    bool isPlaying = false;

    // フレーム計測
    float frame = 0;

    // Transform,色といったトラック群。
    // 最終的なTrack->Items->keyFramesという入れ子構造のデータ
    std::vector<SequenceTrack> tracks;

};

// 保存 / 読み込み
void to_json(nlohmann::json& j, const UIElementAnimation& v);
void from_json(const nlohmann::json& j, UIElementAnimation& v);



/*
*
    UIElementが最終的にもつ,複数のアニメーション情報
*
*/
struct UIElementAnimationDatas {

    std::unordered_map<std::string, UIElementAnimation> animations_;

    // 現在選択中
    std::string currentTag_;
};

// 保存 / 読み込み
void to_json(nlohmann::json& j, const UIElementAnimationDatas& v);
void from_json(const nlohmann::json& j, UIElementAnimationDatas& v);
