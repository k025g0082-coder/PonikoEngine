#pragma once
#include "Vector.h"
#include <vector>
#include <string>

struct KeyframeVector3
{
	float time;
	Vector3 value;
};
struct KeyframeQuaternion
{
	float time;
	Vector4 value;
};

// ボーン一本分
struct NodeAnimation
{
	std::string boneName;

	std::vector<KeyframeVector3> translations;
	std::vector<KeyframeQuaternion> rotations;
	std::vector<KeyframeVector3> scales;
};

// アニメーション情報
struct Animation
{
	std::string name;

	float duration;

	float ticksPerSecond;// FPS的な

	std::vector<NodeAnimation> nodeAnimations;
};