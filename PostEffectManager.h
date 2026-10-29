#pragma once
#include "d3d12.h"

#include "Vector.h"
#include "Matrix.h"

#include "RenderTarget.h"

struct VertexData {
	Vector4 position;
	Vector2 texcoord;
};

// エフェクト用
struct PostEffectParam
{
	float blurPower; // ぼかし量
	float time; // 時間

	float hzWavePower; //  横ラスターの強さ
	float vtWavePower; // 縦ラスターの強さ

	float monotone;    // モノクロ化
	float colorInversion; // 色反転

	float noise; // ノイズ
	float neon;  // ネオン

	float scanLine; // 走査線

	float fishLens; // 魚眼レンズ

	float glitch; // グリッチ

	float padding; // ←調整用　多すぎるほど青くなる

	Vector4 color;

};

/* ブルームのパラメータ */
struct BloomParam
{
	float bloomThreshold; // Bloom抽出開始輝度 - Bloomの明るさ
	float bloomIntensity; // Bloomの強さ       - Bloom強さ
	float bloomSoftness; // 抽出の滑らかさ     - Bloomの周囲のなめらかさ
	float blurTexelSize; // ブラーのサンプリング幅 - ブラーの幅
};


class PostEffectManager
{
public:

	// （安心安定の）シングルトン
	static PostEffectManager* GetInstance();

	// 初期化
	void Initialize();

	// ポストエフェクト受付開始
	void BeginScene();

	// ポストエフェクト受付終了
	void ApplyScene();

	// パラメータデバッグ用
	void ImGuiView();

	//　ポストエフェクトデータの参照
	PostEffectParam& GetPostEffectParam() { return *postEffectData; }
	BloomParam& GetBloomParam() { return *bloomParamData_; }


	D3D12_CPU_DESCRIPTOR_HANDLE GetRtvHandleOffscreen() {
		return rtvHeapOffscreen->GetCPUDescriptorHandleForHeapStart();
	};

private:

	bool firstFrame_ = true;

	ID3D12Device* device_;

	ID3D12Resource* vertexResourceScreen = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewScreen{};

	ID3D12Resource* postEffectResource = nullptr;

	/* 定数バッファ */
	PostEffectParam* postEffectData = nullptr;


	// オフスクリーンRTに描画できるようにする
	D3D12_RESOURCE_BARRIER offscreenTransitionBarrier_ = {};

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};//	



	/* 明るい部分の取り出し */
	void DrawBloomExtract();

	/* 明るい部分のぼかし */
	void DrawBloomBlur();

	/* シーンと合成 */
	void DrawCompositeToBackBuffer();


	/* Bloom用のRenderTarget */
	RenderTarget bloomRenderTarget_;      // 明るい部分の算出結果
	RenderTarget blurRenderTarget_;       // ぼかすための

	// 定数バッファ
	BloomParam* bloomParamData_ = nullptr;
	ID3D12Resource* bloomParamResource = nullptr;


	// ポストエフェクト用のシーン描画のRenderTarget
	ID3D12Resource* offscreenRenderTarget = nullptr;


	ID3D12DescriptorHeap* rtvHeapOffscreen = nullptr;  // オフスクリーン用RTV
	D3D12_CPU_DESCRIPTOR_HANDLE offscreenRTVHandle{};

	// SRV（ポストエフェクトで読むため）
	D3D12_CPU_DESCRIPTOR_HANDLE offscreenSRVHandleCPU{};
	D3D12_GPU_DESCRIPTOR_HANDLE offscreenSRVHandleGPU{};
};

