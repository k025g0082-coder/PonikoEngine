#include "Collision.h"
#include "Matrix.h"

// 箱同士AABBの衝突
bool IsCollision(const AABB& aabb1, const AABB& aabb2) {
	if ((aabb1.min.x <= aabb2.max.x && aabb1.max.x >= aabb2.min.x) && // x#
		(aabb1.min.y <= aabb2.max.y && aabb1.max.y >= aabb2.min.y) && // y#
		(aabb1.min.z <= aabb2.max.z && aabb1.max.z >= aabb2.min.z)) { // z#
		// 衝突
		return true;
	}
	return false;
};


bool IsCollision(const Sphere& s1, const Sphere& s2) {

	//2つの球の中心点間の距離を求める
	float distance = Length(Subtract(s2.center, s1.center));

	//半径の合計よりも短ければ衝突
	if (distance <= s1.radius + s2.radius) {
		//当たった処理を諸々
		return true;
	}

	return false;
};

bool IsCollision(const LineSegment& line, const Sphere& sphere) {

    // 線分の方向と長さの2乗
    Vector3 direction = line.end - line.origin;

    float lengthSq =
        direction.x * direction.x +
        direction.y * direction.y +
        direction.z * direction.z;

    // 線分の長さが0の場合
    if (lengthSq < 1e-6f) {
        Vector3 diff = line.origin - sphere.center;

        return diff.x * diff.x +
            diff.y * diff.y +
            diff.z * diff.z
            <= sphere.radius * sphere.radius;
    }

    // 線分の始点から球の中心へのベクトル
    Vector3 toCenter = sphere.center - line.origin;

    // 最近接点を求める係数
    float t = (
        toCenter.x * direction.x +
        toCenter.y * direction.y +
        toCenter.z * direction.z
        ) / lengthSq;

    // 線分の範囲内に制限
    t = (std::max)(0.0f, (std::min)(1.0f, t));

    // 線分上の最近接点
    Vector3 closest = line.origin + direction * t;

    // 最近接点と球の中心との距離
    Vector3 diff = closest - sphere.center;

    float distanceSq =
        diff.x * diff.x +
        diff.y * diff.y +
        diff.z * diff.z;

    return distanceSq <= sphere.radius * sphere.radius;
}

bool IsCollision(const LineSegment& line, const AABB& aabb) {

    Vector3 direction = line.end - line.origin;

    // 線分上の範囲 0～1
    float tMin = 0.0f;
    float tMax = 1.0f;

    // 各軸を順番に判定
    for (int axis = 0; axis < 3; ++axis) {

        float origin;
        float directionAxis;
        float min;
        float max;

        if (axis == 0) {
            origin = line.origin.x;
            directionAxis = direction.x;
            min = aabb.min.x;
            max = aabb.max.x;
        }
        else if (axis == 1) {
            origin = line.origin.y;
            directionAxis = direction.y;
            min = aabb.min.y;
            max = aabb.max.y;
        }
        else {
            origin = line.origin.z;
            directionAxis = direction.z;
            min = aabb.min.z;
            max = aabb.max.z;
        }

        // その軸に平行な場合
        if (std::abs(directionAxis) < 1e-6f) {
            if (origin < min || origin > max) {
                return false;
            }

            continue;
        }

        // AABBの面と交差する位置
        float t1 = (min - origin) / directionAxis;
        float t2 = (max - origin) / directionAxis;

        if (t1 > t2) {
            std::swap(t1, t2);
        }

        // 交差範囲を絞る
        tMin = (std::max)(tMin, t1);
        tMax = (std::min)(tMax, t2);

        if (tMin > tMax) {
            return false;
        }
    }

    return true;
}