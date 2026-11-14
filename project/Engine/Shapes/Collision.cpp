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

bool IsCollision(const Sphere& sphere, const AABB& aabb)
{
    // AABBの最近接点を使って判定
    Vector3 closestPoint = {
       std::clamp(sphere.center.x, aabb.min.x, aabb.max.x),
       std::clamp(sphere.center.y, aabb.min.y, aabb.max.y),
       std::clamp(sphere.center.z, aabb.min.z, aabb.max.z)
    };

    return Length(closestPoint - sphere.center) <= sphere.radius;
}

CollisionResult ResolveAABBCollision(AABB& playerAABB, Vector3& playerPos, const AABB& wallAABB, const bool& isPushBack) {

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
        if (isPushBack)
            playerPos.x += pushX;

        result.hitWall = true;

    }
    else if (absY < absZ) {

        // Y軸（床や天井）で押し戻し
        if (isPushBack)
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
        if (isPushBack)
            playerPos.z += pushZ;

        // 前後
        result.hitWall = true;

    }

    // 衝突しました
    result.isCollision = true;

    if (!isPushBack) return result;


    // AABB の位置も更新されたプレイヤー座標に合わせる
    // (※AABB のサイズを保持したまま位置だけずらす処理)
    float width = playerAABB.max.x - playerAABB.min.x;
    float height = playerAABB.max.y - playerAABB.min.y;
    float depth = playerAABB.max.z - playerAABB.min.z;

    playerAABB.min = { playerPos.x - width / 2.0f, playerPos.y, playerPos.z - depth / 2.0f };
    playerAABB.max = { playerPos.x + width / 2.0f, playerPos.y + height, playerPos.z + depth / 2.0f };

    return result; // 衝突して押し戻した
}


bool IntersectRayAABB(
    const Ray& ray,
    const AABB& aabb,
    float& distance)
{
    float tMin = 0.0f;
    float tMax = FLT_MAX;

    const float origin[3] = {
        ray.origin.x, ray.origin.y, ray.origin.z
    };
    const float dir[3] = {
        ray.direction.x, ray.direction.y, ray.direction.z
    };
    const float boxMin[3] = {
        aabb.min.x, aabb.min.y, aabb.min.z
    };
    const float boxMax[3] = {
        aabb.max.x, aabb.max.y, aabb.max.z
    };

    for (int i = 0; i < 3; ++i) {
        if (std::abs(dir[i]) < 1.0e-6f) {
            // この軸にほぼ平行
            if (origin[i] < boxMin[i] ||
                origin[i] > boxMax[i]) {
                return false;
            }
        }
        else {
            float t1 = (boxMin[i] - origin[i]) / dir[i];
            float t2 = (boxMax[i] - origin[i]) / dir[i];

            if (t1 > t2) {
                std::swap(t1, t2);
            }

            tMin = (std::max)(tMin, t1);
            tMax = (std::min)(tMax, t2);

            if (tMin > tMax) {
                return false;
            }
        }
    }

    distance = tMin;
    return true;
}

std::optional<Vector3> IntersectRayPlane(const Ray& ray, const Plane& plane)
{
    /*

        レイトレーシング（光線追跡法）
        直線のベクトル方程式（レイ・放射線の数式）

        P(t) = O + tD

        P(t)　当たった座標
        O   plane原点
        t   原点Oからどれだけ進んだか、距離
        D   Rayの方向。正規化して使う

    */

    // 平面の法線
    Vector3 normal = Normalize(plane.normal);
    Vector3 rayDirection = Normalize(ray.direction);

    // レイの方向と法線の内積
    float denominator = Dot(rayDirection, normal);

    // 0除算対策
    if (std::abs(denominator) < 0.0001f) {
        return std::nullopt;
    }

    // レイの始点から平面までの距離を求める
    float t = Dot(
        plane.point - ray.origin,
        normal
    ) / denominator;

    // レイの後方に交点がある
    if (t < 0.0f) {
        return std::nullopt;
    }

    /* 公式に則って当たった座標を返す */
    return ray.origin + t * rayDirection;
}

std::optional<Vector3> IntersectRayPlaneArea(const Ray& ray, const PlaneArea& planeArea)
{
    /*

        当たった座標から２軸を投影して範囲内ならあたった判定らしい

    */
    // 平面の法線
    Vector3 normal = Normalize(planeArea.normal);
    Vector3 rayDirection = Normalize(ray.direction);

    // レイの方向と法線の内積
    float denominator = Dot(rayDirection, normal);

    // 0除算対策
    if (std::abs(denominator) < 0.0001f) {
        return std::nullopt;
    }

    // レイの始点から平面までの距離を求める
    float t = Dot(
        planeArea.point - ray.origin,
        normal
    ) / denominator;

    // レイの後方に交点がある
    if (t < 0.0f) {
        return std::nullopt;
    }

    // 交点
    Vector3 hitPoint = ray.origin + t * rayDirection;


    // 平面中心から交点へのベクトル
    Vector3 offset = hitPoint - planeArea.point;

    // ２軸
    Vector3 axis1 = Normalize(Cross(
        Vector3{ 0.0f, 0.0f, 1.0f },
        normal
    ));
    Vector3 axis2 = Normalize(Cross(normal, axis1));

    // 平面の横・縦方向に投影
    float x = Dot(offset, axis1);
    float y = Dot(offset, axis2);

    // 範囲外なら交差なし
    if (std::abs(x) > planeArea.width * 0.5f ||
        std::abs(y) > planeArea.height * 0.5f) {
        return std::nullopt;
    }

    return hitPoint;
}