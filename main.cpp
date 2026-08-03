#include "PonikoEngine.h"

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int)
{

	PonikoEngine::Initialize(L"ぽにこエンジン");


	// ===カメラ設定
	Camera camera;

	camera.Initialize();


	// ===ライト設定

	DirectionalLight light;

	DirectionalLightParam data = {
		.color = { 1.0f, 1.0f, 1.0f, 1.0f },
		.direction = { 0.0f, 0.0f, 0.0f },
		.intensity = 1.0f
	};

	// ライト初期化
	light.Initialize(data);


	// ===モデル生成
	Model* model = Model::CreateOBJ("resources/Models/test/Capoeira.fbx");
	model->SetCamera(&camera);
	model->SetLight(&light);    // ライト設定

	Model* model2 = Model::CreateOBJ("resources/Models/rail/rail.obj");
	model2->SetCamera(&camera);
	model2->SetLight(&light);   // ライト設定

	// フォント登録
	Text::RegisterFont("LINESeedJP_TTF_Bd", "resources/Fonts/LINESeedJP_TTF_Bd.ttf", 32, 32);


	// ===テキスト
	Text text;
	text.Init();
	text.setFont("LINESeedJP_TTF_Bd");


	while (!WinApp::GetInstance()->ProcessMessage())
	{
		// 更新
		ImGuiManager::GetInstance()->Begin();

			/*　ここに更新処理　*/
			camera.Update();

			model->Update();
			model2->Update();

			text.Render();

	#ifdef USE_IMGUI

			ImGui::DragFloat3("pos", &model->GetWorldTransform().translate.x, 0.5f, 0.0f);
			ImGui::DragFloat3("rot", &model->GetWorldTransform().rotate.x, 0.1f, 0.0f);
			ImGui::DragFloat3("sc ", &model->GetWorldTransform().scale.x, 0.5f, 0.0f);

			ImGui::DragFloat3("pos2", &model2->GetWorldTransform().translate.x, 0.5f, 0.0f);
			ImGui::DragFloat3("rot2", &model2->GetWorldTransform().rotate.x, 0.5f, 0.0f);
			ImGui::DragFloat3("sc2 ", &model2->GetWorldTransform().scale.x, 0.5f, 0.0f);

			ImGui::DragFloat4("lightColor", &light.GetLightData()->color.x, 0.5f, 0.0f);
			ImGui::DragFloat3("lightDIrection", &light.GetLightData()->direction.x, 0.1f, 0.0f);
			ImGui::DragFloat("lightIntensity", &light.GetLightData()->intensity, 0.5f, 0.0f);

	#endif

		ImGuiManager::GetInstance()->End();


		// 描画
		DirectXCommon::GetInstance()->PreDraw();

			model->Draw(DirectXCommon::GetInstance()->GetCommandList().Get());
			model2->Draw(DirectXCommon::GetInstance()->GetCommandList().Get());

			// ２d文字描画
			text.drawString(U"てすとテキスト", { 0.0f, 0.0f });

			// IMGUI byouga
			ImGuiManager::GetInstance()->Draw();

		// 描画終了
		DirectXCommon::GetInstance()->PostDraw();
	}

	PonikoEngine::Finalize();

	return 0;
}