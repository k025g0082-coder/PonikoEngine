
#pragma once
#include "Vector.h"

struct Ray {
	Vector3 origin;    // レイの始点
	Vector3 direction; // 正規化した方向
};
