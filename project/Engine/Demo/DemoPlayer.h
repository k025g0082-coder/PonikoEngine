#pragma once

#include "GameObject.h"
#include "DemoFieldObject.h"
#include "DemoFieldItem.h"

#include "HierarchyPanel.h"

#include "Collision.h"

#include "Particle.h"

#include <vector>

class DemoPlayer : public DemoFieldObject
{
public:

    // ★ コンストラクタをそのまま引き継ぐ
    using DemoFieldObject::DemoFieldObject;

    void Initialize() override;
    void Update() override;
    void Draw() override;

    void Respown();

    // フィールド上の全部と判定
    void CollisionToAllBlock();

    // ***
    void onCollision(DemoFieldObject* other, CollisionResult result) override {

        // プレイヤーに当たったよと言う
        other->onCollisionToPlayer(this,result);

    }

    void onCollisionToGimmick(DemoGimmick* gimmick, CollisionResult result) override;
    void onCollisionToItem(DemoFieldItem* gimmick, CollisionResult result) override;

    void ImGuiView() override;

private:

    // ステージ開始時の座標
    Vector3 beginPosition_;


    Vector3 velocity_;// 移動量

	CollisionResult collisionResult_;// 衝突判定


    // 重力加速度(下方向)
    static inline float kGravityAcceleration = 0.014f;

    // ジャンプ初速(上方向)
    static inline float kJumpAcceleration = 0.3f;


    float emitDistance = 0.05f; // 間隔？
    float emitElapsedTimer = 0.0f;

    // 動いた時の煙
    ParticleParam moveEmitParam;

    // ジャンプ時
    ParticleParam jumpEmitParam;

    // 取得時
    ParticleParam getEmitParam;
};

