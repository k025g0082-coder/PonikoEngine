
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


	/*
		オフスクリーン　RenderTarget初期化
	*/
	offscreenRenderTarget_.Initialize(
		WinApp::GetInstance()->GetClientWidth(),
		WinApp::GetInstance()->GetClientHeight());

	//*  BloomのRenderTarget初期化  *//

	/* 解像度は２分の一。bloom合成時に元に戻す */
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

	/* Fog */
	fogParamResource = DirectXUtility::CreateBufferResource(device_, sizeof(FogParam));

	fogParamResource->Map(
		0,
		nullptr,
		reinterpret_cast<void**>(&fogParamData_)
	);

	// 初期値
	fogParamData_->isFog = true;
	fogParamData_->fogColor = { 0.5f, 0.5f, 0.5f };
	fogParamData_->fogStart = 10.0f;
	fogParamData_->fogEnd = 100.0f;


	/* blur初期化 */
	InitializeBlur();
	
	/* オフスクリーン専用DSV作成 */
	CreateOffscreenDSV(
		WinApp::GetInstance()->GetClientWidth(),
		WinApp::GetInstance()->GetClientHeight(),
		false);

	auto a = offscreenDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	offscreenRenderTarget_.SetDsvCPUHandle(a);

}

// ポストエフェクト受付開始
void PostEffectManager::BeginScene() {

	postEffectData->time += Timer::GetDeltaTime();

	ID3D12GraphicsCommandList* commandList = DirectXCommon::GetCommandList().Get();

	if (!firstFrame_) {

		// オフスクリーンを描画可能に戻す
		//D3D12_RESOURCE_BARRIER barrier{};
		//barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		//barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

		//barrier.Transition.pResource =
		//	offscreenRenderTarget.Get();

		//barrier.Transition.StateBefore =
		//	D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

		//barrier.Transition.StateAfter =
		//	D3D12_RESOURCE_STATE_RENDER_TARGET;

		//commandList->ResourceBarrier(1, &barrier);

		// 深度バッファ用を元に戻す
		D3D12_RESOURCE_BARRIER depthBarrier{};
		depthBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		depthBarrier.Transition.pResource = offscreenDepthStencilResource.Get();
		depthBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		depthBarrier.Transition.StateAfter =D3D12_RESOURCE_STATE_DEPTH_WRITE; 
		depthBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		commandList->ResourceBarrier(1, &depthBarrier);
	}

	// オフスクリーンRTVに切り替え。初期化時のいろは青っぽい色
	offscreenRenderTarget_.BeginRender(true,Vector4{ 0.1f, 0.25f, 0.5f,1.0f });


	firstFrame_ = false;



	//// オフスクリーンRTVに切り替え
	//D3D12_CPU_DESCRIPTOR_HANDLE rtvHandleOffscreen = rtvHeapOffscreen->GetCPUDescriptorHandleForHeapStart();
	//D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = DirectXCommon::GetInstance()->GetDsvHeap()->GetCPUDescriptorHandleForHeapStart();

	//commandList->OMSetRenderTargets(1, &rtvHandleOffscreen, FALSE, &dsvHandle);

	//// クリア
	//float clearColor[] = { 0.1f, 0.25f, 0.5f,1.0f };// 青色。RGBAの順
	//commandList->ClearRenderTargetView(rtvHandleOffscreen, clearColor, 0, nullptr);

	//commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	//commandList->RSSetViewports(1, &DirectXCommon::GetInstance()->GetViewport());
	//commandList->RSSetScissorRects(1, &DirectXCommon::GetInstance()->GetScissorRect());

	

}

// ポストエフェクト受付終了(描画
void PostEffectManager::ApplyScene() {

	ID3D12GraphicsCommandList* commandList = DirectXCommon::GetInstance()->GetCommandList().Get();
	
	// オフスクリーンを読める状態に
	offscreenRenderTarget_.EndRender();


	// 深度バッファを読める状態に
	D3D12_RESOURCE_BARRIER depthBarrier{};
	depthBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	depthBarrier.Transition.pResource = offscreenDepthStencilResource.Get();
	depthBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	depthBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	depthBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	commandList->ResourceBarrier(1, &depthBarrier);


		/* 
			ブラー処理 

			1) 明るいところを摘出
			2) 明るいところをぼかす
			3) 最終的なBuckBufferへの描画。
		　    　backBufferを描画先にして、blurと既存のポストエフェクト効果を掛け合わせたのを描画

		*/
		if (bloomModelPropaties_.size() > 0) {

			// 0.5)* 解像度を2/1に ( なんかバグるのでやめ20261103  多分viewPort関連
			//bloomRenderTarget_.ChangeViewResolution(kBloomResolution_);

			// 1) 明るいところを摘出
			DrawBloomExtract();

			// 2) 明るいところをぼかす
			DrawBloomBlur();
			DrawDownSample();
			DrawUpSample();

			// 2.5)* 解像度を戻の解像度に戻す
			//bloomRenderTarget_.ChangeViewResolution(1.0f);
		}

		


		// 3) 最終的なBuckBufferへの描画。
		//    backBufferを描画先にして、blurと既存のポストエフェクト効果を掛け合わせたのを描画
		DrawCompositeToBackBuffer();

}

void PostEffectManager::ImGuiView(bool& show) {

#ifdef USE_IMGUI

	ImGui::Begin("PostEffectManager",&show);

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
		0.0f, 100.0f
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

	/* Fog */
	ImGui::SeparatorText("Fog");

	ImGui::Checkbox("isFog", &fogParamData_->isFog);
	ImGui::ColorEdit3("Fog Color", (float*)&fogParamData_->fogColor);
	ImGui::SliderFloat("Fog Start", &fogParamData_->fogStart, -250.0f, fogParamData_->fogEnd);
	ImGui::SliderFloat("Fog End", &fogParamData_->fogEnd, fogParamData_->fogStart, 250.0f);


	ImGui::End();

#endif

}


	/*=== ポストエフェクト用、DSV(Depth)の作成 === */
