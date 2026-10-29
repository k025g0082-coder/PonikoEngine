#pragma once
#include "Scene.h"

#include "Text.h"

#include "Sprite.h"
#include "Model.h"

#include "AABB.h"

#include "WorldTransform.h"
#include "ScreenTransform.h"

class TestScene :
    public Scene
{
public:
	void Init();
	void Update(Input* input);
	void Draw();

private:
	PointLightParam* p_ = nullptr;;
	SpotLightParam* s_ = nullptr;
	DirectionalLightParam* d_ = nullptr;


	Sprite sprite;
	Sprite combo;

	Model* model = nullptr;
	Model* fieldModel = nullptr;
	Model* cubeModel = nullptr;

	Model* sheldModel = nullptr;

	Model* backModel = nullptr;
	WorldTransform backWorld1;
	WorldTransform backWorld2;

	Camera camera;

	WorldTransform world;

	DirectionalLightParam* lightAddress = nullptr;

	int timer = 0;

	Text text;
	Text text2;
	Text text3;

	int testCombo = 0;
	ScreenTransform comboPositions_[5];

	// telop
	Sprite telopSprite;
	Text telopText;

	Vector2 ps = { 100.0f,100.0f };
	int fontWidth = 32;
	float margin = 5.0f;

	ScreenTransform spworld1;
	ScreenTransform spworld2;

	AABB aabb;
};

