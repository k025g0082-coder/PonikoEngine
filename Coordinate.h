#pragma once

// 座標系をまとめた

#include "Vector.h"

// 球面座標
// 用途・・原点中心にぐるぐる回すなど

struct Spherical {
	float radius; // 動径
	float theta;  // 仰角
	float phi;    // 方位角
};

// 球面座標→直交座標　の変換
Vector3 ToCartesian(const Spherical& s);

// 直交座標→球面座標　の変換
Spherical ToSpherical(const Vector3& p);