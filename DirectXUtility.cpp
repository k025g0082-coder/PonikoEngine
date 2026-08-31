#include "DirectXUtility.h"
#include <cassert>
#include <iostream>

namespace DirectXUtility
{
	ID3D12DescriptorHeap* CreateDescriptorHeap(
		ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible) {

		ID3D12DescriptorHeap* descriptorHeap = nullptr;
		D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
		descriptorHeapDesc.Type = heapType;
		descriptorHeapDesc.NumDescriptors = numDescriptors;
		descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
		assert(SUCCEEDED(hr));
		return descriptorHeap;
	};

	ID3D12Resource* CreateBufferResource(
		ID3D12Device* device, // デバイス
		size_t sizeInBytes // バッファのサイズ
	) {
		// 頂点リソース用のヒープ
		D3D12_HEAP_PROPERTIES uploadHeapProperties{};
		uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

		// 頂点リソースの設定
		D3D12_RESOURCE_DESC vertexResourceDesc{};

		vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		vertexResourceDesc.Width = sizeInBytes;
		//vertexResourceDesc.Width = sizeof(Vector4) * 3;

		vertexResourceDesc.Height = 1;
		vertexResourceDesc.DepthOrArraySize = 1;
		vertexResourceDesc.MipLevels = 1;
		vertexResourceDesc.SampleDesc.Count = 1;

		// バッファはこれにする決まり
		vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		// 実際に頂点リソースを作る
		ID3D12Resource* vertexResource = nullptr;
		device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
			&vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
			IID_PPV_ARGS(&vertexResource));

		return vertexResource;

		//assert(SUCCEEDED(hr));
	}


	// DepthStencilTextureをつくる
	ID3D12Resource* CreateDepthStencilTexture(ID3D12Device* device, int width, int height) {
		D3D12_RESOURCE_DESC resourceDesc{};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resourceDesc.Width = width;
		resourceDesc.Height = height;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		D3D12_HEAP_PROPERTIES heapProperties{};
		heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

		//深度値のクリア設定
		D3D12_CLEAR_VALUE depthClearValue{};
		depthClearValue.DepthStencil.Depth = 1.0f; // 1.0f でクリア
		depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

		//Resourceの生成
		ID3D12Resource* resource = nullptr;
		HRESULT hr = device->CreateCommittedResource(
			&heapProperties, // HeapOa
			D3D12_HEAP_FLAG_NONE,//Heapの特殊な設定。特になし。
			&resourceDesc, // Resourceの設定
			D3D12_RESOURCE_STATE_DEPTH_WRITE,//深度値を書き込む状態にしておく
			&depthClearValue,//Clear最適値
			IID_PPV_ARGS(&resource));//作成するResourceポインタへのポインタ

		assert(SUCCEEDED(hr));

		return resource;
	};

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index)
	{

		D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
		handleCPU.ptr += (descriptorSize * index);
		return handleCPU;
	}

	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index) {

		D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
		handleGPU.ptr += (descriptorSize * index);
		return handleGPU;
	}
	// string -> wstring
	std::wstring ConvertString(const std::string& str) {
		if (str.empty()) {
			return std::wstring();
		}

		auto sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), NULL, 0);
		if (sizeNeeded == 0) {
			return std::wstring();
		}
		std::wstring result(sizeNeeded, 0);
		MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), &result[0], sizeNeeded);
		return result;
	}

	// string -> wstring
	std::string ConvertString(const std::wstring& str) {
		if (str.empty()) {
			return std::string();
		}

		auto sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), NULL, 0, NULL, NULL);
		if (sizeNeeded == 0) {
			return std::string();
		}
		std::string result(sizeNeeded, 0);
		WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), result.data(), sizeNeeded, NULL, NULL);
		return result;
	}

	void Log(std::ostream& os, const std::string& message)
	{
		os << message << std::endl;
		OutputDebugStringA(message.c_str());
	}
	void Log(const std::wstring& message)
	{
		OutputDebugString(message.c_str());
	}
	void Log(const std::string& message)
	{
		OutputDebugStringA(message.c_str());
	}
	void Log(const char* format, ...)
	{
		char buffer[1024];
		va_list args;
		va_start(args, format);

		vsprintf_s(
			buffer,
			sizeof(buffer),   // ← これが必要
			format,
			args
		);

		va_end(args);
		OutputDebugStringA(buffer);

	}

	float RandomFloat(float min, float max)
	{
		static std::random_device seedGenerator;
		static std::mt19937 randomEngine(seedGenerator());

		std::uniform_real_distribution<float> distribution(min, max);

		return distribution(randomEngine);
	}
}
