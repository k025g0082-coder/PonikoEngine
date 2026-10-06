#pragma once
#include "Vector.h"

struct AABB {

	Vector3 min = { 0,0,1.0f }; // 最小点
	Vector3 max = { 1.0f,1.0f,1.0f }; // 最大点
	
	// デバッグ用
	void ImGuiView();
};


// 箱同士AABBの衝突
bool IsCollision(const AABB& aabb1, const AABB& aabb2);