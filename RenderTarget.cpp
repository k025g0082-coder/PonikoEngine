#include "RenderTarget.h"
#include "PipelineStateManager.h"

#include "DirectXCommon.h"
#include "WinApp.h"
#include "DirectXUtility.h"

void RenderTarget::Initialize(int32_t width, int32_t height, float viewResolution) {

	// 元の
	baseWidth_ = width;
	baseHeight_ = height;

	// 解像度計算
	width_ = static_cast<int32_t>(static_cast<float>(width) * viewResolution);
	height_ = static_cast<int32_t>(static_cast<float>(height) * viewResolution);

	device_ = DirectXCommon::GetInstance()->GetDevice().Get();

	// RTV用ヒープ
	rtvHeap_ = DirectXUtility::CreateDescriptorHeap(device_.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1, false);

	// テクスチャ作成
	D3D12_RESOURCE_DESC rtvDescOffscreen{};
	rtvDescOffscreen.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	rtvDescOffscreen.Width = width_;
	rtvDescOffscreen.Height = height_;
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
		&clearValue, IID_PPV_ARGS(&renderTarget_)
	);

	// RTV作成
	D3D12_RENDER_TARGET_VIEW_DESC rtvDescOff{};
	rtvDescOff.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDescOff.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	device_->CreateRenderTargetView(renderTarget_.Get(), &rtvDescOff, rtvHeap_->GetCPUDescriptorHandleForHeapStart());



	// SRVの番号を割り振る
	UINT index = DirectXCommon::GetInstance()->GetAllocateSrv();

	UINT descriptorSize =
		DirectXCommon::GetInstance()
		->GetDevice()
		->GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	srvHandleCPU_ =
		DirectXUtility::GetCPUDescriptorHandle(
			DirectXCommon::GetInstance()->GetSrvHeap().Get(),
			descriptorSize,
			index);

	srvHandleGPU_ =
		DirectXUtility::GetGPUDescriptorHandle(
			DirectXCommon::GetInstance()->GetSrvHeap().Get(),
			descriptorSize,
			index);


	D3D12_SHADER_RESOURCE_VIEW_DESC srvDescOff{};
	srvDescOff.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	srvDescOff.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDescOff.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDescOff.Texture2D.MipLevels = 1;

	device_->CreateShaderResourceView(renderTarget_.Get(), &srvDescOff, srvHandleCPU_);

	//　ビューポートの設定  設定された大きさで生成
	viewport.Width = static_cast<FLOAT>(width_);
	viewport.Height = static_cast<FLOAT>(height_);
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	//基本的にビューポートと同じ矩形が構成されるようにする
	scissorRect.left = 0;

	scissorRect.right = width_;
	scissorRect.top = 0;
	scissorRect.bottom = height_;

	firstFrame_ = true; // 初回フラグをリセット
}

void RenderTarget::BeginRender(bool clear)
{

	ID3D12GraphicsCommandList* commandList = DirectXCommon::GetCommandList().Get();

	if (!firstFrame_) {

		// PIXELSHADERを描画可能に戻す
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

		barrier.Transition.pResource =
			renderTarget_.Get();

		barrier.Transition.Subresource =
			D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		barrier.Transition.StateBefore =
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

		barrier.Transition.StateAfter =
			D3D12_RESOURCE_STATE_RENDER_TARGET;

		commandList->ResourceBarrier(1, &barrier);
	}
	firstFrame_ = false;

	// オフスクリーンRTVに切り替え
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandleOffscreen = rtvHeap_->GetCPUDescriptorHandleForHeapStart();
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = DirectXCommon::GetInstance()->GetDsvHeap()->GetCPUDescriptorHandleForHeapStart();

	commandList->OMSetRenderTargets(1, &rtvHandleOffscreen, FALSE, &dsvHandle);

	if (clear) {
		// クリア
		float clearColor[] = { 0.0f, 0.0f, 0.0f,0.0f };// 黒色で初期化
		commandList->ClearRenderTargetView(rtvHandleOffscreen, clearColor, 0, nullptr);
	}

	// 画面の深さ？
	commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	/* 初期化時に設定された、指定の大きさのビューポートを設定する */
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);

}

// RENDER_TARGET->PIXELSHADERで元に戻す
void RenderTarget::EndRender()
{
	ID3D12GraphicsCommandList* commandList = DirectXCommon::GetInstance()->GetCommandList().Get();

	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = renderTarget_.Get();//							*
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;//          戻
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;//   す
	commandList->ResourceBarrier(1, &barrier);

	// 下みたいなのはViewPortなどで転移しようね
	
	//commandList->OMSetRenderTargets(
	//	1,
	//	&rtvHandle_,
	//	FALSE,
	//	nullptr);
	//
}

void RenderTarget::ChangeViewResolution(float value_)
{
	
	// 元の解像度と掛けて 計算
	width_= static_cast<uint32_t>(static_cast<float>(baseWidth_) * value_);
	height_ = static_cast<uint32_t>(static_cast<float>(baseHeight_) * value_);

	/* ビューポートも帰る */
	viewport.Width = static_cast<float>(width_);
	viewport.Height = static_cast<float>(height_);

	// 矩形も
	scissorRect.right = static_cast<LONG>(width_);
	scissorRect.bottom = static_cast<LONG>(height_);
}

