#pragma once
#include "Scene.h"

#include "Text.h"

#include "Sprite.h"
#include "Model.h"

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

	Camera camera;

	DirectionalLightParam* lightAddress = nullptr;

	Text text;
	Text text2;
	Text text3;
};

