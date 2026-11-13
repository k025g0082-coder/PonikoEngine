#include "Random.h"

// クラスのメンバ
std::mt19937 randomEngine_{ std::random_device{}() };

// min ～ max の乱数を返す
float Random(float min, float max) {
	std::uniform_real_distribution<float> dist(min, max);
	return dist(randomEngine_);
}