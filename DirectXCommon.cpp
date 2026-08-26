
#include "DirectXCommon.h"
#include "WinApp.h"
#include <format>

#include "DirectXUtility.h"

ComPtr<ID3D12Device> DirectXCommon::device_;
ComPtr<ID3D12GraphicsCommandList> DirectXCommon::commandList_;
ComPtr<ID3D12CommandQueue> DirectXCommon::commandQueue_;
ComPtr<IDXGISwapChain4> DirectXCommon::swapChain_;
ComPtr <ID3D12DescriptorHeap> DirectXCommon::srvHeap_;
ComPtr <ID3D12DescriptorHeap> DirectXCommon::rtvHeap_;
DXGI_FORMAT DirectXCommon::rtvDescFormat_;
UINT DirectXCommon::swapChainBufferCount_;
D3D12_DEPTH_STENCIL_DESC DirectXCommon::depthStencilDesc = {};

// 初期ばんごー
uint32_t DirectXCommon::srvIndex_ = 1;

DirectXCommon* DirectXCommon::GetInstance() {
	static DirectXCommon instance;
	return &instance;
}

/*=== 初期化（全部の関数を列挙）===*/
void DirectXCommon::Initialize() {

	CreateDXGIFactory();
	CreateAdapter();
	CreateDevice();
	CreateCommand();
	CreateSwapChain();
	CreateDescriptorHeap();
	CreateRTV();
	CreateDSV();
	CreateFence();

	// SRV作成.
	srvHeap_ = DirectXUtility::CreateDescriptorHeap(device_.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);

	//　ビューポートの設定
	//クライアント領域のサイズと一緒にして画面全体に表示
	viewport.Width = static_cast<FLOAT>(WinApp::GetInstance()->GetClientWidth());
	viewport.Height = static_cast<FLOAT>(WinApp::GetInstance()->GetClientHeight());
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	//基本的にビューポートと同じ矩形が構成されるようにする
	scissorRect.left = 0;

	scissorRect.right = WinApp::GetInstance()->GetClientWidth();
	scissorRect.top = 0;
	scissorRect.bottom = WinApp::GetInstance()->GetClientHeight();


	// 描画開始できる状態にする
	commandAllocator_->Reset();
	commandList_->Reset(commandAllocator_.Get(), nullptr);
};

/*=== DXGIFactoryの作成 ===*/
void DirectXCommon::CreateDXGIFactory() {

	HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory_));

	assert(SUCCEEDED(hr));
}

/*=== Adapterの作成 === */
void DirectXCommon::CreateAdapter() {

	// 一番いいの（良い順にアダプタ）を頼む
	for (UINT i = 0; dxgiFactory_->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter_)) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		DXGI_ADAPTER_DESC3 adapterDesc{};
		HRESULT hr = adapter_->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr));

		// ソフトウェアでなければ採用！
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {

			// ログで表示
			OutputDebugStringW(
				std::format(L"Use Adapter:{}\n", adapterDesc.Description).c_str());

			break;
		}
		adapter_ = nullptr;
	}

	// 適切なアダプタが見つからなかったので大丈夫じゃない、起動できない。
	assert(adapter_ != nullptr);
}

/*=== Deviceの作成 === */
void DirectXCommon::CreateDevice() {

	// 機能レベルとログ用の文字列
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_2,
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
	};
	const wchar_t* featureLevelStrings[] = {
		L"12.2",
		L"12.1",
		L"12.0",
	};

	// 高い順に生成できるか試す
	for (size_t i = 0; i < std::size(featureLevels); ++i)
	{
		HRESULT hr = D3D12CreateDevice(adapter_.Get(), featureLevels[i], IID_PPV_ARGS(&device_));

		if (SUCCEEDED(hr))
		{
			// ログで表示
				OutputDebugStringW(
					std::format(L"Feature Level: {}\n", featureLevelStrings[i]).c_str());

			break;
		}
	}
	// デバイスの生成が上手くいかないので起動不可
	assert(device_ != nullptr);

	// 初期化完了のログで表示
	OutputDebugStringW(
		std::format(L"Complete create D3D12Device!!!\n").c_str());
};

/*=== Command,CommandAlocatorの作成 === */
void DirectXCommon::CreateCommand() {

	// コマンドキューを作
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	HRESULT hr;

	hr = device_->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue_));

	assert(SUCCEEDED(hr));

	// コマンドアロケータを作成
	hr = device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator_));
	assert(SUCCEEDED(hr));

	// コマンドリストを作成
	hr = device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator_.Get(), nullptr, IID_PPV_ARGS(&commandList_));
	assert(SUCCEEDED(hr));

	// 初回分を閉じておく
	commandList_->Close();
}

/*=== SwapChainの作成 === */
void DirectXCommon::CreateSwapChain() {

	// スワップチェーンを生成
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = WinApp::GetInstance()->GetClientWidth();
	swapChainDesc.Height = WinApp::GetInstance()->GetClientHeight();
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //色
	swapChainDesc.SampleDesc.Count = 1; //マルチサンプルしない
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; //描画のターゲット
	swapChainDesc.BufferCount = 2; //ダブルバッファ
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; //フリップ後に前のバッファの内容は破棄

	// コマンドキューなど渡して生成
	HRESULT hr = dxgiFactory_->CreateSwapChainForHwnd(commandQueue_.Get(), WinApp::GetInstance()->GetHWND(), &swapChainDesc, nullptr, nullptr, (IDXGISwapChain1**)swapChain_.GetAddressOf());

	assert(SUCCEEDED(hr));

	// いれとく
	swapChainBufferCount_ = swapChainDesc.BufferCount;

}

