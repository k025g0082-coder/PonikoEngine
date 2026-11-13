#pragma once
#include "DemoPlayer.h"
#include "Model.h"
#include "Vector.h"
#include "WorldTransform.h"

#include "AABB.h"

#include "Particle.h"

/// <summary>
/// 隕石
/// </summary>
class DemoFieldMeteo{
public:

    void Initialize(Model* model, Vector3 pos , ParticleParam& meteoEmitParam, ParticleParam& meteoBreakEmitParam);
    void Update();
    void Draw();

    // 破壊
    void Break();

    const AABB& GetAABB() const { return aabb_; }
    const bool& IsBreak() const { return isBreak_; }
    const bool& IsDelete() const { return isDelete_; }

    // 隕石速度
    static Vector3 velocity;


   

private:

    bool isDelete_ = false;

    bool isBreak_ = false;

    float kLimit = 3.0f;

    Model* model_ = nullptr;
    WorldTransform world_;


    // 落下中
    ParticleParam* meteoEmitParam_ = nullptr;

    // 落下時
    ParticleParam* meteoBreakEmitParam_ = nullptr;


    AABB aabb_;

    float emitElapsedTimer = 0.0f;

    float timer_ = 0;

};
