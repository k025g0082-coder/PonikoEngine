#include "PonikoEngine.h"
#include "PipelineStateManager.h"
#include "ImGuiManager.h"

#include "Text.h"

#include <memory>
#include <filesystem>

// ここにメンバを書く
namespace {
	class EngineImpl { // 内部実装、の略
	public:
		Triangle triangle;
		Sprite sprite;
		Model* model;
		Model* model2;
		Text text;
		Text text2;
		Camera camera;
		DirectionalLight light;
	};
	static std::unique_ptr<EngineImpl> impl; //　一つのポインタ、そしてメモリを自動で開放してくれるらしい
}

namespace PonikoEngine {

	void Initialize(const wchar_t* title) {

		// COM初期化
		CoInitializeEx(0, COINIT_MULTITHREADED);

		// ポインタ取得
		impl = std::make_unique<EngineImpl>();

		// WinApp初期化。ウインドウ生成
		WinApp::GetInstance()->Initialize(title, 1280, 720);

		// DirectXCommon初期化
		DirectXCommon::GetInstance()->Initialize();

		// 各種、パイプライン設定と登録
		PipelineStateManager::GetInstance()->Initialize(DirectXCommon::GetInstance()->GetDevice().Get());

		// もじシステム初期化
		Text::Initialize();

		// フォント登録
		Text::RegisterFont("LINESeedJP_TTF_Bd", "resources/Fonts/LINESeedJP_TTF_Bd.ttf", 32, 32);

		impl->text.Init();

		// フォント設定ｗ
		impl->text.setFont("LINESeedJP_TTF_Bd");

		// Imgui初期化
		ImGuiManager::GetInstance()->Initialize();

		// カメラ
		impl->camera.Initialize();
	
		// ライト設定
		DirectionalLightParam data ={
			.color = { 1.0f, 1.0f, 1.0f, 1.0f },
			.direction = { 0.0f, 0.0f, 0.0f },
			.intensity = 1.0f
		};

		impl->light.Initialize(data);

		impl->triangle.Initialize();
		impl->triangle.SetCamera(&impl->camera);

		//impl->sprite.Initialize("resources/default.png");

		//impl->sprite.SetPos({ 100.0f,100.0f,0.0f });

		//impl->model = Model::CreateOBJ("resources/Models/test/test.fbx");
		//impl->model = Model::CreateOBJ("resources/Models/cube/cube.fbx");
		impl->model = Model::CreateOBJ("resources/Models/test/Capoeira.fbx");
		//impl->model = Model::CreateOBJ("resources/Models/rail/rail.obj");
		// impl->model = Model::CreateOBJ("resources/Models/cube/cube.obj");
		impl->model->SetCamera(&impl->camera);
		impl->model->SetLight(&impl->light);    // ライト設定

		impl->model2 = Model::CreateOBJ("resources/Models/rail/rail.obj");
		impl->model2->SetCamera(&impl->camera);
		impl->model2->SetLight(&impl->light);   // ライト設定

	};

	bool Update() {

		// IMGUI uketuke kaisi
		ImGuiManager::GetInstance()->Begin();

		impl->camera.Update();

		//impl->triangle.Update();
		//impl->sprite.Update();
		impl->model->Update();
		impl->model2->Update();

		// 文字更新
		impl->text.Render();

		// test
		#ifdef USE_IMGUI	// test

	/*	ImGui::DragFloat3("pos", &impl->model2->GetWorldTransform().translate.x, 0.5f, 0.0f);
		ImGui::DragFloat3("rot", &impl->model2->GetWorldTransform().rotate.x, 0.5f, 0.0f);*/

		ImGui::DragFloat3("pos", &impl->model->GetWorldTransform().translate.x, 0.5f, 0.0f);
		ImGui::DragFloat3("rot", &impl->model->GetWorldTransform().rotate.x, 0.1f, 0.0f);
		ImGui::DragFloat3("sc ", &impl->model->GetWorldTransform().scale.x, 0.5f, 0.0f);

		ImGui::DragFloat3("pos2", &impl->model2->GetWorldTransform().translate.x, 0.5f, 0.0f);
		ImGui::DragFloat3("rot2", &impl->model2->GetWorldTransform().rotate.x, 0.5f, 0.0f);
		ImGui::DragFloat3("sc2 ", &impl->model2->GetWorldTransform().scale.x, 0.5f, 0.0f);

		ImGui::DragFloat4("lightColor", &impl->light.GetLightData()->color.x, 0.5f, 0.0f);
		ImGui::DragFloat3("lightDIrection", &impl->light.GetLightData()->direction.x, 0.1f, 0.0f);
		ImGui::DragFloat("lightIntensity", &impl->light.GetLightData()->intensity, 0.5f, 0.0f);

				/*ImGui::ShowDemoWindow();*/

		#endif


		ImGuiManager::GetInstance()->End();

		return true;
	};
	
	bool Draw() {

		// 描画開始
		DirectXCommon::GetInstance()->PreDraw();

		//impl->triangle.Draw(DirectXCommon::GetInstance()->GetCommandList().Get());
		//impl->sprite.Draw(DirectXCommon::GetInstance()->GetCommandList().Get());
		impl->model->Draw(DirectXCommon::GetInstance()->GetCommandList().Get());
		impl->model2->Draw(DirectXCommon::GetInstance()->GetCommandList().Get());

		// ２ｄ文字描画
		impl->text.drawString(U"testTexts", { 0.0f, 0.0f });

		// IMGUI byouga
		ImGuiManager::GetInstance()->Draw();

		// 描画終了
		DirectXCommon::GetInstance()->PostDraw();



		return true;
	}
	void Finalize() {

		WinApp::GetInstance()->Finalize();

		ImGuiManager::GetInstance()->Finalize();

		CoUninitialize();

	};
};