/*=== DescriptorHeapの作成 === */
void DirectXCommon::CreateDescriptorHeap() {

	// ヒープづくり
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	descriptorHeapDesc.NumDescriptors = 2;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	HRESULT hr = device_.Get()->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&rtvHeap_));
	assert(SUCCEEDED(hr));

}

/*=== RTVの作成 === */
void DirectXCommon::CreateRTV() {

	// swapChainからResourceを取得
	HRESULT hr = swapChain_->GetBuffer(0, IID_PPV_ARGS(&renderTargets_[0]));

	assert(SUCCEEDED(hr));

	hr = swapChain_->GetBuffer(1, IID_PPV_ARGS(&renderTargets_[1]));

	assert(SUCCEEDED(hr));


	// RTVの設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	// ディスクリプタヒープからCPUハンドルを取得
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvHeap_->GetCPUDescriptorHandleForHeapStart();

	// 2つのRTVを作成

	// まず一つ目。
	rtvHandles[0] = rtvStartHandle;
	device_->CreateRenderTargetView(renderTargets_[0].Get(), &rtvDesc, rtvHandles[0]);

	// 二つ目のハンドルを得る
	rtvHandles[1] = rtvStartHandle;
	rtvHandles[1].ptr = rtvHandles[0].ptr + device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// ２つ目。
	device_->CreateRenderTargetView(renderTargets_[1].Get(), &rtvDesc, rtvHandles[1]);

	// いれとく
	rtvDescFormat_ = rtvDesc.Format;

};

/*=== DSV(Depth)の作成 === */
void DirectXCommon::CreateDSV() {

	// DepthStencilTextureを作る
	depthStencilResource = DirectXUtility::CreateDepthStencilTexture(
		device_.Get(), 
		WinApp::GetInstance()->GetClientWidth(),
		WinApp::GetInstance()->GetClientHeight()
	);

	//DSV用のヒープでディスクリプタの数は1。DSVはShader内で触るものではないので、ShaderVisibleはfalse
	dsvDescriptorHeap = DirectXUtility::CreateDescriptorHeap(device_.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);

	// DSVの設定
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//Format。基本的にはResourceに合わせる
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D; // 2dTexture

	//DSVHeapの先頭にDSVをつくる
	device_->CreateDepthStencilView(depthStencilResource.Get(), &dsvDesc, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	//DepthStencilStateの設定
	//Depthの機能を有効化する
	depthStencilDesc.DepthEnable = true;
	//書き込みします
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	//比較関数はLessEqual。つまり、近ければ描画される
	//depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;



}
/*=== Fenceの作成 === */
void DirectXCommon::CreateFence()
{

	// 初期値０でfenceを作成
	fenceValue = 0;
	HRESULT hr = device_->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));

	assert(SUCCEEDED(hr));

	// Fenceのsignalを待つためのイベントを作成
	HANDLE fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

	assert(fenceEvent != nullptr);
//#ifdef USE_IMGUI
//	// IMGUIのおまじない　こういうもんである
//	IMGUI_CHECKVERSION();
//	ImGui::CreateContext();
//	ImGui::StyleColorsDark();
//	ImGui_ImplWin32_Init(hWnd);
//	ImGui_ImplDX12_Init(device.Get(),
//		swapChainDesc.BufferCount,
//		rtvDesc.Format,
//		srvHeap.Get(),
//		srvHeap->GetCPUDescriptorHandleForHeapStart(),
//		srvHeap->GetGPUDescriptorHandleForHeapStart());
//	ImGuiIO& io = ImGui::GetIO();
//	io.Fonts->Build();
//#endif

}

// 描画前に
void DirectXCommon::PreDraw() {

	//これから書き込むバックバッファのインデックスを取得
	UINT backBufferIndex = swapChain_->GetCurrentBackBufferIndex();


	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

	// バリアーの対象
	barrier.Transition.pResource = renderTargets_[backBufferIndex].Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

	// ---バリアーを張る
	commandList_->ResourceBarrier(1, &barrier);



	//指定した色で画面全体をクリアする
	float clearColor[] = { 0.1f, 0.25f, 0.5f,1.0f };// 青色。RGBAの順
	commandList_->ClearRenderTargetView(rtvHandles[backBufferIndex], clearColor, 0, nullptr);

	// 画面の深さ？
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	commandList_->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	//描画先のRTVを設定する
	commandList_->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, &dsvHandle);

	// ビューの設定
	commandList_->RSSetViewports(1, &viewport);
	commandList_->RSSetScissorRects(1, &scissorRect);
};

// 描画後に
void DirectXCommon::PostDraw() {

	// 5. Presentに戻す
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	commandList_->ResourceBarrier(1, &barrier);

	//コマンドリストの内容を確定させる。
	HRESULT hr = commandList_->Close();
	assert(SUCCEEDED(hr));

	///GPUにコマンドリストの実行を行わせる
	ID3D12CommandList* commandLists[] = { commandList_.Get()};

	commandQueue_->ExecuteCommandLists(1, commandLists);

	//GPUとOSに画面の交換を行う
	swapChain_->Present(1, 0);

	// 完了を待つ
	fenceValue++;
	commandQueue_->Signal(fence_.Get(), fenceValue);
	if (fence_->GetCompletedValue() < fenceValue) {
		fence_->SetEventOnCompletion(fenceValue, fenceEvent_);
		WaitForSingleObject(fenceEvent_, INFINITE);
	}

	// 次のフレーム用のコマンドリストを準備
	hr = commandAllocator_->Reset();
	assert(SUCCEEDED(hr));
	hr = commandList_->Reset(commandAllocator_.Get(), nullptr);
	assert(SUCCEEDED(hr));
};

