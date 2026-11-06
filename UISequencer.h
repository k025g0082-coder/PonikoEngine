#pragma once
#include <cstdint>

#include <vector>
#include <string>

#include "Vector.h"
#include "Easing.h"

#include <nlohmann/json.hpp>

/*
* 
    UIのタイムライン表示
*
*/

enum class SequenceItemType {
    kPosition,
    kScale,
    kRotation,
};
inline const char* kItemTypeTags_[] = {
    "座標",
    "拡大",
    "回転",
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
    シーケンス内のアイテム　（座標,スケール,拡大・・
*/
struct SequenceItem
{
    SequenceItemType mType;       // タイプ？

    std::vector<KeyFrame<Vector3>> keyFrames;  //　キーたち

};

// 保存 / 読み込み
void to_json(nlohmann::json& j, const SequenceItem& v);
void from_json(const nlohmann::json& j, SequenceItem& v);


// シーケンスのトラック
//struct SequenceTrack
//{
//    std::vector<SequenceItem> items;  //　アイテムたち
//
//};


/* 
    UIElementがもつ単体の情報 
*/
struct UIElementAnimationData {

    // 予めSRTの３要素を入れとく
    std::vector<SequenceItem> items = {
        { SequenceItemType::kPosition },
        { SequenceItemType::kScale },
        { SequenceItemType::kRotation },
    };

};

// 保存 / 読み込み
void to_json(nlohmann::json& j, const UIElementAnimationData& v);
void from_json(const nlohmann::json& j, UIElementAnimationData& v);


//
class UISequencer
{
public:

	void Update();
	void Draw();



    /* アイテム新規追加 */
    void AddItem(SequenceItemType type);

    /* キーフレーム挿入 */
    void AddKeyFrame(SequenceItemType type, KeyFrame<Vector3> key);

    // いつかやる
    //void AddTrack()
    

    /* 
        アニメーションデータ登録
    */
    void SetUIElementAnimationData(UIElementAnimationData& data) {

        // アドレス登録
        selectElementData = &data;
    }


    const int32_t& GetCurrentFrame() { return currentFrame; }

private:

    /// <summary>
    /// 現在のフレームからキーフレームで補間する
    /// </summary>
    /// 
    /// <param name="itemIndex">アイテムのIndex番号</param>
    /// <param name="keyIndex">キーのIndex番号</param>
    void LerpKeyFrame(int itemIndex, int keyIndex);


	bool isPlaying = false;

	int32_t currentSelectItemIndex = -1;
    int32_t currentSelectItemKeyIndex = -1;

	int32_t currentFrame = 0;
	float currentFloatFrame = 0;

	int32_t previousFrame = 0;

	int32_t startFrame = -10;
	int32_t endFrame = 64;

    
    UIElementAnimationData* selectElementData = nullptr;


	bool transformOpen = true;

	bool doDelete = false;

};


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
