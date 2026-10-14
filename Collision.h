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