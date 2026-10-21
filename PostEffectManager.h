#pragma once
#include "d3d12.h"

#include "Vector.h"
#include "Matrix.h"

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


	D3D12_CPU_DESCRIPTOR_HANDLE GetRtvHandleOffscreen() {
		return rtvHeapOffscreen->GetCPUDescriptorHandleForHeapStart();
	};

private:

	bool firstFrame_ = true;

	ID3D12Device* device_;

	ID3D12Resource* vertexResourceScreen = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewScreen{};

	ID3D12Resource* postEffectResource = nullptr;

	PostEffectParam* postEffectData = nullptr;

	// グローバル変数あたりに追加
	ID3D12Resource* offscreenRenderTarget = nullptr;
	ID3D12DescriptorHeap* rtvHeapOffscreen = nullptr;  // オフスクリーン用RTV
	D3D12_CPU_DESCRIPTOR_HANDLE offscreenRTVHandle{};

	// SRV（ポストエフェクトで読むため）
	D3D12_CPU_DESCRIPTOR_HANDLE offscreenSRVHandleCPU{};
	D3D12_GPU_DESCRIPTOR_HANDLE offscreenSRVHandleGPU{};
};

