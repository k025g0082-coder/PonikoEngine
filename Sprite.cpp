#include "Sprite.h"

//#include "Vector.h"
#include "Matrix.h"

#include "WinApp.h"
#include "PipelineStateManager.h"

#include "DirectXCommon.h"
#include "DirectXUtility.h"

void Sprite::TextTextureInitialize(unsigned char* image,int width,int height) {

	device_ = DirectXCommon::GetInstance()->GetDevice().Get();

	texture.Initialize(image,width,height);

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

	//Sprite用の頂点リソースを作る
	ID3D12Resource* vertexResource = DirectXUtility::CreateBufferResource(device_, sizeof(SpriteVertexData) * 6);

	// リソースの先頭のアドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();

	// 使用するリソースのサイズは頂点３つ分
	vertexBufferView.SizeInBytes = sizeof(SpriteVertexData) * 6;

	//　１頂点のサイズ
	vertexBufferView.StrideInBytes = sizeof(SpriteVertexData);

	// リソースの箱にデータを書き込んでく
	SpriteVertexData* vertexDataSprite = nullptr;
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSprite));

	float tWidth = static_cast<float>(width);
	float tHeight = static_cast<float>(height);

	// 1枚目の三角形
	vertexDataSprite[0].position = { 0.0f, tHeight, 0.0f, 1.0f }; // 左下
	vertexDataSprite[0].texcoord = { 0.0f, 1.0f };

	vertexDataSprite[1].position = { 0.0f,    0.0f,    0.0f, 1.0f }; // 左上
	vertexDataSprite[1].texcoord = { 0.0f, 0.0f };

	vertexDataSprite[2].position = { tWidth,  tHeight, 0.0f, 1.0f }; // 右下
	vertexDataSprite[2].texcoord = { 1.0f, 1.0f };

	// 2枚目の三角形
	vertexDataSprite[3].position = { 0.0f,    0.0f,    0.0f, 1.0f }; // 左上
	vertexDataSprite[3].texcoord = { 0.0f, 0.0f };

	vertexDataSprite[4].position = { tWidth,  0.0f,    0.0f, 1.0f }; // 右上
	vertexDataSprite[4].texcoord = { 1.0f, 0.0f };

	vertexDataSprite[5].position = { tWidth,  tHeight, 0.0f, 1.0f }; // 右下
	vertexDataSprite[5].texcoord = { 1.0f, 1.0f };

	//Sprite用のTransformationMatrix用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	wvpResource = DirectXUtility::CreateBufferResource(device_, sizeof(Matrix4x4));

	//書き込むためのアドレスを取得
	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&mvpData));
	//単位行列を書きこんでおく
	*mvpData = MakeIdentity4x4();

	// WVP行列を計算してリソースに書き込む
	Matrix4x4 worldMatrix = MakeAffineMatrix(worldTransform_.scale, worldTransform_.rotate, worldTransform_.translate);
	Matrix4x4 viewMatrix = MakeIdentity4x4();

	// カメラ使わないため正射影行列の計算
	Matrix4x4 projectionMatrix = MakeOrthographicMatrix(
		0.0f,
		0.0f,
		(float)WinApp::GetInstance()->GetClientWidth(),
		(float)WinApp::GetInstance()->GetClientHeight(),
		0.0f,
		100.0f);

	*mvpData = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));

	// これから書き込むRTVのインデックス
	UINT backBufferIndex = DirectXCommon::GetInstance()->GetSwapChain()->GetCurrentBackBufferIndex();

	// マテリアル用のリソース
	materialResource = DirectXUtility::CreateBufferResource(device_, sizeof(SpriteVertexData));
	Vector4* materialData = nullptr;
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	*materialData = { 1.0f,1.0f,1.0f,1.0f }; // 白色

};

