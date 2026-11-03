#pragma once

#include "GameObject.h"
#include "DemoFieldObject.h"

#include "HierarchyPanel.h"

#include "Collision.h"

#include <vector>

class DemoPlayer : public DemoFieldObject
{
public:

    // ★ コンストラクタをそのまま引き継ぐ
    using DemoFieldObject::DemoFieldObject;

    void Initialize() override;
    void Update() override;
    void Draw() override;

    // フィールド上の全部と判定
    void CollisionToAllBlock();

    // ***
    void onCollision(DemoFieldObject* other, CollisionResult result) override {

        // プレイヤーに当たったよと言う
        other->onCollisionToPlayer(this,result);

    }

    void onCollisionToGimmick(DemoGimmick* gimmick, CollisionResult result) override;

    void ImGuiView() override;

	void SetStageHierarchyList(std::vector<Hierarchy<Model>*>* list) {
        stageHierarchyList_ = list; 
    }

private:

    Vector3 velocity_;// 移動量

	CollisionResult collisionResult_;// 衝突判定


    // 重力加速度(下方向)
    static inline float kGravityAcceleration = 0.014f;

    // ジャンプ初速(上方向)
    static inline float kJumpAcceleration = 0.3f;

    /* ステージ内の連携しているヒエラルキのリストのぽいんた */
    std::vector<Hierarchy<Model>*>* stageHierarchyList_;
};

