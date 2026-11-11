#pragma once

#include <nlohmann/json.hpp>

#include "GameObject.h"

#include "Model.h"
#include "Camera.h"

#include "Collision.h"

#include "WorldTransform.h"

class DemoPlayer;
class DemoGimmick;
class DemoFieldItem;

/// <summary>
/// フィールド上のオブジェクトとしての基底クラス
/// </summary>
/// 
static uint32_t nextUniqueID_ = 1;

class DemoFieldObject : public GameObject{
public:

    /* オブジェクト種類 */
    enum class Type {

        Null,// 無

        kPlayer,
        kBlock,
        kGimmick,
        kItem,

        count
    };

    enum class BlockType {

        kNormal
    };

    // 必要な情報だけGameObjectに渡す
    DemoFieldObject(Model* model, Camera* camera, const GameObjectData& data_, nlohmann::json* json)
        : GameObject(model, camera, data_)
    {
        // タイプを設定
        type_ = static_cast<Type>(data_.objectID);
		subType_ = data_.subObjectID;

        // JSONのアドレスを取得
        json_ = json;

        // ユニークIDを識別
        if (json_ != nullptr) {
            if (json_->contains("uniqueID")) {
                
                // 存在すれば読込
                uniqueID_ = (*json_)["uniqueID"].get<uint32_t>();

                // カウンターの重複のために読み込んだIDより大きいIDに設定し続けておく
                if (uniqueID_ >= nextUniqueID_) {
                    nextUniqueID_ = uniqueID_ + 1;
                }

            }
            else {

                // エディタ等で新しく定義された場合
                uniqueID_ = nextUniqueID_++;
                (*json_)["uniqueID"] = uniqueID_;

            }
        }

    };

    virtual ~DemoFieldObject() = default;

    virtual void Initialize() = 0;//
    virtual void Update() = 0;
    virtual void Draw() = 0;

    // 当たったか判定関数
    static void Collision(DemoFieldObject* a, DemoFieldObject* b);
   

    /// <summary>
    /// 衝突時のエントリーポイント。　ここでother->onCollisionTo.. 別途実装する
    /// </summary>
    /// <param name="other">当たったオブジェクト</param>
    /// <param name="result">判定結果</param>
    virtual void onCollision(DemoFieldObject* other, CollisionResult result) = 0;

    /*
        === 継承された型にあった判定の関数 ===
    */
    // 　ダブルディスパッチ（Visitorパターン） OOP設計　というらしい。　
    //   *このクラスを継承した子は、下の当たった相手の方のplayer,gimmickなどの関数を選んでそのクラスで実行できるので、という
    virtual void onCollisionToPlayer(DemoPlayer* player, CollisionResult result)           { /* プレイヤーに当たった時 */ };
    virtual void onCollisionToGimmick(DemoGimmick* gimmick, CollisionResult result) { /* ギミックに当たった時. */ };
    virtual void onCollisionToItem(DemoFieldItem* gimmick, CollisionResult result) { /* アイテムに当たった時. */ };

    //
    
    //virtual void onCollisionToFieldObject(DemoFieldObject* other)  { /* フィールドブロックに当たった時 */ };

    // GameObject) これももちろん
    virtual void ImGuiView() = 0;


    /* ImGuiウインドウ開始*/
    void BeginImGui();

    /* ImGuiウインドウ終了 */
    void EndImGui();

    // JSON更新
    void UpdateObjectTypeJson();


    const bool& IsDelete()  const { return isDelete; }


    const uint32_t& GetUniqueID() const { return uniqueID_; }


    /* ギミック種類名を取得 */
    static const char* GetFieldObjectTypeName(Type type) {
        return fieldObjectTypeName[(int)type];
    }

protected:

    // フィールドオブジェクトの種類名
    static const char* fieldObjectTypeName[(int)Type::count];



    // ブロックのように当たり判定の際押し戻すか
    bool isCollisionBlock_ = true;


    // 識別用タイプ
    Type type_;

    // 識別２．ギミックの種類など
    uint32_t subType_;

    // fieldObjectのインスタンス識別ID
    uint32_t uniqueID_ = 0;   


    // 削除フラグ
    bool isDelete = false;


    /* 
        ヒエラルキーのJSONのアドレス。データ管理よう。神.
    */
    nlohmann::json* json_ = nullptr;


private:
    bool showImGuiView = false;
};


/* 
    ブロックなど静的なもの
*/
class DemoBlock : public DemoFieldObject {
public:
    
     using DemoFieldObject::DemoFieldObject;

     void Initialize();
     void Update();
     void Draw();

     //
     void onCollision(DemoFieldObject* other, CollisionResult result) override {
         (void)other;
         (void)result;
     }

    // GameObject) これももちろん
     void ImGuiView();

private:
};