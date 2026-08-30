#pragma once
#include "Scene.h"

#include "Text.h"

class ExampleGameScene :
    public Scene
{
public:
	void Init() override;
	void Update(GamePad* gamepad) override;
	void Draw() override;

	int timer = 0;

	bool visible = false;

private:
	Text text;
	Text eventText;
};

