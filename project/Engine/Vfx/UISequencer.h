#pragma once
#include <cstdint>

#include <vector>
#include <string>

#include "Vector.h"
#include "Easing.h"

//
#include "UIData.h"
#include "SequencerData.h"


#include <nlohmann/json.hpp>

/*
* 
    UIのタイムライン表示
*
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
    void SetUIElementAnimationData(UIElement& data) {

        // アドレス登録
        selectElement = &data;
    }


    /// <summary>
    /// 現在のフレームからキーフレームで補間する
    /// </summary>
    /// 
    /// <param name="animeDatas">アニメのデータたち</param>
    /// <param name="animationFramee">アニメの再生フレーム時間</param>
    /// <param name="trackIndex">トラックのIndex番号</param>
    /// <param name="itemIndex">アイテムのIndex番号</param>
    /// <param name="keyIndex">キーのIndex番号</param>
    static void LerpKeyFrame(
        UIElement* element,
        float animationFrame,
        int trackIndex,
        int itemIndex,
        int keyIndex
    );


    // 
    void AnimationComboView();


    const int32_t& GetCurrentFrame() { return currentFrame; }

private:

   


	bool isPlaying = false;


    std::string currentSelectAnimationTag;


	int32_t currentSelectTrackIndex = 0;
	int32_t currentSelectItemIndex = -1;
    int32_t currentSelectItemKeyIndex = -1;

	int32_t currentFrame = 0;
	float currentFloatFrame = 0;

	int32_t previousFrame = 0;

	int32_t startFrame = 0;
	int32_t endFrame = 64;

    
    UIElement* selectElement = nullptr;


//	bool transformOpen = true;

	bool doDelete = false;

};

