
#include "PostEffectManager.h"

#include "PipelineStateManager.h"
#include "ImGuiManager.h"

#include "DirectXCommon.h"
#include "WinApp.h"
#include "DirectXUtility.h"

#include "timer.h"

#include "Scene.h"

#include <format>

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
	vertexDataScreen[0].position = { -1.0f, -1.0f, 0.0f, 1.0f };
	vertexDataScreen[0].texcoord = { 0.0f, 1.0f };

	// 左上
	vertexDataScreen[1].position = { -1.0f,  1.0f, 0.0f, 1.0f };
	vertexDataScreen[1].texcoord = { 0.0f, 0.0f };

	// 右下
	vertexDataScreen[2].position = { 1.0f, -1.0f, 0.0f, 1.0f };
	vertexDataScreen[2].texcoord = { 1.0f, 1.0f };

	// 左上
	vertexDataScreen[3].position = { -1.0f,  1.0f, 0.0f, 1.0f };
	vertexDataScreen[3].texcoord = { 0.0f, 0.0f };

	// 右上
	vertexDataScreen[4].position = { 1.0f,  1.0f, 0.0f, 1.0f };
	vertexDataScreen[4].texcoord = { 1.0f, 0.0f };

	// 右下
	vertexDataScreen[5].position = { 1.0f, -1.0f, 0.0f, 1.0f };
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
	device_->CreateRenderTargetView(offscreenRenderTarget.Get(), &rtvDescOff, rtvHeapOffscreen->GetCPUDescriptorHandleForHeapStart());


	UINT index = DirectXCommon::GetInstance()->GetAllocateSrv();

	UINT descriptorSize =
		DirectXCommon::GetInstance()
			->GetDevice()
			->GetDescriptorHandleIncrementSize(
				D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	offscreenSRVHandleCPU =
		DirectXUtility::GetCPUDescriptorHandle(
			DirectXCommon::GetInstance()->GetSrvHeap().Get(),
			descriptorSize,
			index);

	offscreenSRVHandleGPU =
		DirectXUtility::GetGPUDescriptorHandle(
			DirectXCommon::GetInstance()->GetSrvHeap().Get(),
			descriptorSize,
			index);


	D3D12_SHADER_RESOURCE_VIEW_DESC srvDescOff{};
	srvDescOff.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	srvDescOff.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDescOff.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDescOff.Texture2D.MipLevels = 1;
	device_->CreateShaderResourceView(offscreenRenderTarget.Get(), &srvDescOff, offscreenSRVHandleCPU);


	//*  BloomのRenderTarget初期化  *//

	/* 解像度は２分の一。bloom合成時に元に戻す */

	//blurRenderTarget_.Initialize(WinApp::GetInstance()->GetClientWidth(), WinApp::GetInstance()->GetClientHeight(), kBloomResolution_);
	bloomRenderTarget_.Initialize(WinApp::GetInstance()->GetClientWidth(), WinApp::GetInstance()->GetClientHeight(), kBloomResolution_);


	// 定数バッファの登録
	bloomParamResource = DirectXUtility::CreateBufferResource(device_, sizeof(BloomParam));

	bloomParamResource->Map(
		0,
		nullptr,
		reinterpret_cast<void**>(&bloomParamData_)
	);

	// 初期値
	bloomParamData_->bloomIntensity = 0.8f;
	bloomParamData_->bloomSoftness = 0.4f;
	bloomParamData_->bloomThreshold = 0.1f;
	bloomParamData_->blurTexelSize = 2.0f;


	
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
	postEffectData->glitch = 0.0f;
	postEffectData->scanLine = 0.0f;
	postEffectData->fishLens = 0.0f;

	/* blur初期化 */
	InitializeBlur();
	

	OutputDebugStringA(std::format(
		"offscreenSRVHandleGPU.ptr = 0x{:X}\n",
		offscreenSRVHandleGPU.ptr
	).c_str());
}

// ポストエフェクト受付開始
void PostEffectManager::BeginScene() {

	postEffectData->time += Timer::GetDeltaTime();

	ID3D12GraphicsCommandList* commandList = DirectXCommon::GetCommandList().Get();

	if (!firstFrame_) {
		// オフスクリーンを描画可能に戻す
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

		barrier.Transition.pResource =
			offscreenRenderTarget.Get();

		barrier.Transition.StateBefore =
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

		barrier.Transition.StateAfter =
			D3D12_RESOURCE_STATE_RENDER_TARGET;

		commandList->ResourceBarrier(1, &barrier);
	}

	firstFrame_ = false;

	// オフスクリーンRTVに切り替え
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandleOffscreen = rtvHeapOffscreen->GetCPUDescriptorHandleForHeapStart();
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = DirectXCommon::GetInstance()->GetDsvHeap()->GetCPUDescriptorHandleForHeapStart();

	commandList->OMSetRenderTargets(1, &rtvHandleOffscreen, FALSE, &dsvHandle);

	// クリア
	float clearColor[] = { 0.1f, 0.25f, 0.5f,1.0f };// 青色。RGBAの順
	commandList->ClearRenderTargetView(rtvHandleOffscreen, clearColor, 0, nullptr);

	commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	commandList->RSSetViewports(1, &DirectXCommon::GetInstance()->GetViewport());
	commandList->RSSetScissorRects(1, &DirectXCommon::GetInstance()->GetScissorRect());

	

}

