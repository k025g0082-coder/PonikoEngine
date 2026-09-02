#pragma once
#define _USE_MATH_DEFINES
#include <math.h>

#include "Vector.h"

#include <algorithm>

class EasingVec
{
public:

	void Start(int t, float total, float start, float end);
	void Start(int t, float total, Vector2 startPos, Vector2 endPos);
	void Start(int t, float total, Vector3 startPos, Vector3 endPos);
	void Step();
	void Reset();
	float getEasingTime();
	float getTime() { return time; };

	float getEasingNum();

	Vector2 getEasingVec2();
	Vector3 getEasingVec3();

	bool getIsEasing() { return isEasing; };
	bool getIsFinished() { return isFinished; };

	enum easingType {
		EASE_None,
		EASE_InSine,
		EASE_OutSine,
		EASE_InOutSine,
		EASE_InOutExpo,
		EASE_InBack,
		EASE_OutBack,
		EASE_OutBounce,
		EASE_BouncyMountain,
		EASE_InExpo,
		EASE_SmoothTrapezoid,
		EASE_Jump // 山なり　ジャンプの挙動に

	};

private:
	int type = 0;



	bool isEasing = false;
	bool isFinished = false;
	float easingTotalTime = 0.0f;
	float easingTime = 0.0f;
	float time = 0;

	float c1 = 1.70158f;
	float c3 = c1 + 1;
	float n1 = 7.5625f;
	float d1 = 2.75f;

	float startNum = 0.0f;
	float endNum = 0.0f;

	Vector2 startPosVec2;
	Vector2 endPosVec2;

	Vector3 startPosVec3;
	Vector3 endPosVec3;

	float lerp(float sPos, float ePos, float tm) {
		return (float)((1.0f - tm) * sPos + tm * ePos);
	}
	float smoothstep(float edge0, float edge1, float x)
	{
		float t = (x - edge0) / (edge1 - edge0);
		t = std::clamp(t, 0.0f, 1.0f);
		return t * t * (3.0f - 2.0f * t);
	}

};

