#include "DebugLine.h"

#include "PipelineStateManager.h"
#include "DirectXCommon.h"
#include "DirectXUtility.h"

#include <numbers>

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


void DebugLine::DrawAABB(const AABB& aabb, const Vector4& color) {

	// 8頂点
	Vector3 vertices[8] = {

		// 手前
		{ aabb.min.x, aabb.min.y, aabb.min.z }, // 0
		{ aabb.max.x, aabb.min.y, aabb.min.z }, // 1
		{ aabb.max.x, aabb.max.y, aabb.min.z }, // 2
		{ aabb.min.x, aabb.max.y, aabb.min.z }, // 3

		// 奥
		{ aabb.min.x, aabb.min.y, aabb.max.z }, // 4
		{ aabb.max.x, aabb.min.y, aabb.max.z }, // 5
		{ aabb.max.x, aabb.max.y, aabb.max.z }, // 6
		{ aabb.min.x, aabb.max.y, aabb.max.z }  // 7
	};

	//// スクリーン変換
	//Vector3 screen[8];

	//for (int i = 0; i < 8; i++) {

	//	screen[i] = Transform(vertices[i], camera_->GetProjectionMatrix());
	//	screen[i] = Transform(screen[i], camera_->GetViewMatrix());
	//}

	// 辺の接続
	int edges[12][2] = {

		// 手前
		{0,1},
		{1,2},
		{2,3},
		{3,0},

		// 奥
		{4,5},
		{5,6},
		{6,7},
		{7,4},

		// 前後接続
		{0,4},
		{1,5},
		{2,6},
		{3,7}
	};

	// 描画
	for (int i = 0; i < 12; i++) {

		int start = edges[i][0];
		int end = edges[i][1];

		AddLine(vertices[start], vertices[end], color);

	}

};

// 球描画(SceneのDraw内限定)
void DebugLine::DrawSphere(const Sphere& sphere, const Vector4& color, const int subdivision) {

	// 円周率
	const float kPi = std::numbers::pi_v<float>;


	const float kLatEvery =
		static_cast<float>(kPi) / static_cast<float>(subdivision);

	const float kLonEvery =
		static_cast<float>(2.0f * kPi) / static_cast<float>(subdivision);


	//緯度の方向に分割-T/2~ T/2
	for (int latIndex = 0; latIndex < subdivision; ++latIndex) {

		float lat = -static_cast<float>(kPi) / 2.0f + kLatEvery * latIndex;//現在の緯度

		////経度の方向に分割 0 ~ 2π
		for (int lonIndex = 0; lonIndex < subdivision; ++lonIndex) {

			float lon = lonIndex * kLonEvery;//現在の経度

			float r = sphere.radius;
			Vector3 center = sphere.center;


			// 次
			float latNext = lat + kLatEvery;
			float lonNext = lon + kLonEvery;

			// a
			Vector3 a{
				center.x + r * cosf(lat) * cosf(lon),
				center.y + r * sinf(lat),
				center.z + r * cosf(lat) * sinf(lon)
			};

			// b（経度方向）
			Vector3 b{
				center.x + r * cosf(lat) * cosf(lonNext),
				center.y + r * sinf(lat),
				center.z + r * cosf(lat) * sinf(lonNext)
			};

			// c（緯度方向）
			Vector3 c{
				center.x + r * cosf(latNext) * cosf(lon),
				center.y + r * sinf(latNext),
				center.z + r * cosf(latNext) * sinf(lon)
			};
			//.. らしいです

			//	ab,bcで線を引く
			AddLine(a, b, color);
			AddLine(b, c, color);
		}
	};
}


//　簡単な座標軸を描画 
void DebugLine::DrawAxis(const Vector3& position, float axisLength) {

	// X軸：赤
	AddLine(
		{ position.x - axisLength, position.y, position.z },
		{ position.x + axisLength, position.y, position.z },
		{ 1.0f, 0.0f, 0.0f, 1.0f }
	);

	// Y軸：緑
	AddLine(
		{ position.x, position.y - axisLength, position.z },
		{ position.x, position.y + axisLength, position.z },
		{ 0.0f, 1.0f, 0.0f, 1.0f }
	);

	// Z軸：青
	AddLine(
		{ position.x, position.y, position.z - axisLength },
		{ position.x, position.y, position.z + axisLength },
		{ 0.0f, 0.0f, 1.0f, 1.0f }
	);
	
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