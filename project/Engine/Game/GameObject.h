#pragma once

#include "Model.h"
#include "Camera.h"

#include "AABB.h"

#include "WorldTransform.h"

#include "HierarchyPanel.h"
#include "HierarchyNode.h"

/* ゲームのオブジェクトの基本データ（初期化時などに値を引き渡す用） */
struct GameObjectData {

    Model* model = nullptr;

    Vector3 position;
    Vector3 rotation = { 0, 0, 0 };
    Vector3 scale = { 1, 1, 1 };

    AABB aabb_;// 判定用

    uint32_t objectID = 0;    // 識別ID
    uint32_t subObjectID = 0; // サブ識別ID

	HierarchyNode* hierarchy = nullptr; // 連携してるヒエラルキのポインタ

};

/// <summary>
/// ゲーム上のオブジェクトとしての基底クラス
/// </summary>
/// 
class GameObject
{
public:
    
    GameObject( Model* model,Camera* camera, const GameObjectData& data_);
    virtual ~GameObject() = default;

    virtual void Update() = 0;
    virtual void Draw() = 0;

    // ImGuiとして調整できる画面
    virtual void ImGuiView() = 0;

    WorldTransform& GetWorldTransform(){ return worldTransform; } 

    void SetAABB(const AABB& aabb) { aabb_ = aabb; }
    AABB& GetAABB() { return aabb_; }
   
	const HierarchyNode* GetHierarchy() const { return hierarchy; }

    const uint32_t& GetId() const { return objectID_; }
    const uint32_t& GetSubId() const { return objectSubID_; }

    void SetView(bool b) { isView_ = b; }


    // ヒエラルキーモデルからゲームデータに変換
    static GameObjectData ConvertGameData(ModelNode& hierarchy);


    /* === 派生クラスで引き継げる機能 === */

    // 他と接触があった場合
    virtual void onCollision(GameObject* other) {};


    // 接触があったオブジェクトのOnCollisionを呼ぶ
    static void Collision(GameObject* a, GameObject* b);

 
protected:

    Model* model_ = nullptr;
    Camera* camera_ = nullptr;

    HierarchyNode* hierarchy = nullptr; // 連携してるヒエラルキのポインタ

    WorldTransform worldTransform;// WorldTransform情報

    uint32_t objectID_    = 0;           // オブジェクトの識別ID？
    uint32_t objectSubID_ = 0;           // オブジェクトのサブ識別ID？

    AABB aabb_;

    /* レイヤー表示非表示切り替え */
    bool isView_ = true;

private:

    uint32_t countId_ = 0;
};

