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

CollisionResult ResolveAABBCollision(AABB& playerAABB, Vector3& playerPos, const AABB& wallAABB) {

    CollisionResult result;

    // 1. 各軸のめり込み（重なり）量を計算
    // 右方向・左方向の両方の差分を計算
    float overlapX1 = wallAABB.max.x - playerAABB.min.x; // プレイヤーが壁の右からめり込んだ場合
    float overlapX2 = playerAABB.max.x - wallAABB.min.x; // プレイヤーが壁の左からめり込んだ場合

    float overlapY1 = wallAABB.max.y - playerAABB.min.y; // 下から
    float overlapY2 = playerAABB.max.y - wallAABB.min.y; // 上から

    float overlapZ1 = wallAABB.max.z - playerAABB.min.z; // 奥から
    float overlapZ2 = playerAABB.max.z - wallAABB.min.z; // 手前から

    // どれかの軸で重なりが 0 以下なら、衝突していない（離れている）
    if (overlapX1 <= 0 || overlapX2 <= 0 ||
        overlapY1 <= 0 || overlapY2 <= 0 ||
        overlapZ1 <= 0 || overlapZ2 <= 0) {
        return result; // 衝突なし
    }

    // 2. 各軸ごとの「最小めり込み量」と「押し戻し方向（サイン）」を求める
    float pushX = (overlapX1 < overlapX2) ? overlapX1 : -overlapX2;
    float pushY = (overlapY1 < overlapY2) ? overlapY1 : -overlapY2;
    float pushZ = (overlapZ1 < overlapZ2) ? overlapZ1 : -overlapZ2;

    // 絶対値（めり込みの深さ）を取得
    float absX = std::abs(pushX);
    float absY = std::abs(pushY);
    float absZ = std::abs(pushZ);

    // 3. 一番重なりが浅い（絶対値が小さい）軸を選んで押し戻す
    if (absX < absY && absX < absZ) {

        // X軸（左右の壁）で押し戻し
        playerPos.x += pushX;
        result.hitWall = true;

    }
    else if (absY < absZ) {

        // Y軸（床や天井）で押し戻し
        playerPos.y += pushY;

        if (pushY > 0.0f) {
            // プラス方向に押し戻された ＝ プレイヤーの下に床がある！
            result.isGrounded = true;
        }
        else {
            // マイナス方向に押し戻された ＝ プレイヤーの上に天井がある！
            result.hitCeiling = true;
        }

    }
    else {
        // Z軸（前後の壁）で押し戻し
        playerPos.z += pushZ;

        // 前後
        result.hitWall = true;

    }

    // AABB の位置も更新されたプレイヤー座標に合わせる
    // (※AABB のサイズを保持したまま位置だけずらす処理)
    float width = playerAABB.max.x - playerAABB.min.x;
    float height = playerAABB.max.y - playerAABB.min.y;
    float depth = playerAABB.max.z - playerAABB.min.z;

    playerAABB.min = { playerPos.x - width / 2.0f, playerPos.y, playerPos.z - depth / 2.0f };
    playerAABB.max = { playerPos.x + width / 2.0f, playerPos.y + height, playerPos.z + depth / 2.0f };

    return result; // 衝突して押し戻した
}