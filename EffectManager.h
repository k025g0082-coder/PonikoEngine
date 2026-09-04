#pragma once
#include "Effect.h"
#include "Camera.h"
#include <vector>

// エフェクトを登録する
class EffectManager
{
public:

	static void AddEffect(Effect* effect, Vector3 position);
	static void Update();

//	static void SetCamera(Camera& c);
private:
	static std::vector<Effect*> effects_;

	static int nextEffectId_;
};

