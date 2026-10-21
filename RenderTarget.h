#pragma once
#include "d3d12.h"

#include <wrl.h>
#include <wrl/client.h>

#include <cstdint>

/*

	RenderTargetクラス　専用のSRVをもってます

	ここで作られたSRV♂を専用のViewやマテリアルのモデルやテクスチャ描画時のときに適用させる

*/

class RenderTarget
{
public:

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="width">横幅</param>
	/// <param name="height">縦幅</param>
	void Initialize(int32_t width,int32_t height);

	// 描画開始。描画先のRTVの切り替えとPIXELSHADER->RENDER_TARGET のバリアーの遷移
	void BeginRender();

	//  描画終了。RENDER_TARGET->PIXELSHADER のバリアーの遷移で元に戻す
	void EndRender();

	// *描画時のSRVハンドル
	D3D12_GPU_DESCRIPTOR_HANDLE GetSRVHandleGPU() const { return srvHandleGPU_; }


	/* 指定された大きさなのでそれ相応のビューポートを作る. */
	// ビューポート
	D3D12_VIEWPORT viewport;

	// シザー矩形
	D3D12_RECT scissorRect;

private:

	Microsoft::WRL::ComPtr<ID3D12Device> device_;


	// *頂点bufferViewはいらない 描画時のSRVのハンドルがあればいい

	Microsoft::WRL::ComPtr<ID3D12Resource> renderTarget_;

	// なんか加工できる人
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap_;
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle_{};


	// CPUに知らせる用のハンドル 作成時に使った。
	D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU_{};

	// GPUに知らせる用の、SRV（Viewportで使うよ！）
	D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU_{};

	bool firstFrame_ = true;
};

