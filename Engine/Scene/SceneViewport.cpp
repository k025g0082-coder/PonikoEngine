#include "SceneViewport.h"
#include "PostEffectManager.h"

#include "ImGuiManager.h"

void SceneViewport::Initialize(uint32_t width, uint32_t height)
{
	width_ = width;
	height_ = height;

	camera_.Initialize();

	// ちょい奥
	camera_.GetWorldPosition().z = -20.0f;


	//* RenderTarget初期化 ユニークポインタではこうやる
	renderTarget_ = std::make_unique<RenderTarget>();
	renderTarget_->Initialize(width,height);

}

void SceneViewport::Update()
{

	// モデルアプデ
	for (Model* model : models_) {
		model->Update();
	}

	camera_.Update();

}

void SceneViewport::Draw()
{

	/* 描画開始. バリアーを専用のRTVに遷移させる */
	renderTarget_->BeginRender();

	// モデル描画。
	// 
	//  20261023 なせかこれをDrawするとDrawの際にモデルが超白くなるバグ？あったのでOFFに
	// 
	//for (Model* model : models_) {
	//	model->Draw(camera_);
	//}

	/* 描画終了. バリアーを元のPIXELSHADEＲに遷移させる */
	renderTarget_->EndRender();
	

	//* 次の描画先を、PostEffect用に設定し直し
	ID3D12GraphicsCommandList* commandList = DirectXCommon::GetInstance()->GetCommandList().Get();

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandleOffscreen = PostEffectManager::GetInstance()->GetRtvHandleOffscreen();
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = DirectXCommon::GetInstance()->GetDsvHeap()->GetCPUDescriptorHandleForHeapStart();

	// 転移
	commandList->OMSetRenderTargets(1, &rtvHandleOffscreen, FALSE, &dsvHandle);

	commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// ビューポートも戻す
	commandList->RSSetViewports(1, &DirectXCommon::GetInstance()->GetViewport());
	commandList->RSSetScissorRects(1, &DirectXCommon::GetInstance()->GetScissorRect());
}

void SceneViewport::ImGuiView()
{
#ifdef USE_IMGUI
	
	ImGui::SeparatorText("ViewPropaty");

	ImGui::DragFloat3("camera Pos", &camera_.GetWorldTransform().translation_.x, 0.1f);
	ImGui::DragFloat3("camera Rot", &camera_.GetWorldTransform().rotation_.x, 0.1f);
#endif
}
