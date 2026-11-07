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

    bool isOpen = false; // 開いているか
};

// 保存 / 読み込み
void to_json(nlohmann::json& j, const SequenceTrack& v);
void from_json(const nlohmann::json& j, SequenceTrack& v);



/*
* 
* 
    アニメ
*
*
*/
struct UIElementAnimation {
    
    // アニメ名
    std::string animationName = "NewAnimetesti";

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



/*


    こっからクラス

*/
class UISequencer
{
public:

	void Update();
	void Draw();



    /* アイテム新規追加 */
    //void AddItem(SequenceItemType type);

    /* キーフレーム挿入 */
    void AddKeyFrame(SequenceItemType type, KeyFrame<Vector3> key);

    // いつかやる
    //void AddTrack()
    

    /*
        デフォルトの設定をしたElementAnimationDataを返す
    */
    static void SetDefaultTrackDatas(UIElementAnimation& anime);


    /* 
        アニメーションデータ登録
    */
    void SetUIElementAnimationData(UIElementAnimationDatas& data) {

        // アドレス登録
        selectElementData = &data;
    }


    // 
    void AnimationComboView();


    const int32_t& GetCurrentFrame() { return currentFrame; }

private:

    /// <summary>
    /// 現在のフレームからキーフレームで補間する
    /// </summary>
    /// 
    /// <param name="trackIndex">トラックのIndex番号</param>
    /// <param name="itemIndex">アイテムのIndex番号</param>
    /// <param name="keyIndex">キーのIndex番号</param>
    void LerpKeyFrame(int trackIndex,int itemIndex, int keyIndex);


	bool isPlaying = false;


    std::string currentSelectAnimationTag;


	int32_t currentSelectTrackIndex = -1;
	int32_t currentSelectItemIndex = -1;
    int32_t currentSelectItemKeyIndex = -1;

	int32_t currentFrame = 0;
	float currentFloatFrame = 0;

	int32_t previousFrame = 0;

	int32_t startFrame = -10;
	int32_t endFrame = 64;

    
    UIElementAnimationDatas* selectElementData = nullptr;


//	bool transformOpen = true;

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