void Sprite::Initialize(std::string texturePath)
{

	// テクスチャ
	texture.Initialize(texturePath);

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
	
	//Sprite用の頂点リソースを作る
	ID3D12Resource* vertexResource = DirectXUtility::CreateBufferResource(device_, sizeof(SpriteVertexData) * 6);

	// リソースの先頭のアドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();

	// 使用するリソースのサイズは頂点３つ分
	vertexBufferView.SizeInBytes = sizeof(SpriteVertexData) * 6;

	//　１頂点のサイズ
	vertexBufferView.StrideInBytes = sizeof(SpriteVertexData);

	// リソースの箱にデータを書き込んでく
	SpriteVertexData* vertexDataSprite = nullptr;
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSprite));

	float tWidth = texture.GetTextureWidth();
	float tHeight = texture.GetTextureHeight();

	// 1枚目の三角形
	vertexDataSprite[0].position = { 0.0f, tHeight, 0.0f, 1.0f }; // 左下
	vertexDataSprite[0].texcoord = { 0.0f, 1.0f };

	vertexDataSprite[1].position = { 0.0f,    0.0f,    0.0f, 1.0f }; // 左上
	vertexDataSprite[1].texcoord = { 0.0f, 0.0f };

	vertexDataSprite[2].position = { tWidth,  tHeight, 0.0f, 1.0f }; // 右下
	vertexDataSprite[2].texcoord = { 1.0f, 1.0f };

	// 2枚目の三角形
	vertexDataSprite[3].position = { 0.0f,    0.0f,    0.0f, 1.0f }; // 左上
	vertexDataSprite[3].texcoord = { 0.0f, 0.0f };

	vertexDataSprite[4].position = { tWidth,  0.0f,    0.0f, 1.0f }; // 右上
	vertexDataSprite[4].texcoord = { 1.0f, 0.0f };

	vertexDataSprite[5].position = { tWidth,  tHeight, 0.0f, 1.0f }; // 右下
	vertexDataSprite[5].texcoord = { 1.0f, 1.0f };

	//Sprite用のTransformationMatrix用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	wvpResource = DirectXUtility::CreateBufferResource(device_, sizeof(Matrix4x4));

	//書き込むためのアドレスを取得
	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&mvpData));
	//単位行列を書きこんでおく
	*mvpData = MakeIdentity4x4();

	// WVP行列を計算してリソースに書き込む
	Matrix4x4 worldMatrix = MakeAffineMatrix(worldTransform_.scale, worldTransform_.rotate,worldTransform_.translate);
	Matrix4x4 viewMatrix = MakeIdentity4x4();

	// カメラ使わないため正射影行列の計算
	Matrix4x4 projectionMatrix = MakeOrthographicMatrix(
		0.0f,
		0.0f,
		(float)WinApp::GetInstance()->GetClientWidth(),
		(float)WinApp::GetInstance()->GetClientHeight(),
		0.0f,
		100.0f);

	*mvpData = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));

	// これから書き込むRTVのインデックス
	UINT backBufferIndex = DirectXCommon::GetInstance()->GetSwapChain()->GetCurrentBackBufferIndex();

	// マテリアル用のリソース
	materialResource = DirectXUtility::CreateBufferResource(device_, sizeof(SpriteVertexData));
	Vector4* materialData = nullptr;
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	*materialData = { 1.0f,1.0f,1.0f,1.0f }; // 白色

};

void Sprite::TextTextureUpdate(unsigned char* image) {

	// テクスチャ更新
	texture.TextTextureUpdate(image);
};

void Sprite::Update() {
	UpdateMatrix();
};
// 行列更新
void Sprite::UpdateMatrix() {

	// 合成
	Matrix4x4 worldMatrix = MakeAffineMatrix(worldTransform_.scale, worldTransform_.rotate, worldTransform_.translate);

	// 正射影行列を生成(カメラを使わない、表面のアレのため)
	Matrix4x4 projectionMatrix = MakeOrthographicMatrix(
		0.0f,
		0.0f,
		(float)WinApp::GetInstance()->GetClientWidth(),
		(float)WinApp::GetInstance()->GetClientHeight(),
		0.0f,
		100.0f);

	*mvpData = Multiply(worldMatrix, projectionMatrix);
};
void Sprite::Draw(ID3D12GraphicsCommandList* commandList) {

	//RootSignatureを設定。PSOに設定しているけど別途設定が必要
	commandList->SetGraphicsRootSignature(PipelineStateManager::GetInstance()->GetPipelineState(PipelineType::Sprite).rootSignature);
	// PSOを設定
	commandList->SetPipelineState(PipelineStateManager::GetInstance()->GetPipelineState(PipelineType::Sprite).graphicsPipelineState);

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

	//描画!(DrawCall/ドローコール)
	commandList->DrawInstanced(6, 1, 0, 0);

};