// ポストエフェクト受付終了(描画
void PostEffectManager::ApplyScene() {

	ID3D12GraphicsCommandList* commandList = DirectXCommon::GetInstance()->GetCommandList().Get();

		// オフスクリーンを読める状態に
		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = offscreenRenderTarget.Get();
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		commandList->ResourceBarrier(1, &barrier);


		/* 
			ブラー処理 

			1) 明るいところを摘出
			2) 明るいところをぼかす
			3) 最終的なBuckBufferへの描画。
		　    　backBufferを描画先にして、blurと既存のポストエフェクト効果を掛け合わせたのを描画

		*/

		// 0.5)* 解像度を2/1に
		bloomRenderTarget_.ChangeViewResolution(kBloomResolution_);
		//blurRenderTarget_.ChangeViewResolution(kBloomResolution_);

		// 1) 明るいところを摘出
		DrawBloomExtract();

		// 2) 明るいところをぼかす
		//DrawBloomBlur();
		DrawDownSample();
		DrawUpSample();


		// 2.5)* 解像度を戻の解像度に戻す
		bloomRenderTarget_.ChangeViewResolution(1.0f);


		// 3) 最終的なBuckBufferへの描画。
		//    backBufferを描画先にして、blurと既存のポストエフェクト効果を掛け合わせたのを描画
		DrawCompositeToBackBuffer();

}

void PostEffectManager::ImGuiView() {

#ifdef USE_IMGUI

	ImGui::Begin("PostEffectManager");

	/* メイン */
	ImGui::SeparatorText("Effect");

	ImGui::SliderFloat("blurPower", &postEffectData->blurPower, 0.0f, 10.0f);
	ImGui::SliderFloat("vtWavePower", &postEffectData->vtWavePower, 0.0f, 1.0f);
	ImGui::SliderFloat("hzWavePower", &postEffectData->hzWavePower, 0.0f, 1.0f);
	ImGui::ColorEdit4("color", (float*)&postEffectData->color);
	ImGui::SliderFloat("monotone", &postEffectData->monotone, 0.0f, 1.0f);
	ImGui::SliderFloat("colorInversion", &postEffectData->colorInversion, 0.0f, 1.0f);
	ImGui::SliderFloat("neon", &postEffectData->neon, 0.0f, 1.0f);
	ImGui::SliderFloat("noise", &postEffectData->noise, 0.0f, 1.0f);
	ImGui::SliderFloat("glitch", &postEffectData->glitch, 0.0f, 1.0f);
	ImGui::SliderFloat("scanLine", &postEffectData->scanLine, 0.0f, 1.0f);
	ImGui::SliderFloat("fishLens", &postEffectData->fishLens, 0.0f, 1.0f);

	/* ブルーム */
	ImGui::SeparatorText("Bloom");

	ImGui::DragFloat(
		"BloomThreshold",
		&bloomParamData_->bloomThreshold,
		0.01f,0.0f, 0.5f
	);

	ImGui::SliderFloat(
		"BloomIntensity",
		&bloomParamData_->bloomIntensity,
		0.0f, 30.0f
	);

	ImGui::SliderFloat(
		"BloomSoftness",
		&bloomParamData_->bloomSoftness,
		0.0f, 30.0f
	);

	ImGui::SliderFloat(
		"BlurTexelSize",
		&bloomParamData_->blurTexelSize,
		0.0f, 30.0f
	);


	ImGui::End();

#endif

}


