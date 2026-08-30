#pragma once
#include "Scene.h"

#include "Text.h"
#include "Sprite.h"

class TestScene :
    public Scene
{
public:
	void Init();
	void Update(GamePad* gamepad);
	void Draw();

private:

	Sprite sprite;

	Text text;
	Text text2;
	Text text3;
};

