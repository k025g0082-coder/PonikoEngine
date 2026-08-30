#include "TestScene.h"

#include "WinApp.h"

void TestScene::Init() {

	text.Init();
	text.setFont("default");

	text2.Init();
	text2.setFont("default");

};

void TestScene::Update(GamePad* gamepad) {

};

void TestScene::Draw() {

	// 中央
	Vector2 pos{
		(float)WinApp::GetInstance()->GetClientWidth() / 2.0f,
		(float)WinApp::GetInstance()->GetClientHeight() / 2.0f
	};

	// 文字の設定をまとめたレイアウト
	fontLayout layout{

		.textSize = 20,
		.textColor = Vector3{ 255,255,255 },

		.outlineSize = 10,
		.outlineColor = Vector3{ 0,0,0 },
		.isOutlineBlur = true

	};

	text.SetFontLayout(layout);
	text2.SetFontLayout(layout);

	text2.SetFontSize(10);

	text.drawString("ponikoEngine", {0,0}, Text::TextAnchor::LeftTop);
	text2.drawString("Test",pos, Text::TextAnchor::Center);

	//sprite.SetPosition(pos);
	//sprite.Draw();
};