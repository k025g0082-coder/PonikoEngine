#pragma once

// フィールド上の基底クラス
#include "DemoFieldObject.h"


//== デモのギミックの基底クラス  ==//
class DemoGimmick : public DemoFieldObject{
public:

    // FieldObjectの引数をリレーする
    DemoGimmick(Model* model, Camera* camera, const FieldObjectData& data_)
        : DemoFieldObject(model, camera, data_)
    {
        // なにかあれば
    }

    virtual ~DemoGimmick() = default;

    virtual void Initialize() = 0;
    virtual void Update() = 0;
    virtual void Draw() = 0;

    virtual void ImGuiView() = 0;

    /* ギミック種類 */
    enum class GimmickType {
        kSwitch,
        kGoal
    };

protected:
    uint32_t gimmickID_ = 0;           // ギミックの識別ID？
};


/// <summary>
/// ギミックのスイッチ。
/// 
/// 必要：押すと対応するID？
/// 
/// </summary>
class DemoSwitch : public DemoGimmick {
public:

    // ★ DemoGimmick のコンストラクタをそのまま引き継ぐ
    using DemoGimmick::DemoGimmick;

    void ImGuiView() override;

    //void Initialize() override;
    //void Update() override;
    //void Draw() override;



    // 追加
    void AddID(uint32_t instanceId) {
        
        //重複はめんどいからあと
        processIDs_.push_back(instanceId);
    }
private:

    bool isPush_ = false;

    // 作動させるIDを追加する
    std::vector<uint32_t> processIDs_;

};

/// <summary>
/// ゴール
/// </summary>
class DemoGoal : public DemoGimmick {
public:

    // ★ DemoGimmick のコンストラクタをそのまま引き継ぐ
    using DemoGimmick::DemoGimmick;


    void ImGuiView() override;

    /* 基底クラスと同じ処理をしたい場合・・書かなくてよい */
       // void Initialize() override;
        //void Update() override;
    //void Draw() override;

private:

    bool isGoal_ = false;

};
