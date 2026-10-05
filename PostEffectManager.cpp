
#include "PostEffectManager.h"

#include "PipelineStateManager.h"

#include "DirectXCommon.h"
#include "WinApp.h"
#include "DirectXUtility.h"

// （安心安定の）シングルトン
PostEffectManager* PostEffectManager::GetInstance() {
	static PostEffectManager instance;
	return &instance;
}

void PostEffectManager::Initialize() {

	device_ = DirectXCommon::GetInstance()->GetDevice().Get();

	// ポストエフェクト用の頂点リソースを作る
	vertexResourceScreen = DirectXUtility::CreateBufferResource(device_, sizeof(VertexData) * 6);

	//頂点バッファビューを作成する
	vertexBufferViewScreen.BufferLocation =
		vertexResourceScreen->GetGPUVirtualAddress();

	vertexBufferViewScreen.SizeInBytes = sizeof(VertexData) * 6;
	//1頂点あたりのサイズ
	vertexBufferViewScreen.StrideInBytes = sizeof(VertexData);

	VertexData* vertexDataScreen = nullptr;
	vertexResourceScreen->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataScreen));
	// 左下
	vertexDataScreen[0].position = { 0.0f,   720.0f, 0.0f, 1.0f };
	vertexDataScreen[0].texcoord = { 0.0f, 1.0f };

	// 左上
	vertexDataScreen[1].position = { 0.0f,   0.0f, 0.0f, 1.0f };
	vertexDataScreen[1].texcoord = { 0.0f, 0.0f };

	// 右下
	vertexDataScreen[2].position = { 1280.0f, 720.0f, 0.0f, 1.0f };
	vertexDataScreen[2].texcoord = { 1.0f, 1.0f };

	// 左上
	vertexDataScreen[3].position = { 0.0f,   0.0f, 0.0f, 1.0f };
	vertexDataScreen[3].texcoord = { 0.0f, 0.0f };

	// 右上
	vertexDataScreen[4].position = { 1280.0f, 0.0f, 0.0f, 1.0f };
	vertexDataScreen[4].texcoord = { 1.0f, 0.0f };

	// 右下
	vertexDataScreen[5].position = { 1280.0f, 720.0f, 0.0f, 1.0f };
	vertexDataScreen[5].texcoord = { 1.0f, 1.0f };



	// オフスクリーンRTV用ヒープ
	rtvHeapOffscreen = DirectXUtility::CreateDescriptorHeap(device_, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1, false);

	// オフスクリーン用テクスチャ作成
	D3D12_RESOURCE_DESC rtvDescOffscreen{};
	rtvDescOffscreen.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	rtvDescOffscreen.Width = WinApp::GetInstance()->GetClientWidth();
	rtvDescOffscreen.Height = WinApp::GetInstance()->GetClientHeight();
	rtvDescOffscreen.DepthOrArraySize = 1;
	rtvDescOffscreen.MipLevels = 1;
	rtvDescOffscreen.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDescOffscreen.SampleDesc.Count = 1;
	rtvDescOffscreen.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	D3D12_HEAP_PROPERTIES heapProps{};
	heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	clearValue.Color[0] = 0.1f; clearValue.Color[1] = 0.25f; clearValue.Color[2] = 0.5f; clearValue.Color[3] = 1.0f;

	device_->CreateCommittedResource(
		&heapProps, D3D12_HEAP_FLAG_NONE,
		&rtvDescOffscreen, D3D12_RESOURCE_STATE_RENDER_TARGET,
		&clearValue, IID_PPV_ARGS(&offscreenRenderTarget)
	);

	// RTV作成
	D3D12_RENDER_TARGET_VIEW_DESC rtvDescOff{};
	rtvDescOff.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDescOff.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	device_->CreateRenderTargetView(offscreenRenderTarget, &rtvDescOff, rtvHeapOffscreen->GetCPUDescriptorHandleForHeapStart());

	// SRV作成（ポストエフェクトで使う）
	offscreenSRVHandleCPU = DirectXUtility::GetCPUDescriptorHandle(DirectXCommon::GetSrvHeap().Get(), DirectXCommon::GetAllocateSrv(), 3); // 3番目に確保
	offscreenSRVHandleGPU = DirectXUtility::GetGPUDescriptorHandle(DirectXCommon::GetSrvHeap().Get(), DirectXCommon::GetAllocateSrv(), 3);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDescOff{};
	srvDescOff.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	srvDescOff.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDescOff.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDescOff.Texture2D.MipLevels = 1;
	device_->CreateShaderResourceView(offscreenRenderTarget, &srvDescOff, offscreenSRVHandleCPU);



	// ポストエフェクト
	postEffectResource = DirectXUtility::CreateBufferResource(device_, sizeof(PostEffectParam));

	postEffectResource->Map(
		0,
		nullptr,
		reinterpret_cast<void**>(&postEffectData)
	);

	postEffectData->blurPower = 1.0f;

	postEffectData->time = 0.0f;
	postEffectData->vtWavePower = 0.0f;
	postEffectData->hzWavePower = 0.0f;

	postEffectData->color = { 1.0f, 1.0f, 1.0f, 1.0f };  // 最初は白（変化なし）

	postEffectData->monotone = 0.0f;
	postEffectData->colorInversion = 0.0f;

	postEffectData->neon = 0.0f;
	postEffectData->noise = 0.0f;
	postEffectData->fishLens = 0.0f;

}

// ポストエフェクト受付開始
void PostEffectManager::BeginScene() {

	// オフスクリーンRTVに切り替え
	ID3D12GraphicsCommandList* commandList = DirectXCommon::GetCommandList().Get();
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandleOffscreen = rtvHeapOffscreen->GetCPUDescriptorHandleForHeapStart();
	commandList->OMSetRenderTargets(1, &rtvHandleOffscreen, false, nullptr);

	// クリア
	float clearColor[] = { 0.1f, 0.25f, 0.5f, 1.0f };
	commandList->ClearRenderTargetView(rtvHandleOffscreen, clearColor, 0, nullptr);

}

// ポストエフェクト受付終了(描画
void PostEffectManager::ApplyScene() {

	ID3D12GraphicsCommandList* commandList = DirectXCommon::GetInstance()->GetCommandList().Get();

		// オフスクリーンを読める状態に
		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = offscreenRenderTarget;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		commandList->ResourceBarrier(1, &barrier);

		// これから書き込むRTVのインデックス
		UINT backBufferIndex = DirectXCommon::GetSwapChain()->GetCurrentBackBufferIndex();

		barrier.Transition.pResource = DirectXCommon::GetBackBuffer(backBufferIndex).Get();
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		commandList->ResourceBarrier(1, &barrier);
	
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = DirectXCommon::GetBackBufferRTV(backBufferIndex);
		commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

		//RootSignatureを設定。PSOに設定しているけど別途設定が必要
		//commandList->SetGraphicsRootSignature(PipelineStateManager::GetInstance()->GetPipelineState(PipelineType::Sprite).rootSignature);
		
		// PSOを設定
		commandList->SetPipelineState(PipelineStateManager::GetInstance()->GetPipelineState(PipelineType::PostEffect).graphicsPipelineState);

		commandList->IASetVertexBuffers(0, 1, &vertexBufferViewScreen);
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// ポストエフェクト　テキスチャ
		commandList->SetGraphicsRootDescriptorTable(0, offscreenSRVHandleGPU);

		// ポストエフェクト　パラメータ値
		commandList->SetGraphicsRootConstantBufferView(1, postEffectResource->GetGPUVirtualAddress());

		commandList->DrawInstanced(6, 1, 0, 0);
	
}