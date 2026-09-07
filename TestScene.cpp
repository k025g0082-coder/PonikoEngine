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
	model->SetAnimationLoop(false);

	fieldModel = Model::CreateOBJ("resources/Models/field/field.obj");

	cubeModel = Model::CreateOBJ("resources/Engine/plane.obj");

	backModel = Model::CreateOBJ("resources/Models/backGroundCloud/backGroundCloud.obj");
	backWorld1.Initialize();
	backWorld2.Initialize();

	// LightingManagerで設定した空間のにモデルをセット
	model->SetLightSets();
	fieldModel->SetLightSets();

	sprite.Initialize("resources/Engine/uvChecker.png");
	sprite.SetTextureRect(110.0f, 10.0f, 100.0f, 100.0f);

	// 読込み
	if (SceneComponentLoader::GetInstance()->Load("resources/Scene/TestScene.json")) {

		SceneComponentLoader* loder = SceneComponentLoader::GetInstance();
		
		// ライト配置のロード
		SceneComponentLoader::GetInstance()->LightLoad();

		// カメラの情報を読み込む
		camera.SetWorldTransform(loder->GetWorldTransformData("camera"));
		model->SetWorldTransform(loder->GetWorldTransformData("player"));

		sprite.SetScreenTransform(loder->GetScreenTransformData("sprite"));

		
	}

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

	hpBarBackSprite.Initialize("resources/white100x100.png");
	hpBarFrontSprite.Initialize("resources/white100x100.png");
	hpBarFrontSprite.SetColorRGBA({ 1.0f,0,0,1.0f });

};

void TestScene::Update(GamePad* gamepad) {

	// 必ず必要
	Scene::Update(gamepad);

	model->Update();

	cubeModel->Update();
	fieldModel->Update();
	backModel->Update();

	sprite.UpdateScreenTransform();

	camera.Update();

	world.UpdateMatrix();

	backWorld1.scale_ = { 2.0f,2.0f,2.0f };
	backWorld1.rotation_.y += 0.01f;

	backWorld1.UpdateMatrix();
	backWorld2.UpdateMatrix();

	for (int i = 0; i < 5; i++) {
		comboPositions_[i].UpdateMatrix();
	}

	timer++;

#ifdef USE_IMGUI
	
	camera.GetWorldTransform().ImGuiView("camera");

	model->GetWorldTransform().ImGuiView("player");

	sprite.GetScreenTransform().ImGuiView("sprite");

	if (ImGui::Button("Save")) {

		// ※注　ポインタ
		SceneComponentLoader* loder = SceneComponentLoader::GetInstance();

		json& data = loder->GetJson();

		// 必ず
		SceneComponentLoader::GetInstance()->ClearJson();


		// ライト配置のセーブ
		SceneComponentLoader::GetInstance()->LightSave();

		loder->SetWorldTransformData("camera", camera.GetWorldTransform());
		loder->SetWorldTransformData("player", model->GetWorldTransform());
		loder->SetScreenTransformData("sprite", sprite.GetScreenTransform());
		

		SceneComponentLoader::GetInstance()->Save("resources/Scene/TestScene.json");

	}
	if (ImGui::Button("Effect")) {


		Particle::GetInstance()->SetCamera(camera);

		// とうろく
		EffectManager::AddEffect(new ExampleEffect, {0,0,0});

	}
	ImGui::DragFloat2("Position", &ps.x, 1.0f);
	ImGui::DragInt("Font Width", &fontWidth, 1);
	ImGui::DragFloat("Margin", &margin, 0.1f);
	ImGui::Separator();

	ImGui::DragInt("HP", &hp_, 1, 0, maxHp_);
	ImGui::DragInt("Max HP", &maxHp_, 1, 1, 999);

	ImGui::DragFloat("Bar Width", &barWidth, 1.0f, 1.0f, 1000.0f);
	ImGui::DragFloat("Bar Height", &barHeight, 1.0f, 1.0f, 200.0f);
	ImGui::DragFloat("Margin", &barMargin, 1.0f, 0.0f, 50.0f);

	ImGui::DragFloat2("Position", &barps.x, 1.0f);
#endif

	telopSprite.UpdateScreenTransform();

	hpBarBackSprite.UpdateScreenTransform();
	hpBarFrontSprite.UpdateScreenTransform();
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
	//fieldModel->Draw(camera);

	sprite.Draw();

	backModel->Draw(backWorld1,camera);
	backModel->Draw(backWorld2,camera);

	text.SetFontLayout(layout);
	text2.SetFontLayout(layout);

	//sprite.Draw();

	std::string t = "スコア：";

	//testCombo++;
	
	//text.drawString(t, { (float)testCombo ,(float)testCombo }, Text::TextAnchor::Center);

	//text2.drawString(std::to_string(timer), {50,0}, Text::TextAnchor::LeftTop);


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



	// 背景
	hpBarBackSprite.SetPosition(barps);
	hpBarBackSprite.SetSize(
		(int)barWidth,
		(int)barHeight
		);
	hpBarBackSprite.Draw();

	// HP割合
	float hpRate = static_cast<float>(hp_) / maxHp_;

	// 前景
	hpBarFrontSprite.SetPosition({
		barps.x + barMargin,
		barps.y + barMargin
		});

	hpBarFrontSprite.SetSize(
		(int)((barWidth - barMargin * 2.0f) * hpRate),
		(int)(barHeight - barMargin * 2.0f)
		);
	hpBarFrontSprite.Draw();

	//combo.Draw(comboPositions_[1]);


	//int c = 75678;

	//for (int i = 0; i < 5; i++)
	//{
	//	int digit = c % 10;
	//	c /= 10;

	//	float left = digit * fontW;

	//	combo.SetTextureRect(
	//		left,
	//		0.0f,
	//		fontW,
	//		fontH
	//	);

	//	comboPositions_[i].translation_ = {
	//		pos.x + i * fontW,
	//		pos.y,
	//		0.0f
	//	};

	//	comboPositions_[i].UpdateMatrix();

	//	/*combo.Draw(comboPositions_[i]);*/

	//	// 全桁読み終わった
	//	if (c == 0)
	//	{
	//		break;
	//	}
	//}
};