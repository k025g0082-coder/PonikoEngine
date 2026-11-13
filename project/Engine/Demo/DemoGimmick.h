#pragma once

// フィールド上の基底クラス
#include "DemoFieldObject.h"

#include "DemoPlayer.h"
#include "EasingVec.h"

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


    // ギミックが、他のギミックから発火された際に実行される
    virtual void ByTriggerGimmick(DemoGimmick* gimmick) { gimmick; }


    // ***
    void onCollision(DemoFieldObject* other, CollisionResult result) override {

        // ギミックに当たったよと言う
        other->onCollisionToGimmick(this,result);

    }

    virtual void ImGuiView() = 0;

    /* ギミック種類 */
    enum class GimmickType {

        kSwitch,
        kDoor,
        kGoal,

        count
    };

    /* ギミック種類名を取得 */
	static const char* GetGimmickTypeName(GimmickType type) {
		return GimmickTypeName[(int)type];
	}

protected:

   
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

    // FieldObjectの引数をリレーする
    DemoSwitch(Model* model, Camera* camera, const GameObjectData& data_, nlohmann::json* json)
        : DemoGimmick(model, camera, data_, json)
    {
        // もしなければ新しく作る
        if (!json_->contains("processID")) {
            (*json_)["processID"] = nlohmann::json::array();
        }

        // jsonからprocessIDを取り出す
        for (const auto& item : json_->at("processID")) {

            // 数字だった場合
            if (item.is_number_integer()) {
                processIDs_.push_back(item.get<uint32_t>());
            }

        }
        
    }

    void Initialize() {};
    void Update();
    void Draw();

    void ImGuiView();

    //*** プレイヤにあたった
    void onCollisionToPlayer(DemoPlayer* player, CollisionResult result) override;

    // 追加
    void AddID(uint32_t instanceId) {
        
        //検索して無かったら追加
        auto it = std::find(
            processIDs_.begin(),
            processIDs_.end(),
            instanceId);

        if (it == processIDs_.end()) {
            processIDs_.push_back(instanceId);

            if (json_ != nullptr) {
                (*json_)["processID"].push_back(instanceId); // 単体の ID を追加
            }
        }

        
    }

private:

    bool isPush_ = false;
    bool isProcess_ = false;

    EasingVec pushScaleYEase;

    // 作動させるIDを追加する
    std::vector<uint32_t> processIDs_;

};

/// <summary>
/// ドア
/// </summary>
class DemoDoor : public DemoGimmick {
public:

    // ★ DemoGimmick のコンストラクタをそのまま引き継ぐ
    using DemoGimmick::DemoGimmick;

    /* 基底クラスと同じ処理をしたい場合・・書かなくてよい */
    void Initialize() {};
    void Update();
    void Draw();

    // ギミックが、他のギミックから発火された際に実行される
    void ByTriggerGimmick(DemoGimmick* gimmick)override;


    void ImGuiView() { DemoFieldObject::ImGuiView(); };
private:

    bool isTriggered_ = false;

    EasingVec openPositionEase;
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

    void onCollisionToPlayer(DemoPlayer* player);


    void ImGuiView() { DemoFieldObject::ImGuiView(); };
private:

    bool isGoal_ = false;

};