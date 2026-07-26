#pragma once
#include "Matrix.h"
#include <string>
#include "DirectXUtility.h"

// ボーン
struct Bone {
	std::string name;

	int parentIndex = -1;
	std::vector<uint32_t> children;// 子供の番号

	Matrix4x4 offsetMatrix;

	Matrix4x4 localBindMatrix; // アニメしてない初期姿勢の行列　初期値？

	Matrix4x4 localMatrix; // SRTから作られる行列
	Matrix4x4 globalMatrix;// 親Boneから計算される

	Matrix4x4 finalMatrix;// 親子関係などの計算を経て、最終的にシェーダーに送る行列
};

// モデルー＞ボーン空間に変換する行列
struct BoneInfo {
	Matrix4x4 offsetMatrix;
	Matrix4x4 finalTransform;
};

// ボーン情報
struct VertexBoneWeightData {

	// ひとモデル最大骨４本
	static const uint32_t kMaxBoneInfluence = 4;

	// 骨管理番号
	uint32_t boneIndex[kMaxBoneInfluence] = {};

	// 重み
	float weight[kMaxBoneInfluence] = {};

	// 骨を登録？
	void AddBoneData(uint32_t index, float w) {

		DirectXUtility::Log("AddBoneData index=%u weight=%f\n", index, w);

		for (uint32_t i = 0; i < kMaxBoneInfluence; i++) {
			if (weight[i] == 0.0f) {
				boneIndex[i] = index;
				weight[i] = w;
				return;
			}
		}
	}
};
