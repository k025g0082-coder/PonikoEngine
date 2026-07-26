#pragma once
#include <d3d12.h>
#include <string>

namespace DirectXUtility
{

// ヒープの作成をする関数
	ID3D12DescriptorHeap* CreateDescriptorHeap(
		ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);

	// bufferResourceを作る関数
	ID3D12Resource* CreateBufferResource(
		ID3D12Device* device, // デバイス
		size_t sizeInBytes // バッファのサイズ
	);

	// DepthStencilTextureをつくる
	ID3D12Resource* CreateDepthStencilTexture(ID3D12Device* device, int width, int height);

	//void CompileShader();

	// wstring -> string
	std::wstring ConvertString(const std::string& str);

	// string -> wstring
	std::string ConvertString(const std::wstring& str);

	void Log(std::ostream& os, const std::string& message);
	void Log(const std::wstring& message);
	void Log(const std::string& message);
	void Log(const char* format, ...);
};

