#include "TestScene.h"

#include "WinApp.h"
#include "ImGuiManager.h"

#include "SceneComponentLoader.h"

void TestScene::Init() {

	text.Init();
	text.setFont("default");

	text2.Init();
	text2.setFont("default");

	camera.Initialize();

	//DirectionalLightParam dLightParam {
	//	.color = {255,255,255},
	//	.direction = {0.0f, -1.0f, 0.0f},
	//	.intensity = 1.0f
	//};

	PointLightParam pLightParam;
	SpotLightParam sLightParam;
	DirectionalLightParam dLightParam;

	
	LightingManager::GetInstance()->AddLight(pLightParam);


	model = Model::CreateOBJ("resources/Models/bunny.obj");

	// LightingManagerで設定した空間のにモデルをセット
	model->SetLightSets();


	// yomikomi
	if (SceneComponentLoader::GetInstance()->Load("resources/Scene/TestScene.json")) {

		// ※注　ポインタ
		json& data = SceneComponentLoader::GetInstance()->GetJson();


		camera.SetWorldTransform(JsonToWorld(data["camera"]));


		//camera.GetWorldPosition() = JsonToVector3(data["camera"]["position"]);
		//data["player"]["position"] = json::array({ 0.0f, 1.0f, 2.0f});


	}

};

void TestScene::Update(GamePad* gamepad) {

	model->Update();
	camera.Update();

#ifdef USE_IMGUI
	
	ImGui::DragFloat3("camera", &camera.GetWorldPosition().x, 0.1f);

	if (ImGui::Button("Save")) {

		// ※注　ポインタ
		json& data = SceneComponentLoader::GetInstance()->GetJson();


		data["camera"] =WorldToJson(camera.GetWorldTransform());


		SceneComponentLoader::GetInstance()->Save("resources/Scene/TestScene.json");

	}

	//ImGui::DragFloat3("dLightPosition", &dLight.GetLightData(0)->direction.x, 0.1f);
	//ImGui::DragFloat3("plightPosition", &pLight.GetLightData(0)->position.x, 0.1f);
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