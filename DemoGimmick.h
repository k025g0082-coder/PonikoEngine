#pragma once

// フィールド上の基底クラス
#include "DemoFieldObject.h"


//== デモのギミックの基底クラス  ==//
class DemoGimmick : public DemoFieldObject {
public:

    // FieldObjectの引数をリレーする
    DemoGimmick(Model* model, Camera* camera, const GameObjectData& data_, nlohmann::json* json)
        : DemoFieldObject(model, camera, data_,json)
    {
        // タイプを設定
        type_ = static_cast<Type>(data_.objectID);
        subType_ = data_.subObjectID;

        // JSONのアドレスを取得
        json_ = json;
    }

    virtual ~DemoGimmick() = default;

    virtual void Initialize() = 0;
    virtual void Update() = 0;
    virtual void Draw() = 0;

    virtual void ImGuiView() = 0;

    /* ギミック種類 */
    enum class GimmickType {

        kSwitch,
        kGoal,

        count
    };

    /* ギミック種類名を取得 */
	static const char* GetGimmickTypeName(GimmickType type) {
		return GimmickTypeName[(int)type];
	}

protected:
    uint32_t gimmickID_ = 0;           // ギミックの識別ID？
private:

    // ギミックの種類名
    static const char* GimmickTypeName[(int)GimmickType::count];
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



    void Initialize() {};
    void Update();
    void Draw();

    void ImGuiView() { DemoFieldObject::ImGuiView(); }

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

    /* 基底クラスと同じ処理をしたい場合・・書かなくてよい */
    void Initialize() {};
    void Update();
    void Draw();

    void ImGuiView() { DemoFieldObject::ImGuiView(); };
private:

    bool isGoal_ = false;

};
