#include "TestScene.h"

#include "WinApp.h"
#include "ImGuiManager.h"

#include "SceneComponentLoader.h"

#include "ModelLoader.h"

#include "Particle.h"

void TestScene::Init() {

	text.Init();
	text.setFont("default");

	text2.Init();
	text2.setFont("default");

	camera.Initialize();

	PointLightParam pLightParam;
	SpotLightParam sLightParam;
	DirectionalLightParam dLightParam;


	lightAddress = &LightingManager::GetInstance()->AddLight(dLightParam);


	model = Model::CreateOBJ("resources/Models/bunny.obj");
	fieldModel = Model::CreateOBJ("resources/Models/backGroundCloud/backGroundCloud.obj");


	cubeModel = Model::CreateOBJ("resources/Engine/plane.obj");


	// LightingManagerで設定した空間のにモデルをセット
	model->SetLightSets();
	//fieldModel->SetLightSets();

	// 登録
	Particle::GetInstance()->RegionParticleModel("plane", "resources/Engine/plane.obj");
	Particle::GetInstance()->RegionParticleModel("cube", "resources/Models/cube/cube.obj");

	// カメラ登録
	Particle::GetInstance()->SetCamera(camera);

	// yomikomi
	if (SceneComponentLoader::GetInstance()->Load("resources/Scene/TestScene.json")) {

		// ※注　ポインタ
		json& data = SceneComponentLoader::GetInstance()->GetJson();

		// カメラの情報を読み込む
		camera.SetWorldTransform(JsonToWorld(data["camera"]));

	}

};

void TestScene::Update(GamePad* gamepad) {

	model->Update();

	cubeModel->Update();
	fieldModel->Update();

	camera.Update();

	ParticleParam param;

	if (timer % 10 == 0) {
		// パーティクル放出
		param.isBilliboard = true;
		Particle::GetInstance()->MakeNewParticle("plane", Vector3{ 0,0,20.0f }, 2, param);
		param.isBilliboard = false;
		param.speed = 1.0f;
		Particle::GetInstance()->MakeNewParticle("cube", Vector3{ 0,0,20.0f }, 10, param);
	}

	timer++;

#ifdef USE_IMGUI
	
	ImGui::DragFloat3("cameraPosition", &camera.GetWorldPosition().x, 0.1f);
	ImGui::DragFloat3("cameraRotation", &camera.GetRotation().x, 0.01f);

	//ImGui::DragFloat3("lightPos", &lightAddress->position.x, 0.1f);

	if (ImGui::Button("Save")) {

		// ※注　ポインタ
		json& data = SceneComponentLoader::GetInstance()->GetJson();


		data["camera"] =WorldToJson(camera.GetWorldTransform());


		SceneComponentLoader::GetInstance()->Save("resources/Scene/TestScene.json");

	}

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
	//fieldModel->Draw(camera);

	text.SetFontLayout(layout);
	text2.SetFontLayout(layout);

	text2.SetFontSize(10);

	text.drawString("ponikoEngine", {0,0}, Text::TextAnchor::LeftTop);

};