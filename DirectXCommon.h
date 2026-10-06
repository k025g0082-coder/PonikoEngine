#pragma once

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#include "wrl.h"

using namespace Microsoft::WRL;

#include <d3d12.h>
#include <dxgi1_6.h>

#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")

#include <cassert>
#include <cstdint>


class DirectXCommon{
public:

    // シングルトン
    static DirectXCommon* GetInstance();

    // 初期化
    void Initialize();

    // 描画前に
    void PreDraw();

    // 描画後に
    void PostDraw();

    // パイプライン設定の初期化・登録の際に必要
    static ComPtr<ID3D12Device> GetDevice() { return device_; };

    // 描画の際
    static ComPtr<ID3D12GraphicsCommandList> GetCommandList() { return commandList_; };
    static ComPtr<ID3D12CommandQueue> GetCommandQueue() { return commandQueue_; };

    // 三角の描画時に必要
    static ComPtr<IDXGISwapChain4> GetSwapChain() { return swapChain_; };

    static DXGI_FORMAT GetRtvDescFormat() { return rtvDescFormat_; };
    static UINT GetSwapChainBufferCount() { return swapChainBufferCount_; };

    // テクスチャの、SRV指定だったりに必要
    static ComPtr <ID3D12DescriptorHeap> GetSrvHeap() { return srvHeap_; }

    static ComPtr <ID3D12DescriptorHeap> GetRtvHeap() { return rtvHeap_; }

    static ComPtr<ID3D12DescriptorHeap> GetDsvHeap() { return dsvDescriptorHeap; }

    static ComPtr<ID3D12Resource> GetBackBuffer(int i) { return renderTargets_[i];}
    static D3D12_CPU_DESCRIPTOR_HANDLE GetBackBufferRTV(int i) { return rtvHandles[i]; }

	static D3D12_VIEWPORT& GetViewport() { return viewport; }
	static D3D12_RECT& GetScissorRect() { return scissorRect; }

    // モデルのパイプライン設定で必要
    static D3D12_DEPTH_STENCIL_DESC GetDepthStencilDesc() { return depthStencilDesc; }

    // SRV管理番号を返す
    static uint32_t GetAllocateSrv() { return srvIndex_++; }

private:

    void CreateDXGIFactory();
    void CreateAdapter();
    void CreateDevice();
    void CreateCommand();
    void CreateSwapChain();
    void CreateDescriptorHeap();
    void CreateRTV();
    void CreateDSV();
    void CreateFence();
    void InitializeDxc();

    DirectXCommon() = default;

    ComPtr<IDXGIFactory7> dxgiFactory_;
    ComPtr<IDXGIAdapter4> adapter_;           //使用するアダプタ用の変数
    static ComPtr<ID3D12Device> device_;

    static ComPtr<ID3D12CommandQueue> commandQueue_;
    ComPtr<ID3D12CommandAllocator> commandAllocator_;
    static ComPtr<ID3D12GraphicsCommandList> commandList_;

    static ComPtr<IDXGISwapChain4> swapChain_;

    // なんかimguiで必要
    static DXGI_FORMAT rtvDescFormat_;
    static UINT swapChainBufferCount_;

    static ComPtr<ID3D12DescriptorHeap> rtvHeap_;

    static ComPtr<ID3D12Resource> renderTargets_[2];
    static D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];

    static D3D12_DEPTH_STENCIL_DESC depthStencilDesc;
    ComPtr<ID3D12Resource> depthStencilResource;
    static ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap;

    ComPtr<ID3D12Fence> fence_;
    HANDLE fenceEvent_;
    uint64_t fenceValue;

    UINT backBufferIndex_;

    D3D12_RESOURCE_BARRIER barrier{};

    // SRV
    static ComPtr <ID3D12DescriptorHeap> srvHeap_;

    // SRVの管理番号。Textureを足すたびに加算される？
    static uint32_t srvIndex_;

    //ビューポート
    static D3D12_VIEWPORT viewport;
    //シザー矩形
    static D3D12_RECT scissorRect;

    // リソースの開放のリークをチェックする
    // 消されたら自動で呼ばれるらしい
    struct D3DResourceLeakChecker {
        ~D3DResourceLeakChecker() {

            //リソースリークチェック
            Microsoft::WRL::ComPtr<IDXGIDebug1> debug;
            if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
                debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
                debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
                debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);

            }
        }
    };

    D3DResourceLeakChecker leakCheck;
};

