#pragma once
#include "AABB.h"
#include "LineSegment.h"
#include "Sphere.h"

// 当たり判定クラス

// 箱同士AABBの衝突
bool IsCollision(const AABB& aabb1, const AABB& aabb2);

// 球同士Sphereの衝突
bool IsCollision(const Sphere& s1, const Sphere& s2);

bool IsCollision(const LineSegment& line, const Sphere& sphere);
bool IsCollision(const LineSegment& line, const AABB& aabb);


// 衝突した面を特定するためのフラグ構造体
struct CollisionResult {
    bool isCollision = false; // 当たった
    bool isGrounded = false; // 床（下）に当たった
    bool hitCeiling = false; // 天井（上）に当たった
    bool hitWall = false; // 横（壁）に当たった
};

/// <summary>
/// AABB同士の衝突判定と押し戻し処理
/// </summary>
/// <param name="aabb">対象のAABB</param>
/// <param name="position">対象の中心座標</param>
/// <param name="targetAABB">ターゲットのAABB</param>
/// <param name="isPushBack">押し戻すか</param>
/// <returns></returns>
CollisionResult ResolveAABBCollision(AABB& aabb, Vector3& position, const AABB& targetAABB, const bool& isPushBack = false);