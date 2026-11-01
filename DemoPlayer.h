#pragma once

#include "GameObject.h"
#include "HierarchyPanel.h"

#include "Collision.h"

#include <vector>

class DemoPlayer : public GameObject
{
public:

    // ★ GameObject のコンストラクタをそのまま引き継ぐ
    using GameObject::GameObject;

    void Update() override;
    void Draw() override;

    void ImGuiView() override;

	void SetStageHierarchyList(std::vector<Hierarchy<Model>*>* list) {
        stageHierarchyList_ = list; 
    }

private:

    Vector3 velocity_;// 移動量

	CollisionResult collisionResult_;// 衝突判定


    // 重力加速度(下方向)
    static inline float kGravityAcceleration = 0.015f;

    // ジャンプ初速(上方向)
    static inline float kJumpAcceleration = 0.3f;

    /* ステージ内の連携しているヒエラルキのリストのぽいんた */
    std::vector<Hierarchy<Model>*>* stageHierarchyList_;
};

