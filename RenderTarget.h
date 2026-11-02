#pragma once
#include "d3d12.h"

#include <wrl.h>
#include <wrl/client.h>

#include <cstdint>

#include "Vector.h"

/*

	RenderTargetクラス　専用のSRVをもってます

	ここで作られたSRV♂を専用のViewやマテリアルのモデルやテクスチャ描画時のときに適用させる

*/

class RenderTarget
{
public:

	RenderTarget() = default;
	~RenderTarget() = default;

	// コピーの禁止（浅いコピーによるリソース二重解放を防ぐ）
	RenderTarget(const RenderTarget&) = delete;
	RenderTarget& operator=(const RenderTarget&) = delete;

	// ムーブの許可（std::vector の resize や push_back で安全に要素移動できるようにする）
	RenderTarget(RenderTarget&&) noexcept = default;
	RenderTarget& operator=(RenderTarget&&) noexcept = default;
	
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="width">横幅</param>
	/// <param name="height">縦幅</param>
	/// <param name="viewResolution">画面の解像度の倍率（低いほど低負荷）</param>
	void Initialize(int32_t width,int32_t height, float viewResolution = 1.0f);

	/// <summary>
	/// 描画開始。描画先のRTVの切り替えとPIXELSHADER->RENDER_TARGET のバリアーの遷移
	/// </summary>
	/// <param name="clear">画面をクリアするか</param>
	/// <param name="clolor">画面をクリア時カラ－</param>
	/// <param name="depth">深度バッファをクリアするか</param>
	void BeginRender(
		bool clear = true,
		Vector4 clearColor = {0.0f,0.0f,0.0f,0.0f}, 
		bool depth = true);

	//  描画終了。RENDER_TARGET->PIXELSHADER のバリアーの遷移で元に戻す
	void EndRender();


	/// <summary>
	/// 大きさリサイズ
	/// </summary>
	/// <param name="width">横</param>
	/// <param name="height">縦</param>
	/// <param name="offset">オフセット</param>
	void Resize(int32_t width, int32_t height, Vector2 offset);

	// 解像度変更
	void ChangeViewResolution(float value_);

	// *描画時のSRVハンドル
	D3D12_GPU_DESCRIPTOR_HANDLE GetSRVHandleGPU() const { return srvHandleGPU_; }

	//
	D3D12_CPU_DESCRIPTOR_HANDLE GetRtvHandle() {
		return rtvHeap_->GetCPUDescriptorHandleForHeapStart();
	};

	D3D12_VIEWPORT& GetViewPort() { return outViewport; };
	D3D12_RECT& GetScissorRect() { return outScissorRect; };

	/* 指定された大きさなのでそれ相応のビューポートを作る. */
	// ビューポート
	D3D12_VIEWPORT viewport;

	// バックバッファ出力用（offset が反映された位置）
	D3D12_VIEWPORT outViewport;
	D3D12_RECT outScissorRect;


	int32_t baseWidth_;
	int32_t baseHeight_;

	int32_t width_;	// ビューの大きさ
	int32_t height_;//　ビューの大きさ
	Vector2 offset_;// ビューのオフセット

	struct Rect {
		int32_t left;
		int32_t right;
		int32_t top;
		int32_t bottom;
	};
	Rect viewDrawRect_; // ビュー自体の描画サイズ

	/// <summary>
	/// ビュー内の描画区域を設定
	/// </summary>
	/// <param name="rect">それぞれ左右上下の座標</param>
	void SetDrawRect(Rect rect);

	/// <summary>
	/// ビューを表示する座標のオフセット
	/// </summary>
	/// <param name="offset">オフセット</param>
	void SetViewOffset(Vector2 offset);

	const Vector2& GetViewOffset() const { return offset_; }


	// RenderTargetの描画区域など
	void ImGuiView();


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

	// 解像度
	float resolution_ = 1.0f;

	bool firstFrame_ = true;
};