void PostEffectManager::DrawBloomExtract()
{
	
	/* オフスクリーンのテキスチャから、明るいを抽出. */

	// BeginRender・・・　PixelShaderからRenderTargetの遷移。描画先のOMSetRenderTargets()の設定。（描き込み先の設定）画面の初期化。viewPortの設定
	// EndRender・・・　  RenderTargetからPixelShaderへの遷移。


	//
	bloomRenderTarget_.BeginRender();

	{
		ID3D12GraphicsCommandList* commandList = DirectXCommon::GetCommandList().Get();

		// モデル描画。　登録されたモデルに紐づいたワールド座標で
		for (auto& [model, worldTransforms] : bloomModelPropaties_) {

			for (WorldTransform* world : worldTransforms) {
				model->DrawBloom(*world, *Scene::GetSceneCamera(), bloomParamResource.Get());
			}
		}

		/* SceneBloom の Rootsignatureと、パイプラインの設定 */
		//commandList->SetGraphicsRootSignature(PipelineStateManager::GetInstance()->GetPipelineState(PipelineType::SceneBloom).rootSignature);
		//commandList->SetPipelineState(PipelineStateManager::GetInstance()->GetPipelineState(PipelineType::SceneBloom).graphicsPipelineState);

		//ID3D12DescriptorHeap* descriptorHeaps[] = {
		//DirectXCommon::GetSrvHeap().Get()
		//};

		//commandList->SetDescriptorHeaps(1, descriptorHeaps);


		//commandList->IASetVertexBuffers(0, 1, &vertexBufferViewScreen);
		//commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


		//// 読込：オフスクリーンのテキスチャ
		//commandList->SetGraphicsRootDescriptorTable(0, offscreenSRVHandleGPU);

		//// ブルーム　パラメータ値
		//commandList->SetGraphicsRootConstantBufferView(3, bloomParamResource->GetGPUVirtualAddress());

		//// 描画
		//commandList->DrawInstanced(6, 1, 0, 0);

	}

	//
	bloomRenderTarget_.EndRender();


}

void PostEffectManager::DrawBloomBlur()
{
	/* その取り出した明るさをぼかす. */

	// BeginRender・・・　PixelShaderからRenderTargetの遷移。描画先のOMSetRenderTargets()の設定。（描き込み先の設定）画面の初期化。viewPortの設定
	// EndRender・・・　  RenderTargetからPixelShaderへの遷移。


	//
	//blurRenderTarget_.BeginRender();

	//{


	//	ID3D12GraphicsCommandList* commandList = DirectXCommon::GetCommandList().Get();

	//	/* SceneBloom の Rootsignatureと、パイプラインの設定 */
	//	commandList->SetGraphicsRootSignature(PipelineStateManager::GetInstance()->GetPipelineState(PipelineType::SceneBlurDownSample).rootSignature);
	//	commandList->SetPipelineState(PipelineStateManager::GetInstance()->GetPipelineState(PipelineType::SceneBlurDownSample).graphicsPipelineState);

	//	ID3D12DescriptorHeap* descriptorHeaps[] = {
	//	DirectXCommon::GetSrvHeap().Get()
	//	};

	//	commandList->SetDescriptorHeaps(1, descriptorHeaps);


	//	commandList->IASetVertexBuffers(0, 1, &vertexBufferViewScreen);
	//	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	//	// 読込：さっきの取り出した明るさのテキスチャ
	//	commandList->SetGraphicsRootDescriptorTable(0, bloomRenderTarget_.GetSRVHandleGPU());

	//	// ブルーム　パラメータ値
	//	commandList->SetGraphicsRootConstantBufferView(2, bloomParamResource->GetGPUVirtualAddress());

	//	// 描画
	//	commandList->DrawInstanced(6, 1, 0, 0);

	//	

	//	
	//}
	//blurRenderTarget_.EndRender();
}


