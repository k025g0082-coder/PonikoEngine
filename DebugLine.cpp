#include "DebugLine.h"

#include "PipelineStateManager.h"
#include "DirectXCommon.h"
#include "DirectXUtility.h"

std::vector<DebugLineVertex> DebugLine::vertices_{};
Camera* DebugLine::camera_ = nullptr;

// シングルトン
DebugLine* DebugLine::GetInstance() {
	static DebugLine instance;
	return &instance;
}

void DebugLine::Initialize() {

	// マックスの頂点数を設定
	constexpr size_t kMaxVertices = 10000;

	vertices_.reserve(kMaxVertices);

	device_ = DirectXCommon::GetInstance()->GetDevice().Get();

	vertexResource_ =
		DirectXUtility::CreateBufferResource(device_,
			sizeof(DebugLineVertex) * kMaxVertices
		);

	vertexBufferView_.BufferLocation =
		vertexResource_->GetGPUVirtualAddress();

	// マックスを設定？
	vertexBufferView_.SizeInBytes =
		static_cast<UINT>(
			sizeof(DebugLineVertex) * kMaxVertices
			);

	vertexBufferView_.StrideInBytes =
		sizeof(DebugLineVertex);


	//カメラのviewProjectionMatrixを送るためのリソースを作る。Matrix4x4 1つ分のサイズを用意する
	viewProjectionResource_ = DirectXUtility::CreateBufferResource(DirectXCommon::GetInstance()->GetDevice().Get(), sizeof(Matrix4x4));

	//書き込むためのアドレスを取得
	viewProjectionResource_->Map(0, nullptr, reinterpret_cast<void**>(&viewProjectionData_));

	//単位行列を書きこんでおく
	*viewProjectionData_ = MakeIdentity4x4();
}

// 線追加
void DebugLine::AddLine(const Vector3& start, const Vector3& end, const Vector4& color){
	vertices_.push_back({ start, color });
	vertices_.push_back({ end, color });
}

void DebugLine::Render() {
	Clear();
}

void DebugLine::Draw(){

	if (vertices_.empty()) {
		return;
	}

	// 転送用データに書き込む
	*viewProjectionData_ = Multiply(camera_->GetViewMatrix(), camera_->GetProjectionMatrix());

	// 範囲を指定してコピー？
	void* mapped = nullptr;

	D3D12_RANGE readRange{};
	vertexResource_->Map(
		0,
		&readRange,
		&mapped
	);
	memcpy(mapped, vertices_.data(),
		sizeof(DebugLineVertex) * vertices_.size());
	vertexResource_->Unmap(0, nullptr);


	ID3D12GraphicsCommandList* commandList = DirectXCommon::GetInstance()->GetCommandList().Get();

	//RootSignatureを設定。PSOに設定しているけど別途設定が必要
	commandList->SetGraphicsRootSignature(PipelineStateManager::GetInstance()->GetPipelineState(PipelineType::DebugLine).rootSignature);
	// PSOを設定
	commandList->SetPipelineState(PipelineStateManager::GetInstance()->GetPipelineState(PipelineType::DebugLine).graphicsPipelineState);

	// VBVを設定
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);//

    // b0 カメラ
	commandList->SetGraphicsRootConstantBufferView(
		0,
		viewProjectionResource_->GetGPUVirtualAddress()
	);

	commandList->DrawInstanced(
		static_cast<UINT>(vertices_.size()),
		1,
		0,
		0
	);
}

void DebugLine::Clear(){
	vertices_.clear();
}