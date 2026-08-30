#pragma once
#include "Scene.h"

#include "Text.h"

#include "Sprite.h"
#include "Model.h"

#include "DirectionalLight.h"
#include "SpotLight.h"
#include "PointLight.h"


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

	Camera camera;

	DirectionalLight dLight;
	SpotLight sLight;
	PointLight pLight;

	Text text;
	Text text2;
	Text text3;
};

