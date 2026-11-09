#pragma once

#include "Vector.h"

// 平面
struct Plane {

    Vector3 point; // 中心座標
    Vector3 normal{ 0,1.0f,0 }; ; // 法線（向き）

    float distance = 1.0f; // 原点からの距離
};

// 平面(縦横版)
struct PlaneArea {

    Vector3 point; // 中心座標
    Vector3 normal{ 0,1.0f,0 }; // 法線（向き）

    float width = 1.0f;
    float height = 1.0f;
};