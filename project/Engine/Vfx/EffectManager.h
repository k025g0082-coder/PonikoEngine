#pragma once
#include "Effect.h"
#include "Camera.h"
#include <vector>

#include <memory>

// エフェクトを登録する
class EffectManager
{
public:

	/// <summary>
	/// エフェクト増やす
	/// <param name="effect">増やすエフェクトクラス</param>
	/// <param name="position">生成座標</param>
	/// <param name="isDebugMode">デバッグモード（ウインドウ出すか）</param>
	/// </summary>
	static void AddEffect(std::unique_ptr<Effect> effect, Vector3 position,bool isDebugMode = true);
	static void Update();

	// エフェクト消去 すべての終了フラグ立てる
	static void ResetEffect();

private:
	static std::vector<std::unique_ptr<Effect>> effects_;

	static int nextEffectId_;
};

