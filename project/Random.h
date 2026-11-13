#pragma once

#include <random>

// クラスのメンバ
extern std::mt19937 randomEngine;

// min ～ max の乱数を返す
float Random(float min, float max);