#include "TestScene.h"

#include "WinApp.h"
#include "ImGuiManager.h"

void TestScene::Init() {

	text.Init();
	text.setFont("default");

	text2.Init();
	text2.setFont("default");

	camera.Initialize();
	camera.GetWorldPosition().z = -15.0f;

	//DirectionalLightParam dLightParam {
	//	.color = {255,255,255},
	//	.direction = {0.0f, -1.0f, 0.0f},
	//	.intensity = 1.0f
	//};

	//PointLightParam pLightParam;
	SpotLightParam sLightParam;

	sLight.Initialize(sLightParam);

	model = Model::CreateOBJ("resources/Models/bunny.obj");
	model->SetSpotLight(&sLight);


};

void TestScene::Update(GamePad* gamepad) {

	model->Update();
	camera.Update();

#ifdef USE_IMGUI
	ImGui::DragFloat3("camera", &camera.GetWorldPosition().x, 0.1f);
	ImGui::DragFloat3("spotLightPosition", &sLight.GetLightData()->position.x, 0.1f);
#endif

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

	model->Draw(camera);

	text.SetFontLayout(layout);
	text2.SetFontLayout(layout);

	text2.SetFontSize(10);

	text.drawString("ponikoEngine", {0,0}, Text::TextAnchor::LeftTop);

	//text2.drawString("Test",pos, Text::TextAnchor::Center);

	//sprite.SetPosition(pos);
	//sprite.Draw();
};