#pragma once
#include "Scene.h"

#include "Text.h"

#include "Sprite.h"
#include "Model.h"

#include "WorldTransform.h"

class TestScene :
    public Scene
{
public:
	void Init();
	void Update(GamePad* gamepad);
	void Draw();

private:

	Sprite sprite;
	Model* model = nullptr;
	Model* fieldModel = nullptr;
	Model* cubeModel = nullptr;

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
};

