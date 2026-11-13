#pragma once
#include "Vector.h"

struct AABB {

	Vector3 min = { 0,0,1.0f }; // 最小点
	Vector3 max = { 1.0f,1.0f,1.0f }; // 最大点

	Vector3 offset; // オフセット

	/* 縦横幅 */
	float width_  = 1.0f;
	float height_ = 1.0f;
	float depth_  = 1.0f;

	Vector3 GetCenter() const {
		return (min + max) * 0.5f;
	}

	// 縦横幅から自動計算
	void UpdatePosition(const Vector3& position);

	// デバッグ用
	void ImGuiView(bool viewMinMax = true);
};
