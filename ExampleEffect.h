#pragma once
#include "Effect.h"

// テスト用
class ExampleEffect : public Effect {

public:
	void Initialize(Vector3 position) override;
	void Update() override;

private:
	int timer_ = 0;
};