#include "EasingVec.h"

void EasingVec::Start(int t, float total, float start, float end) {

	//if (isEasing)return;

	isEasing = true;

	startNum = start;
	endNum = end;

	type = t;
	easingTime = 0;
	easingTotalTime = total;
	time = 0;
}
void EasingVec::Start(int t, float total, Vector2 start, Vector2 end) {

	/*if (isEasing)return;*/

	isEasing = true;

	startPosVec2 = start;
	endPosVec2 = end;

	type = t;
	easingTime = 0;
	easingTotalTime = total;
	time = 0;
}

void EasingVec::Start(int t, float total, Vector3 start, Vector3 end) {

	isEasing = true;

	startPosVec3 = start;
	endPosVec3 = end;

	type = t;
	easingTime = 0;
	easingTotalTime = total;
	time = 0;
}

void EasingVec::Step() {
	if (isEasing) {
		time = easingTime / easingTotalTime;

		easingTime++;

		if (time >= 1.0f) {
			isEasing = false;
			isFinished = true;
			time = 1.0f;
		}
	}
}

void EasingVec::Reset() {
	easingTime = 0;
	isEasing = false;
	isFinished = false;
}

float EasingVec::getEasingTime() {

	float up;
	float down;

	switch (type) {
	case EASE_None:
		return time;
		break;
	case EASE_InSine:
		return 1 - cosf((time * (float)M_PI) / 2);
		break;
	case EASE_OutSine:
		return sinf((time * (float)M_PI) / 2);
		break;
	case EASE_InOutSine:
		return -(cosf((float)M_PI * time) - 1) / 2;
		break;
	case EASE_InOutExpo:
		if (time == 0.0f) {
			return 0.0f;
		}
		if (time == 1.0f) {
			return 1.0f;
		}

		if (time < 0.5f) {
			return powf(2.0f, 20.0f * time - 10.0f) / 2.0f;
		}
		else {
			return (2.0f - powf(2.0f, -20.0f * time + 10.0f)) / 2.0f;
		}
		break;


	case EASE_InBack:


		return c3 * time * time * time - c1 * time * time;
		break;
	case EASE_OutBack:


		return 1 + c3 * powf(time - 1, 3) + c1 * powf(time - 1, 2);
		break;
	case EASE_OutBounce:

		if (time < 1.0f / d1) {
			return n1 * time * time;
		}
		else if (time < 2.0f / d1) {
			float t = time - 1.5f / d1;
			return n1 * t * t + 0.75f;
		}
		else if (time < 2.5f / d1) {
			float t = time - 2.25f / d1;
			return n1 * t * t + 0.9375f;
		}
		else {
			float t = time - 2.625f / d1;
			return n1 * t * t + 0.984375f;
		}
		break;

	case EASE_BouncyMountain:
		return sinf(time * 3.14159265f) * (1.0f + 0.2f * sinf(time * 6.28318f));
		break;

	case EASE_InExpo:
		if (time <= 0.0f) {
			return 0.0f;
		}
		return powf(2.0f, 10.0f * time - 10.0f);
		break;
	case EASE_SmoothTrapezoid:

		up = smoothstep(0.0f, 0.25f, time);
		down = 1.0f - smoothstep(0.75f, 1.0f, time);
		return up * down;
		break;
	case EASE_Jump:
		return 4.0f * time * (1.0f - time);
		break;
	}
	return 0;
}

float EasingVec::getEasingNum() {
	return lerp(startNum, endNum, getEasingTime());
};
Vector2 EasingVec::getEasingVec2() {
	Vector2 v;

	v.x = lerp(startPosVec2.x, endPosVec2.x, getEasingTime());
	v.y = lerp(startPosVec2.y, endPosVec2.y, getEasingTime());
	return v;
};

Vector3 EasingVec::getEasingVec3() {
	Vector3 v;

	v.x = lerp(startPosVec3.x, endPosVec3.x, getEasingTime());
	v.y = lerp(startPosVec3.y, endPosVec3.y, getEasingTime());
	v.z = lerp(startPosVec3.z, endPosVec3.z, getEasingTime());
	return v;
};