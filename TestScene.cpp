#include "Scene.h"
#include "TestScene.h"

#include "Editor.h"

#include "WinApp.h"
#include "ImGuiManager.h"

#include "PostEffectManager.h"

#include "SceneComponentLoader.h"
#include "SoundManager.h"

#include "ShadowManager.h"

#include "ModelLoader.h"


#include "Particle.h"

#include "ExampleEffect.h"
#include "EffectManager.h"

#include "DebugLine.h"


void TestScene::Init() {

	text.Init();

	camera.Initialize();

	/* 必）シーンカメラ設定 */
	Scene::SetSceneCamera(&camera);

	// シーンのモデルなど読込
	Editor::GetInstance()->HierarchyLoad("resources/Scene/example.json");


	PointLightParam pLightParam;
	SpotLightParam sLightParam;
	DirectionalLightParam dLightParam;

	//SoundManager::GetInstance()->LoadWave("bgm", "resources/Sound/grandciel.wav");
	//SoundManager::GetInstance()->SetSoundLoop("bgm", 28.375f, 99.5f);
	//SoundManager::GetInstance()->Play("bgm",1.0f,95.0f);


	lightAddress = &LightingManager::GetInstance()->AddLight(dLightParam);


//	model = Model::CreateOBJ("resources/Models/player/player.fbx");
	model = Model::CreateOBJ("resources/Models/cube/cube.obj");


	//model->SetAnimationSpeed(0.5f);
	//model->SetAnimationLoop(false);

	sheldModel = Model::CreateOBJ("resources/Models/sheld/sheld.obj");

	fieldModel = Model::CreateOBJ("resources/Models/field/field.obj");

	cubeModel = Model::CreateOBJ("resources/Engine/plane.obj");

	backModel = Model::CreateOBJ("resources/Models/backGroundCloud/backGroundCloud.obj");
	backWorld1.Initialize();
	backWorld2.Initialize();

	// LightingManagerで設定した空間のにモデルをセット
	model->SetLightSets();
	fieldModel->SetLightSets();

	sprite.Initialize("resources/Engine/uvChecker.png");
	//sprite.SetTextureRect(110.0f, 10.0f, 100.0f, 100.0f);

	spworld1.Initialize();
	spworld2.Initialize();

	// 読込み
	if (SceneComponentLoader::GetInstance()->Load("resources/Scene/TestScene.json")) {

		SceneComponentLoader* loder = SceneComponentLoader::GetInstance();
		
		// ライト配置のロード
		SceneComponentLoader::GetInstance()->LightLoad();

		loder->BeginGroup("Test");

		loder->Property("player", model->GetWorldTransform());

		loder->EndGroup();

		loder->LoadGroup("Test");

		//// カメラの情報を読み込む
		camera.SetWorldTransform(loder->GetWorldTransformData("camera"));
		//model->SetWorldTransform(loder->GetWorldTransformData("player"));

		sprite.SetScreenTransform(loder->GetScreenTransformData("sprite"));

		loder->LoadTextData("text", &text2);

	}
	
	sheldModel->SetLightSets();

	world.Initialize();

	Text::RegisterFont("main", "resources/Fonts/DenkiChip.ttf", 32, 32);
	text.setFont("main");

	combo.Initialize("resources/Textures/comboFont.png");

	for (int i = 0; i < 5; i++) {
		comboPositions_[i].Initialize();
	}

	telopText.setFont("main");

	telopSprite.Initialize("resources/white100x100.png");
	telopSprite.SetColorRGBA({ 0,0,0,1.0f});

	telopText.Init();
	telopText.setFont("main");

	ShadowManager::GetInstance()->Register(model);
	//ShadowManager::GetInstance()->Register(fieldModel);
};

