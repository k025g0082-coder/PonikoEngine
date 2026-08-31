#pragma once
#include "Scene.h"

#include "Text.h"

#include "Sprite.h"
#include "Model.h"
#include "Particle.h"


class TestScene :
    public Scene
{
public:
	void Init();
	void Update(GamePad* gamepad);
	void Draw();

private:
	Particle particle;

	Sprite sprite;
	Model* model = nullptr;
	Model* fieldModel = nullptr;
	Model* cubeModel = nullptr;

	Camera camera;

	PointLightParam* lightAddress = nullptr;

	Text text;
	Text text2;
	Text text3;
};

