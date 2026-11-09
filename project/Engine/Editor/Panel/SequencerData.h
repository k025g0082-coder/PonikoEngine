#pragma once
#include "Vector.h"
#include "Easing.h"

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

enum class SequenceItemType {

    /*
        Transform
    */
    kPosition,
    kScale,
    kRotation,

    /*
        Alpha
    */
    kAlpha
};

template <typename T>

/*
    キーフレーム
*/
struct KeyFrame {

    int32_t frame;             // 時間
    T value;                // 値　floatやVector3など
    EasingType easingType; // 補間タイプ


    // 保存 / 読み込み
    static nlohmann::json ToJson(KeyFrame key);
    static KeyFrame FromJson(const nlohmann::json& data);

};


/*
    シーケンス内のアイテム　（Transform：座標,スケール,拡大・・
*/
struct SequenceItem
{
    SequenceItemType type;//  タイプ
    std::string name;//  アイテム名

    std::vector<KeyFrame<Vector3>> keyFrames;  //　キーたち

};

// 保存 / 読み込み
void to_json(nlohmann::json& j, const SequenceItem& v);
void from_json(const nlohmann::json& j, SequenceItem& v);


/*
    シーケンス内のトラック　（Transform, Color)  ・・・
*/
struct SequenceTrack
{
    std::string trackName;            //  トラック名
    std::vector<SequenceItem> items;  //　アイテムたち

    bool isOpen = true; // 開いているか
};

// 保存 / 読み込み
void to_json(nlohmann::json& j, const SequenceTrack& v);
void from_json(const nlohmann::json& j, SequenceTrack& v);




/*
    キー保存
*/
template<typename T>
inline nlohmann::json KeyFrame<T>::ToJson(KeyFrame key)
{
    nlohmann::json data;

    data["frame"] = key.frame;
    data["value"] = key.value;
    data["easingType"] = (int)key.easingType;

    return data;
}

/*
    キー読み取り
*/
template<typename T>
KeyFrame<T> KeyFrame<T>::FromJson(const nlohmann::json& data)
{
    KeyFrame<T> key;

    key.frame = data.at("frame").get<int32_t>();
    key.value = data.at("value").get<T>();
    key.easingType = (EasingType)data.at("easingType").get<int>();

    return key;
}