void PostEffectManager::DrawCompositeToBackBuffer()
{

	ID3D12GraphicsCommandList* commandList = DirectXCommon::GetCommandList().Get();

	// ★バックバッファを RENDER_TARGET に（安全のため入れる）
	UINT backBufferIndex = DirectXCommon::GetInstance()->GetSwapChain()->GetCurrentBackBufferIndex();
	offscreenTransitionBarrier_.Transition.pResource = DirectXCommon::GetInstance()->GetBackBuffer(backBufferIndex).Get(); // 適切な取得関数に合わせて
	offscreenTransitionBarrier_.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	offscreenTransitionBarrier_.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	commandList->ResourceBarrier(1, &offscreenTransitionBarrier_);


	/* 描画先はBackBuffer */
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle =
		DirectXCommon::GetInstance()->GetBackBufferRTV(backBufferIndex);

	commandList->OMSetRenderTargets(
		1,
		&rtvHandle,
		FALSE,
		nullptr);

	//// ★修正 3: バックバッファのクリア（必要に応じて）
	//float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	//commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);


	// ポストエフェクト用のViewport / Scissorを設定
	commandList->RSSetViewports(
		1,
		&DirectXCommon::GetInstance()->GetViewport()
	);

	commandList->RSSetScissorRects(
		1,
		&DirectXCommon::GetInstance()->GetScissorRect()
	);


	// RootSignatureを設定。
	commandList->SetGraphicsRootSignature(PipelineStateManager::GetInstance()->GetPipelineState(PipelineType::PostEffect).rootSignature);

	// PSOを設定
	commandList->SetPipelineState(PipelineStateManager::GetInstance()->GetPipelineState(PipelineType::PostEffect).graphicsPipelineState);

	ID3D12DescriptorHeap* descriptorHeaps[] = {
	DirectXCommon::GetSrvHeap().Get()
	};

	commandList->SetDescriptorHeaps(1, descriptorHeaps);


	commandList->IASetVertexBuffers(0, 1, &vertexBufferViewScreen);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	/* 合成 */
	commandList->SetGraphicsRootDescriptorTable(0, offscreenSRVHandleGPU);
	commandList->SetGraphicsRootDescriptorTable(1, blurRenderTargets_.back().GetSRVHandleGPU());


	//// ポストエフェクト　パラメータ値
	commandList->SetGraphicsRootConstantBufferView(2, postEffectResource->GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(3, bloomParamResource->GetGPUVirtualAddress());

	commandList->DrawInstanced(6, 1, 0, 0);
}

void PostEffectManager::InitializeBlur()
{
	/*
		ピラミッド方式のを使用
	*/

	/*　段階数 */
	kBloomLevel = 4;

	/* 
		1280x720
		640,360
		320,180.. とどんどん解像度をさげる
	*/


	blurRenderTargets_.resize(kBloomLevel);

	uint32_t width = WinApp::GetInstance()->GetClientWidth() / 2;
	uint32_t height = WinApp::GetInstance()->GetClientHeight() / 2;

	for (uint32_t i = 0; i < kBloomLevel; ++i)
	{
		blurRenderTargets_[i].Initialize(width, height);

		// どんどん割る
		width /= 2;
		height /= 2;
	}
}

void PostEffectManager::DrawDownSample()
{

	ID3D12GraphicsCommandList* commandList = DirectXCommon::GetCommandList().Get();

	commandList->SetGraphicsRootSignature(PipelineStateManager::GetInstance()->GetPipelineState(PipelineType::SceneBlurDownSample).rootSignature);
	commandList->SetPipelineState(PipelineStateManager::GetInstance()->GetPipelineState(PipelineType::SceneBlurDownSample).graphicsPipelineState);

	commandList->IASetVertexBuffers(0, 1, &vertexBufferViewScreen);
	commandList->IASetPrimitiveTopology(
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST
	);

	/* Blurピラミッドを採用 */

	/*
		1280x720
		640x360
		320x180... と徐々に下げて描画していく
	*/

	for (uint32_t i = 0; i < blurRenderTargets_.size(); ++i)
	{
		// 入力SRV
		D3D12_GPU_DESCRIPTOR_HANDLE srvHandle{};

		if (i == 0)
		{
			// 最初はBloom抽出RT
			srvHandle = bloomRenderTarget_.GetSRVHandleGPU();
		}
		else
		{
			// i = 1~ なのでひとつ前の解像度のRT
			srvHandle = blurRenderTargets_[i - 1].GetSRVHandleGPU();
		}


		// 描画先RT
		blurRenderTargets_[i].BeginRender();

			// 描画
			commandList->SetGraphicsRootDescriptorTable(0, srvHandle);
			commandList->DrawInstanced(6, 1, 0, 0);

		blurRenderTargets_[i].EndRender();

	}
}

/*
	320x180
	640x360
	1280x720.. とブレンドの加算を加えながら描画
*/
void PostEffectManager::DrawUpSample()
{

	ID3D12GraphicsCommandList* commandList = DirectXCommon::GetCommandList().Get();

	commandList->SetGraphicsRootSignature(PipelineStateManager::GetInstance()->GetPipelineState(PipelineType::SceneBlurUpSample).rootSignature);
	commandList->SetPipelineState(PipelineStateManager::GetInstance()->GetPipelineState(PipelineType::SceneBlurUpSample).graphicsPipelineState);

	commandList->IASetVertexBuffers(0, 1, &vertexBufferViewScreen);
	commandList->IASetPrimitiveTopology(
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST
	);

	commandList->IASetVertexBuffers(0, 1, &vertexBufferViewScreen);
	commandList->IASetPrimitiveTopology(
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST
	);

	// 小さい解像度から大きい解像度へ
	for (int32_t i = static_cast<int32_t>(blurRenderTargets_.size()) - 1;
		i > 0; --i)
	{
		// 入力：ひとつ小さいRT
		D3D12_GPU_DESCRIPTOR_HANDLE srvHandle =
			blurRenderTargets_[i].GetSRVHandleGPU();

		// 出力：ひとつ大きいRT
		RenderTarget* destination = &blurRenderTargets_[i - 1];

		// 加算描画開始 - 画面クリアはしない。どんどん上に重ねていく必要があるから
		destination->BeginRender(false);

		commandList->SetGraphicsRootDescriptorTable(0, srvHandle);

		commandList->DrawInstanced(6, 1, 0, 0);

		destination->EndRender();
	}
}
