#include "PonikoEngine.h"
#include "PipelineStateManager.h"
#include "ImGuiManager.h"

#include <memory>

// ここにメンバを書く
namespace {
	class EngineImpl { // 内部実装、の略
	public:
		Triangle triangle;
		Sprite sprite;
		Model* model;
		Model* model2;
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

		// Imgui初期化
		ImGuiManager::GetInstance()->Initialize();

		// カメラ
		impl->camera.Initialize();
	
		// ライト設定
		DirectionalLightParam data ={
			.color = { 1.0f, 1.0f, 1.0f, 1.0f },
			.direction = { 0.0f, 0.0f, 0.0f },
			.intensity = 5.0f
		};

		impl->light.Initialize(data);


		impl->triangle.Initialize(DirectXCommon::GetInstance()->GetDevice().Get());
		impl->triangle.SetCamera(&impl->camera);

		impl->sprite.Initialize(DirectXCommon::GetInstance()->GetDevice().Get());

		impl->sprite.SetPos({ 100.0f,100.0f,0.0f });

		//impl->model = Model::CreateOBJ("resources/Models/cube/cube.fbx");
		//impl->model = Model::CreateOBJ("resources/Models/test/test.fbx");
		impl->model = Model::CreateOBJ("resources/Models/rail/rail.obj");
		// impl->model = Model::CreateOBJ("resources/Models/cube/cube.obj");
		impl->model->SetCamera(&impl->camera);
		impl->model->SetLight(&impl->light);    // ライト設定

		//impl->model2 = Model::CreateOBJ("resources/Models/cube/cube.obj");
		//impl->model2->SetCamera(&impl->camera);
		//impl->model2->SetLight(&impl->light);   // ライト設定



	};

	bool Update() {

		// IMGUI uketuke kaisi
		ImGuiManager::GetInstance()->Begin();

		impl->camera.Update();

		//impl->triangle.Update();
		//impl->sprite.Update();
		impl->model->Update();
		//impl->model2->Update();

		// test
		#ifdef USE_IMGUI	// test

	/*	ImGui::DragFloat3("pos", &impl->model2->GetWorldTransform().translate.x, 0.5f, 0.0f);
		ImGui::DragFloat3("rot", &impl->model2->GetWorldTransform().rotate.x, 0.5f, 0.0f);*/

		ImGui::DragFloat3("pos", &impl->model->GetWorldTransform().translate.x, 0.5f, 0.0f);
		ImGui::DragFloat3("rot", &impl->model->GetWorldTransform().rotate.x, 0.1f, 0.0f);
		ImGui::DragFloat3("sc ", &impl->model->GetWorldTransform().scale.x, 0.5f, 0.0f);

		//ImGui::DragFloat3("pos", &impl->model2->GetWorldTransform().translate.x, 0.5f, 0.0f);
		//ImGui::DragFloat3("rot", &impl->model2->GetWorldTransform().rotate.x, 0.5f, 0.0f);
		//ImGui::DragFloat3("sc ", &impl->model2->GetWorldTransform().scale.x, 0.5f, 0.0f);

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
		//impl->model2->Draw(DirectXCommon::GetInstance()->GetCommandList().Get());

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