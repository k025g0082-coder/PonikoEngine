#pragma once
#include "Sprite.h"
#include "Vector.h"
#include "Easing.h"

#include "SequencerData.h"

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



enum class UIType {
    kNone,
    kButton,
};

struct UIElement {

    UIType type_ = UIType::kNone;  //	タイプ
    Sprite sprite_;				   //	スプライト

    std::string pathID_;			   //   アセット参照用

    std::string name_;                 //   エレメントの名前。
    std::string groupName_ = "Main";   //   グループの名前。

    UIElementAnimationDatas animationDatas_; // アニメーションのデータ？

    // SRT
    Vector3 position_;
    Vector3 scale_;
    Vector3 rotation_;


    // 親のアドレス
    UIElement* parent_ = nullptr;

    // 親の名前(データ輸送用,実際の表示には使わない)
    std::string parentName_;

};

/* 一つのキャンバス内全てのUIデータ */
struct UIData {

    // 名前　キャンバスレイヤーの名前からもらう（予定）
    std::string name_;

    // 表示非表示
    bool isView_ = true;

    // UIElementたち
    std::vector<UIElement> elements_;

};
