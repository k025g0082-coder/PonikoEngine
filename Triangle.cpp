#include "Triangle.h"

//#include "Vector.h"
#include "Matrix.h"

#include "WinApp.h"
#include "PipelineStateManager.h"

#include "DirectXCommon.h"
#include "DirectXUtility.h"

void Triangle::Initialize()
{
	device_ = DirectXCommon::GetInstance()->GetDevice().Get();

	worldTransform_.translate = { 0.0f,0.0f,0.0f };
	worldTransform_.rotate = { 0.0f,0.0f,0.0f };
	worldTransform_.scale = { 1.0f,1.0f,1.0f };

	//頂点リソース用のヒープ
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

	// 頂点リソースの設定
	D3D12_RESOURCE_DESC vertexResourceDesc{};

	vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vertexResourceDesc.Width = sizeof(SpriteVertexData) * 3;

	vertexResourceDesc.Height = 1;
	vertexResourceDesc.DepthOrArraySize = 1;
	vertexResourceDesc.MipLevels = 1;
	vertexResourceDesc.SampleDesc.Count = 1;

	// バッファはこれにする決まり
	vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	// 実際に頂点リソースを作る
	ID3D12Resource* vertexResource = nullptr;
	HRESULT hr = device_->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
		&vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(&vertexResource));

	assert(SUCCEEDED(hr));

	// リソースの先頭のアドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();

	// 使用するリソースのサイズは頂点３つ分
	vertexBufferView.SizeInBytes = sizeof(SpriteVertexData) * 3;

	//　１頂点のサイズ
	vertexBufferView.StrideInBytes = sizeof(SpriteVertexData);

	// リソースの箱にデータを書き込んでく
	SpriteVertexData* vertexData = nullptr;

	// 書き込むアドレスを取得
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	// 左下
	vertexData[0].position = { -0.5f,-0.5f,0.0f,1.0f };
	vertexData[0].texcoord = { 0.0f,1.0f };

	// 上
	vertexData[1].position = { 0.0f,0.5f,0.0f,1.0f };
	vertexData[1].texcoord = { 0.5f,0.0f };

	// 右下
	vertexData[2].position = { 0.5f,-0.5f,0.0f,1.0f };
	vertexData[2].texcoord = { 1.0f,1.0f };

	// これから書き込むRTVのインデックス
	UINT backBufferIndex = DirectXCommon::GetInstance()->GetSwapChain()->GetCurrentBackBufferIndex();

	// マテリアル用のリソース
	materialResource = DirectXUtility::CreateBufferResource(device_, sizeof(SpriteVertexData));
	Vector4* materialData = nullptr;
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	*materialData = { 1.0f,1.0f,1.0f,1.0f }; // 白色

	// mvp用のリソース作り
	wvpResource = DirectXUtility::CreateBufferResource(device_, sizeof(Matrix4x4));
	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&mvpData));
	*mvpData = MakeIdentity4x4();

	// テクスチャ
	texture.Initialize("resources/default.png");
};

void Triangle::Update() {

	// 回すだけ
	worldTransform_.rotate.y += 0.03f;

	// 合成
	Matrix4x4 worldMatrix = MakeAffineMatrix(worldTransform_.scale, worldTransform_.rotate, worldTransform_.translate);

	// カメラのと合成して計算
	Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(camera_->GetViewMatrix(), camera_->GetProjectionMatrix()));
	*mvpData = worldViewProjectionMatrix;
};

void Triangle::Draw(ID3D12GraphicsCommandList* commandList) {

	//RootSignatureを設定。PSOに設定しているけど別途設定が必要
	commandList->SetGraphicsRootSignature(PipelineStateManager::GetInstance()->GetPipelineState(PipelineType::Triangle).rootSignature);
	// PSOを設定
	commandList->SetPipelineState(PipelineStateManager::GetInstance()->GetPipelineState(PipelineType::Triangle).graphicsPipelineState);
	// VBVを設定
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);

	//形状を設定。PSOに設定しているものとはまた別。同じものを設定すると考えておけば良い
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// マテリアルCBufferの場所を設定
	commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());

	// 座標CBufferの場所を設定
	commandList->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());

	ID3D12DescriptorHeap* heaps[] = {
	DirectXCommon::GetInstance()->GetSrvHeap().Get()
	};

	commandList->SetDescriptorHeaps(1, heaps);

	// SRVDescriptorTableの先頭を設定。２，はRootParametor[2]の２
	commandList->SetGraphicsRootDescriptorTable(2, texture.GetTextureSrvHandleGPU());

	//描画!(DrawCall/ドローコール)。3頂点で1つのインスタンス。インスタンスについては今後
	commandList->DrawInstanced(3, 1, 0, 0);

};