void TestScene::Update(Input* input) {

	model->Update();

	cubeModel->Update();
	fieldModel->Update();
	backModel->Update();

	sheldModel->Update();
	sprite.UpdateScreenTransform();

	
	camera.Update();
	
	// キャラ中心に回す専用のUpdate
	//camera.SphericalCameraUpdate(world.translation_);

	world.UpdateMatrix();

	backWorld1.scale_ = { 2.0f,2.0f,2.0f };
	backWorld1.rotation_.y += 0.01f;

	backWorld2.rotation_.y += 0.02f;

	backWorld1.UpdateMatrix();
	backWorld2.UpdateMatrix();

	for (int i = 0; i < 5; i++) {
		comboPositions_[i].UpdateMatrix();
	}

	spworld1.UpdateMatrix();
	spworld2.UpdateMatrix();


	timer++;

#ifdef USE_IMGUI
	
	camera.GetWorldTransform().ImGuiView("camera");

	model->GetWorldTransform().ImGuiView("player");
	fieldModel->GetWorldTransform().ImGuiView("field");
	//world.ImGuiView("player");

	sprite.GetScreenTransform().ImGuiView("sprite");

	if (ImGui::Button("Save")) {

		// ※注　ポインタ
		SceneComponentLoader* loder = SceneComponentLoader::GetInstance();

		json& data = loder->GetJson();

		// 必ず
		SceneComponentLoader::GetInstance()->ClearJson();

		// ライト配置のセーブ
		SceneComponentLoader::GetInstance()->LightSave();

		loder->SaveGroup("Test");

		loder->SetWorldTransformData("camera", camera.GetWorldTransform());
		loder->SetWorldTransformData("player",model->GetWorldTransform());
		loder->SetScreenTransformData("sprite", sprite.GetScreenTransform());
		
		loder->SaveTextData("text", &text2);

		SceneComponentLoader::GetInstance()->Save("resources/Scene/TestScene.json");

	}

	if (ImGui::Button("Effect")) {


		Particle::GetInstance()->SetCamera(&camera);

		// とうろく
		EffectManager::AddEffect(new ExampleEffect, {0,0,0});

	}
	if (ImGui::Button("FadeStopBgm")) {

		SoundManager::GetInstance()->FadeOut("bgm", 1.0f);

	}
	ImGui::DragFloat2("Position", &ps.x, 1.0f);
	ImGui::DragInt("Font Width", &fontWidth, 1);
	ImGui::DragFloat("Margin", &margin, 0.1f);
	ImGui::Separator();

	text2.ImGuiView();

	ImGui::Separator();

	sheldModel->GetWorldTransform().ImGuiView("sheld");
	ImGui::Separator();
	
	// 球面座標設定用
	camera.SphericalImGuiView();

	aabb.ImGuiView();
	ImGui::Text("MousePositionX:%f, Y:%f", Input::GetInstance()->GetMousePosition().x, Input::GetInstance()->GetMousePosition().y);
	ImGui::Text("MouseCliCK:L %d, R:%d", Input::GetInstance()->GetLeftMouseButtonDown(), Input::GetInstance()->GetRightMouseButtonDown());
	ImGui::Text("Mousewheel:%d", Input::GetInstance()->GetMouseWheel());

	ImGui::DragFloat3("LightShadowPosition", &ShadowManager::GetInstance()->lightPosition.x, 1.0f);
	ImGui::DragFloat3("LightShadowDirection", &ShadowManager::GetInstance()->lightDirection.x, 1.0f);
#endif

	// ポストエフェクトお試し
	PostEffectManager::GetInstance()->ImGuiView();


	telopSprite.UpdateScreenTransform();
	

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
	//model->Draw(world,camera);

	fieldModel->Draw(camera);

	// 複数テスト用
	//sprite.Draw(spworld1);
	//sprite.Draw(spworld2);


	//sheldModel->Draw(camera);

//	backModel->Draw(backWorld1,camera);
//	backModel->Draw(backWorld2,camera);



	text.SetFontLayout(layout);

	//sprite.Draw();

	std::string t = "スコア：";

	//testCombo++;
	
	//text.drawString("test", {(float)testCombo ,(float)testCombo}, Text::TextAnchor::Center);

	text2.drawString("textTest", TextAnchor::LeftTop);


	// telop
	std::string text = "text";

	telopSprite.SetPosition(ps);
	telopSprite.SetSize((int)text.length() * fontWidth, 64 + (int)margin);
	//telopSprite.Draw();

	Vector2 pps;
	pps.x = ps.x +margin;
	pps.y = ps.y +margin;

	//telopText.drawString(text, pps);

	//combo.SetTextureRect(
	//	256.0f,
	//	0.0f,
	//	64.0f,
	//	128.0f
	//);
	//combo.Draw();

	float fontW = 64.0f;
	float fontH = 128.0f; 
	
	combo.SetTextureRect(
		64.0f,
		0.0f,
		fontW,
		fontH
	); 
//	combo.Draw(comboPositions_[0]);


	combo.SetTextureRect(
		100.0f,
		0.0f,
		fontW,
		fontH
	);
	comboPositions_[1].translation_ = { 128.0f,0 };
	comboPositions_[1].UpdateMatrix();


	//sprite.Draw();


	DebugLine::SetCamera(&camera);

	// デバッグ用の線 追加
	//DebugLine::AddLine(world.translation_, sheldModel->GetWorldTransform().translation_ + Vector3{ 0,1,0 }, Vector4{ 1,0,0,1 });


	// AABB描画
	//DebugLine::DrawAABB(aabb, { 0,0,1,1 });

};