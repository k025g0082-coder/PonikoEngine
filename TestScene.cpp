#include "TestScene.h"

#include "WinApp.h"
#include "ImGuiManager.h"

#include "SceneComponentLoader.h"

#include "ModelLoader.h"


#include "Particle.h"

#include "ExampleEffect.h"
#include "EffectManager.h"


void TestScene::Init() {

	text.Init();


	text2.Init();
	text2.setFont("default");

	camera.Initialize();

	PointLightParam pLightParam;
	SpotLightParam sLightParam;
	DirectionalLightParam dLightParam;


	lightAddress = &LightingManager::GetInstance()->AddLight(dLightParam);


	model = Model::CreateOBJ("resources/Models/player/player.fbx");
	model->SetAnimationSpeed(0.5f);

	fieldModel = Model::CreateOBJ("resources/Models/field/field.obj");

	cubeModel = Model::CreateOBJ("resources/Engine/plane.obj");

	backModel = Model::CreateOBJ("resources/Models/backGroundCloud/backGroundCloud.obj");
	backWorld1.Initialize();
	backWorld2.Initialize();

	// LightingManagerで設定した空間のにモデルをセット
	model->SetLightSets();
	fieldModel->SetLightSets();

	// 読込み
	if (SceneComponentLoader::GetInstance()->Load("resources/Scene/TestScene.json")) {

		SceneComponentLoader* loder = SceneComponentLoader::GetInstance();
		
		// ライト配置のロード
		SceneComponentLoader::GetInstance()->LightLoad();

		// カメラの情報を読み込む
		camera.SetWorldTransform(loder->GetWorldTransformData("camera"));
		model->SetWorldTransform(loder->GetWorldTransformData("player"));

		
	}

	world.Initialize();

	Text::RegisterFont("main", "resources/Fonts/DenkiChip.ttf", 32, 32);
	text.setFont("main");
};

void TestScene::Update(GamePad* gamepad) {

	// 必ず必要
	Scene::Update(gamepad);

	model->Update();

	cubeModel->Update();
	fieldModel->Update();
	backModel->Update();

	camera.Update();

	world.UpdateMatrix();

	backWorld1.scale_ = { 2.0f,2.0f,2.0f };
	backWorld1.rotation_.y += 0.01f;

	backWorld1.UpdateMatrix();
	backWorld2.UpdateMatrix();

	timer++;

#ifdef USE_IMGUI
	
	camera.GetWorldTransform().ImGuiView("camera");

	model->GetWorldTransform().ImGuiView("player");

	if (ImGui::Button("Save")) {

		// ※注　ポインタ
		json& data = SceneComponentLoader::GetInstance()->GetJson();

		// 必ず
		SceneComponentLoader::GetInstance()->ClearJson();


		// ライト配置のセーブ
		SceneComponentLoader::GetInstance()->LightSave();

		data["camera"] = WorldToJson(camera.GetWorldTransform());
		data["player"] = WorldToJson(model->GetWorldTransform());


		SceneComponentLoader::GetInstance()->Save("resources/Scene/TestScene.json");

	}
	if (ImGui::Button("Effect")) {


		Particle::GetInstance()->SetCamera(camera);

		// とうろく
		EffectManager::AddEffect(new ExampleEffect, {0,0,0});

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

	model->Draw(world,camera);
	fieldModel->Draw(camera);

	backModel->Draw(backWorld1,camera);
	backModel->Draw(backWorld2,camera);

	text.SetFontLayout(layout);
	text2.SetFontLayout(layout);

	std::string t = "スコア：";

	text.drawString(t, {0,0}, Text::TextAnchor::LeftTop);

	text2.drawString(std::to_string(timer), {50,0}, Text::TextAnchor::LeftTop);

};