#include "ImGuiManager.h"

#include "WinApp.h"
#include "DirectXCommon.h"

ImGuiManager* ImGuiManager::GetInstance() {
	static ImGuiManager instance;
	return &instance;
}

// 初期化
void ImGuiManager::Initialize() {

#ifdef USE_IMGUI

	// ながいからこうした
	auto* dx = DirectXCommon::GetInstance();

	// IMGUIのおまじない　こういうもんである
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(WinApp::GetInstance()->GetHWND());
	ImGui_ImplDX12_Init(dx->GetDevice().Get(),
		dx->GetSwapChainBufferCount(),
		dx->GetRtvDescFormat(),
		dx->GetSrvHeap().Get(),
		dx->GetSrvHeap()->GetCPUDescriptorHandleForHeapStart(),
		dx->GetSrvHeap()->GetGPUDescriptorHandleForHeapStart());
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->Build();

#endif
};

// 受付開始
void ImGuiManager::Begin() {
#ifdef USE_IMGUI
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();


#endif
};

// 受付終了
void ImGuiManager::End() {
#ifdef USE_IMGUI
	ImGui::Render();
#endif
};

// 描画
void ImGuiManager::Draw() {
#ifdef USE_IMGUI
	ID3D12DescriptorHeap* heaps[] = {
	DirectXCommon::GetInstance()->GetSrvHeap().Get()
	};

	DirectXCommon::GetInstance()->GetCommandList()->SetDescriptorHeaps(1, heaps);


	// imguiの描画command
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), DirectXCommon::GetInstance()->GetCommandList().Get());

	//auto* drawData = ImGui::GetDrawData();
	//assert(drawData);
	//assert(drawData->CmdListsCount > 0);
#endif
};

// 終了処理
void ImGuiManager::Finalize() {

#ifdef USE_IMGUI
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

#endif
}