void PostEffectManager::CreateOffscreenDSV(int w,int h, bool resize) {

	device_ = DirectXCommon::GetInstance()->GetDevice().Get();

	//.. リサイズの場合
	if (resize) {
		
		offscreenDepthStencilResource.Reset();// 古いDepthStencilResourceを解放

	}

	// DepthStencilTextureを作る
	offscreenDepthStencilResource = DirectXUtility::CreateDepthStencilTexture(
		device_,
		w,
		h
	);

	//DSV用のヒープでディスクリプタの数は1。DSVはShader内で触るものではないので、ShaderVisibleはfalse
	if (!resize)
		offscreenDsvDescriptorHeap = DirectXUtility::CreateDescriptorHeap(device_, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);

	// DSVの設定
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//Format。基本的にはResourceに合わせる
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D; // 2dTexture

	//DSVHeapの先頭にDSVをつくる
	device_->CreateDepthStencilView(offscreenDepthStencilResource.Get(), &dsvDesc, offscreenDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	//DepthStencilStateの設定

	//Depthの機能を有効化する
	offscreenDepthStencilDesc.DepthEnable = true;
	//書き込みします
	offscreenDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	//比較関数はLessEqual。つまり、近ければ描画される
	offscreenDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	/*
		20261102 深度を参照するためのSRVを作る
	*/

	// 設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping =
		D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = 1;

	if (!resize) {
		// SRVの一番後ろ取得
		offscreenDepthSRVIndex_ =
			DirectXCommon::GetInstance()->GetAllocateSrv();
	}

	// CPU用のSRVハンドル生成
	auto srvHandle = DirectXCommon::GetSRVHandle(offscreenDepthSRVIndex_);


	// DepthSRVのGPUハンドルを取得
	offscreenDepthSRVHandleGPU =
			DirectXCommon::GetInstance()->GetSrvHeap()
				->GetGPUDescriptorHandleForHeapStart();

		UINT incrementSize =
			device_->GetDescriptorHandleIncrementSize(
				D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
			);

		offscreenDepthSRVHandleGPU.ptr += incrementSize * offscreenDepthSRVIndex_;

	// SRV作成
	DirectXCommon::GetInstance()->GetDevice().Get()->CreateShaderResourceView(
		offscreenDepthStencilResource.Get(),
		&srvDesc,
		srvHandle
	);
}

void PostEffectManager::ResizeOffscreen(uint32_t width, uint32_t height, Vector2 offset) {

	offscreenRenderTarget_.Resize(width, height, offset);

	bloomRenderTarget_.Resize(width, height, offset);

	for (RenderTarget& r : blurRenderTargets_) {
		r.Resize(width, height, offset);
	}

	/* DSVも作りなおす */
	CreateOffscreenDSV((int)width, (int)height, true);
}


void PostEffectManager::DrawBloomExtract()
{
	
	/* オフスクリーンのテキスチャから、明るいを抽出. */

	// BeginRender・・・　PixelShaderからRenderTargetの遷移。描画先のOMSetRenderTargets()の設定。（描き込み先の設定）画面の初期化。viewPortの設定
	// EndRender・・・　  RenderTargetからPixelShaderへの遷移。


	//
	bloomRenderTarget_.BeginRender(true,{0,0,0,0},false);

	{
		ID3D12GraphicsCommandList* commandList = DirectXCommon::GetCommandList().Get();

		// モデル描画。　登録されたモデルに紐づいたワールド座標で
		for (auto& [model, worldTransforms] : bloomModelPropaties_) {

			for (WorldTransform* world : worldTransforms) {
				model->DrawBloom(*world, *Scene::GetSceneCamera(), bloomParamResource.Get());
			}
		}

		/* シーン全体をBloomしたときのやつ */
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

	//
	commandList->OMSetRenderTargets(
		1,
		&rtvHandle,
		FALSE,nullptr);


	// バックバッファ全体を黒でクリア（リサイズ時に余白部分が残らないようにする）
	float clearColor[] = { 0.10f, 0.10f, 0.10f, 1.0f };
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	// ポストエフェクト用のViewport / Scissorを設定
	commandList->RSSetViewports(
		1,
		&offscreenRenderTarget_.GetViewPort()
	);

	commandList->RSSetScissorRects(
		1,
		&offscreenRenderTarget_.GetScissorRect()
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
	commandList->SetGraphicsRootDescriptorTable(0, offscreenRenderTarget_.GetSRVHandleGPU());
	commandList->SetGraphicsRootDescriptorTable(1, blurRenderTargets_.back().GetSRVHandleGPU());
	commandList->SetGraphicsRootDescriptorTable(5, offscreenDepthSRVHandleGPU);


	//// ポストエフェクト　パラメータ値
	commandList->SetGraphicsRootConstantBufferView(2, postEffectResource->GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(3, bloomParamResource->GetGPUVirtualAddress());


		commandList->SetGraphicsRootConstantBufferView(4, fogParamResource->GetGPUVirtualAddress());
	

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
			commandList->SetGraphicsRootConstantBufferView(1, bloomParamResource->GetGPUVirtualAddress());

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
		commandList->SetGraphicsRootConstantBufferView(1, bloomParamResource->GetGPUVirtualAddress());

		commandList->DrawInstanced(6, 1, 0, 0);

		destination->EndRender();
	}
}
