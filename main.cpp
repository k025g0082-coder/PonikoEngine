#include "PonikoEngine.h"

#include "ExampleScene.h"

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int)
{

	PonikoEngine::Initialize(L"ぽにこエンジンn");

	// 初期Gameシーンの登録
	Game::GetInstance()->StartGameScene(new ExampleScene);


	while (!WinApp::GetInstance()->ProcessMessage())
	{

		PonikoEngine::Update();
		PonikoEngine::Draw();
	
	}

	PonikoEngine::Finalize();

	return 0;
}

// Exampleの替え
//#include "PonikoEngine.h"
//
//int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int)
//{
//
//	PonikoEngine::Initialize(L"ぽにこエンジンn");
//
//
//	// ===カメラ設定
//	Camera camera;
//
//	camera.Initialize();
//
//
//	// ===ライト設定
//
//	DirectionalLight light;
//
//	DirectionalLightParam data = {
//		.color = { 1.0f, 1.0f, 1.0f, 1.0f },
//		.direction = { 0.0f, 0.0f, 0.0f },
//		.intensity = 1.0f
//	};
//
//	// ライト初期化
//	light.Initialize(data);
//
//
//	// ===モデル生成
//	Model* model = Model::CreateOBJ("resources/Models/test/Capoeira.fbx");
//	model->SetLight(&light);    // ライト設定
//
//	Model* model2 = Model::CreateOBJ("resources/Models/rail/rail.obj");
//	model2->SetLight(&light);   // ライト設定
//
//	//=== スプライト
//	Sprite sprite;
//	sprite.Initialize("resources/Engine/uvChecker.png");
//	sprite.SetPos({ 0.0f, 0.0f, 0.0f });
//
//	// フォント登録
//	Text::RegisterFont("LINESeedJP_TTF_Bd", "resources/Engine/default.ttf", 32, 32);
//
//
//	// ===テキスト
//	Text text;
//	text.Init();
//	text.setFont("LINESeedJP_TTF_Bd");
//
//
//	while (!WinApp::GetInstance()->ProcessMessage())
//	{
//		// 更新
//		ImGuiManager::GetInstance()->Begin();
//
//		/*　ここに更新処理　*/
//		camera.Update();
//
//		model->Update();
//		model2->Update();
//
//#ifdef USE_IMGUI
//
//		//ImGui::DragFloat3("model-pos", &model->GetWorldTransform().translation_.x, 0.5f, 0.0f);
//		//ImGui::DragFloat3("model-rot", &model->GetWorldTransform().rotation_.x, 0.1f, 0.0f);
//		//ImGui::DragFloat3("model-sc ", &model->GetWorldTransform().scale_.x, 0.5f, 0.0f);
//
//		//ImGui::DragFloat3("model2-pos", &model2->GetWorldTransform().translation_.x, 0.5f, 0.0f);
//		//ImGui::DragFloat3("model2-rot", &model2->GetWorldTransform().rotation_.x, 0.5f, 0.0f);
//		//ImGui::DragFloat3("model2-sc ", &model2->GetWorldTransform().scale_.x, 0.5f, 0.0f);
//
//		//ImGui::DragFloat4("lightColor", &light.GetLightData()->color.x, 0.5f, 0.0f);
//		//ImGui::DragFloat3("lightDIrection", &light.GetLightData()->direction.x, 0.1f, 0.0f);
//		//ImGui::DragFloat("lightIntensity", &light.GetLightData()->intensity, 0.5f, 0.0f);
//
//#endif
//
//		ImGuiManager::GetInstance()->End();
//
//
//		// 描画
//		DirectXCommon::GetInstance()->PreDraw();
//
//		model->Draw(camera);
//		//model2->Draw(camera);
//
//		sprite.Draw();
//
//		// ２d文字描画
//		//text.drawString(U"てすとテキスト", { 0.0f, 0.0f });
//
//		// IMGUI byouga
//		ImGuiManager::GetInstance()->Draw();
//
//		// 描画終了
//		DirectXCommon::GetInstance()->PostDraw();
//	}
//
//	PonikoEngine::Finalize();
//
//	return 0;
//}