#include "RenderTarget.h"
#include "PipelineStateManager.h"

#include "ImGuiManager.h"

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
	
	viewDrawRect_.right = width_;
	viewDrawRect_.bottom = width_;

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

	// 初期位置・矩形設定
	viewDrawRect_ = { 0, width_, 0, height_ };
	SetDrawRect(viewDrawRect_);


	firstFrame_ = true; // 初回フラグをリセット
}

void RenderTarget::BeginRender(bool clear, Vector4 clearColor, bool depth)
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
	


	/* もし深度DSVが設定されていれば・・ */
	if (dsvCPUHandle.ptr) {
		commandList->OMSetRenderTargets(1, &rtvHandleOffscreen, FALSE, &dsvCPUHandle);
	}
	else {

		// ない場合共通のを使う
		D3D12_CPU_DESCRIPTOR_HANDLE defaultDsvHandle = DirectXCommon::GetInstance()->GetDsvHeap()->GetCPUDescriptorHandleForHeapStart();

		commandList->OMSetRenderTargets(1, &rtvHandleOffscreen, FALSE, &defaultDsvHandle);
	}
	

	if (clear) {

		// クリア時のカラー
		float color[] = { 
			clearColor.x,
			clearColor.y, 
			clearColor.z, 
			clearColor.w 
		};

		commandList->ClearRenderTargetView(rtvHandleOffscreen, color, 0, nullptr);
	}

	// 画面の深さ初期化？ これがあるとfogが動かないので
	if (depth) {
		commandList->ClearDepthStencilView(dsvCPUHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	}

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

void RenderTarget::Resize(int32_t width, int32_t height, Vector2 offset)
{
	width_ = width;
	height_ = height;

	baseWidth_ = width;
	baseHeight_ = height;


	// GPUが旧リソースを使用していないことを確認してから解放
	renderTarget_.Reset();

	// 新しいテクスチャの設定
	D3D12_RESOURCE_DESC desc{};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	desc.Width = width;
	desc.Height = height;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	desc.SampleDesc.Count = 1;
	desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	D3D12_HEAP_PROPERTIES heapProps{};
	heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Format = desc.Format;
	clearValue.Color[0] = 0.1f;
	clearValue.Color[1] = 0.25f;
	clearValue.Color[2] = 0.5f;
	clearValue.Color[3] = 1.0f;

	HRESULT hr = device_->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		&clearValue,
		IID_PPV_ARGS(&renderTarget_)
	);
	assert(SUCCEEDED(hr));

	// 既存のRTVを更新
	D3D12_RENDER_TARGET_VIEW_DESC rtvDescOff{};
	rtvDescOff.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDescOff.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	device_->CreateRenderTargetView(
		renderTarget_.Get(),
		&rtvDescOff,
		rtvHeap_->GetCPUDescriptorHandleForHeapStart()
	);

	// ★修正箇所: 既存のSRVを更新（nullptr ではなく明示的に desc を渡す）
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDescOff{};
	srvDescOff.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	srvDescOff.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDescOff.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDescOff.Texture2D.MipLevels = 1;

	device_->CreateShaderResourceView(
		renderTarget_.Get(),
		&srvDescOff,
		srvHandleCPU_
	
	);


	// Viewport
	viewport.TopLeftX = offset.x;
	viewport.TopLeftY = offset.y;
	viewport.Width = static_cast<float>(width);
	viewport.Height = static_cast<float>(height);

	// 描画区域の大きさも更新（オフセットXYは０にすることで一緒についてくる）
	viewDrawRect_ = { 
		0,
		static_cast<int32_t>(width_),
		0, 
	    static_cast<int32_t>(height_) };

	offset_ = offset;

	// 矩形（描画区域）も更新
	SetDrawRect(viewDrawRect_);

	//firstFrame_ = true;

}

void RenderTarget::ChangeViewResolution(float value_)
{
	resolution_ = value_;

	// 元の解像度と掛けて 計算
	width_= static_cast<uint32_t>(static_cast<float>(baseWidth_) * value_);
	height_ = static_cast<uint32_t>(static_cast<float>(baseHeight_) * value_);

	/* ビューポートも換える */
	viewport.Width = static_cast<float>(width_);
	viewport.Height = static_cast<float>(height_);

}

void RenderTarget::SetViewOffset(Vector2 offset)
{
	offset_ = offset;

	// 矩形（描画区域）も更新
	SetDrawRect(viewDrawRect_);
}
void RenderTarget::SetDrawRect(Rect rect)
{
	viewDrawRect_ = rect;

	// ① テクスチャ内部用（BeginRenderで使用）：オフセットなしの原点(0,0)基準
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = static_cast<float>(width_);
	viewport.Height = static_cast<float>(height_);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	scissorRect.left = static_cast<LONG>(rect.left);
	scissorRect.right = static_cast<LONG>(rect.right);
	scissorRect.top = static_cast<LONG>(rect.top);
	scissorRect.bottom = static_cast<LONG>(rect.bottom);

	// ② バックバッファ出力用（DrawCompositeToBackBufferで使用）：offset_ を反映
	outViewport.TopLeftX = offset_.x;
	outViewport.TopLeftY = offset_.y;
	outViewport.Width = static_cast<float>(width_);
	outViewport.Height = static_cast<float>(height_);
	outViewport.MinDepth = 0.0f;
	outViewport.MaxDepth = 1.0f;

	// outViewport.TopLeftX/Y に offset_.x/y が既に入っているため、
	// シザー領域の開始地点（left/top）は offset_.x + rect.left に設定し、幅（right）もそれに合わせます
	outScissorRect.left = static_cast<LONG>(offset_.x + rect.left);
	outScissorRect.right = static_cast<LONG>(offset_.x + rect.right);
	outScissorRect.top = static_cast<LONG>(offset_.y + rect.top);
	outScissorRect.bottom = static_cast<LONG>(offset_.y + rect.bottom);
}


void RenderTarget::ImGuiView()
{
#ifdef USE_IMGUI
	ImGui::SeparatorText("RenderTarget");

	bool isChangeWindow = false;

	// ビューのサイズ
	isChangeWindow |= ImGui::DragInt("Width", &width_, 1, 1, baseWidth_);
	isChangeWindow |= ImGui::DragInt("Height", &height_, 1, 1, baseHeight_);

	// ビューのオフセット
	isChangeWindow |= ImGui::DragFloat2("Offset", &offset_.x, 1.0f);

	if (isChangeWindow) {
		Resize(width_, height_, offset_);
	}


	// 描画範囲
	ImGui::SeparatorText("Draw Rect");

	bool isChangeRect = false;
	isChangeRect |= ImGui::DragInt("Left", &viewDrawRect_.left, 1);
	isChangeRect |= ImGui::DragInt("Right", &viewDrawRect_.right, 1);
	isChangeRect |= ImGui::DragInt("Top", &viewDrawRect_.top, 1);
	isChangeRect |= ImGui::DragInt("Bottom", &viewDrawRect_.bottom, 1);

	if (isChangeRect) {
		SetDrawRect(viewDrawRect_);
	}

	ImGui::Separator();

#endif
}
