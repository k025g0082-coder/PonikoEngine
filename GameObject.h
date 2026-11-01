#pragma once

#include "Model.h"
#include "Camera.h"

#include "AABB.h"

#include "WorldTransform.h"

#include "HierarchyPanel.h"

/* ゲームのオブジェクトの基本データ（初期化時などに値を引き渡す用） */
struct GameObjectData {

    Model* model = nullptr;

    Vector3 position;
    Vector3 rotation = { 0, 0, 0 };
    Vector3 scale = { 1, 1, 1 };

    AABB aabb_;// 判定用

    uint32_t objectID = 0;    // 識別ID
    uint32_t subObjectID = 0; // サブ識別ID

	Hierarchy<Model>* hierarchy = nullptr; // 連携してるヒエラルキのポインタ

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
    const AABB& GetAABB() const { return aabb_; }
	const Hierarchy<Model>* GetHierarchy() const { return hierarchy; }

    /* === 派生クラスで引き継げる機能 === */

    // 他と接触があった場合
    virtual void onCollision(GameObject* other) {};

protected:

    Model* model_ = nullptr;
    Camera* camera_ = nullptr;

	Hierarchy<Model>* hierarchy = nullptr; // 連携してるヒエラルキのポインタ

    WorldTransform worldTransform;// WorldTransform情報

    uint32_t objectID_ = 0;           // オブジェクトの個別ID？

    AABB aabb_;

private:

    uint32_t countId_ = 0;
};

