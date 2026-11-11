#pragma once
#include "DemoFieldObject.h"

//== デモのアイテムの基底クラス  ==//
class DemoFieldItem : public DemoFieldObject {
public:

    // FieldObjectの引数をリレーする
    DemoFieldItem(Model* model, Camera* camera, const GameObjectData& data_, nlohmann::json* json)
        : DemoFieldObject(model, camera, data_, json)
    {
        // タイプを設定
        type_ = static_cast<Type>(data_.objectID);
        subType_ = data_.subObjectID;

        // JSONのアドレスを取得
        json_ = json;
    }

    virtual ~DemoFieldItem() = default;

    virtual void Initialize() = 0;
    virtual void Update() = 0;
    virtual void Draw() = 0;


    // ***
    void onCollision(DemoFieldObject* other, CollisionResult result) override {

        // アイテムに当たったよと言う
        other->onCollisionToItem(this, result);

    }

    virtual void ImGuiView() = 0;

    /* アイテム種類 */
    enum class ItemType {

        kCoin,

        count
    };

    /* ギミック種類名を取得 */
    static const char* GetItemTypeName(ItemType type) {
        return ItemTypeName[(int)type];
    }

protected:


private:

    // アイテムの種類名
    static const char* ItemTypeName[(int)ItemType::count];
};



/// <summary>
/// コイン
/// </summary>
class DemoCoin : public DemoFieldItem {
public:

    // ★ DemoGimmick のコンストラクタをそのまま引き継ぐ
    using DemoFieldItem::DemoFieldItem;

    /* 基底クラスと同じ処理をしたい場合・・書かなくてよい */
    void Initialize();
    void Update();
    void Draw();


    // プレイヤーに当たった時
    void onCollisionToPlayer(DemoPlayer* player, CollisionResult result);


    void ImGuiView() { DemoFieldObject::ImGuiView(); };
private:

    bool isGoal_ = false;

};
