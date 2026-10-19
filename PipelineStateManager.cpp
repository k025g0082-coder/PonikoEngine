#include "PipelineStateManager.h"
#include "DirectXCommon.h"

#include "Particle.h"

std::unordered_map<PipelineType, PipelineState> PipelineStateManager::pipelines_;
ID3D12PipelineState* PipelineStateManager::blendGraphicsPipelineState[(int)BlendMode::Count] = {};

PipelineStateManager* PipelineStateManager::GetInstance() {
	static PipelineStateManager p;
	return &p;
}

void PipelineStateManager::Initialize(ID3D12Device* device){

	// 必要
	device_ = device;

	// シェーダーコンパイラ初期化、兼Dxc初期化
	dxcCompiler_.InitializeDxc();

	// それぞれの設定を行う。
	createSpritePipeline();
	createModelPipeline();

	// test
	createTrianglePipeline();

	// パーティクル
	createParticlePipeline();

	// 影用
	createShadowPipeline();

	// デバッグ用ライン
	createDebugLinePipeline();
	
	// ポストエフェクト
	createPostEffectPipeline();
}

void PipelineStateManager::createSpritePipeline() {


	PipelineState& state = pipelines_[PipelineType::Sprite];  // 先に登録

	// === SRVの設定 複数枚の設定をまとめて設定する設定する設定する設定する設定ｓｒsrsrsrrsrsrrrrrr
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0; // 0から始める
	descriptorRange[0].NumDescriptors = 1; // 一つ。ここを増やすと同時に描画できる数が増える？
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // SRVを使う。
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


	// === RootSignatureの設定
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT; // InputLayoutを使うことを許可

	// === Samplerの設定
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER; // L(
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
	staticSamplers[0].ShaderRegister = 0;// レジスタ番号0を使う
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

	// RootParameterの設定(追加時は随時長さを足す）
	D3D12_ROOT_PARAMETER rootParameters[3] = {};

	// 色
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].Descriptor.ShaderRegister = 0; // b0
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // 全てのシェーダーから見えるように

	// 座標
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[1].Descriptor.ShaderRegister = 0; // b0
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; // 全てのシェーダーから見えるように

	// 上のSRVの複数枚の設定の設定の（ｒｙ
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // DescriptorTable15
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShader5
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange; // Tableの中身の配列を指定
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange); // Table3

	descriptionRootSignature.pParameters = rootParameters;
	descriptionRootSignature.NumParameters = _countof(rootParameters);

	// シリアライズしてバイナリにする
	ID3DBlob* rootSignatureBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &rootSignatureBlob, &errorBlob);

	if (FAILED(hr))
	{
		/*Log(reinterpret_cast
			<char*>(errorBlob->GetBufferPointer()));*/
		assert(false);
	}

	// バイナリをもとに生成
	hr = device_->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(),
		rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&state.rootSignature));

	assert(SUCCEEDED(hr));
	// ===

	// === Pipelineの設定

	// InputLayoutの設定 (要は、ここでVertexShaderに頂点データのレイアウトをつたえる)
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[2] = {};

	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;//	 Vector4なので
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT; //		 Vector2なので
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;


	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	// BlendState0
	D3D12_BLEND_DESC blendDesc{};

	//すべての色要素を書き込む
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	// アルファブレンド有効
	blendDesc.RenderTarget[0].BlendEnable = TRUE;

	// RGB
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;

	// Alpha
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;

	//RasterizerState
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	//裏面(時計回り)を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;

	//三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;;

	//Shaderをコンパイルする
	IDxcBlob* vertexShaderBlob = dxcCompiler_.CompileShader(L"resources/Engine/Shader/Sprite.VS.hlsl",
		L"vs_6_0", dxcCompiler_.GetIDxcUtils(), dxcCompiler_.GetDxcCompiler(), dxcCompiler_.GetIncludeHandler());
	assert(vertexShaderBlob != nullptr);

	IDxcBlob* pixelShaderBlob = dxcCompiler_.CompileShader(L"resources/Engine/Shader/Sprite.PS.hlsl",
		L"ps_6_0", dxcCompiler_.GetIDxcUtils(), dxcCompiler_.GetDxcCompiler(), dxcCompiler_.GetIncludeHandler());
	assert(pixelShaderBlob != nullptr);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = state.rootSignature; // RootSignature
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc; // InputLayout
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),
	vertexShaderBlob->GetBufferSize() };// VertexShader
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),
	pixelShaderBlob->GetBufferSize() };// PixelShader
	graphicsPipelineStateDesc.BlendState = blendDesc;// BlendState
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;// RasterizerState
	//書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	//利用するトポロジ(形状)のタイプ。三角形
	graphicsPipelineStateDesc.PrimitiveTopologyType =
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//どのように画面に色を打ち込むかの設定(気にしなくて良い)
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	//実際に生成
	hr = device_->CreateGraphicsPipelineState(&graphicsPipelineStateDesc,
		IID_PPV_ARGS(&state.graphicsPipelineState));
	assert(SUCCEEDED(hr));
	// ===


}

void PipelineStateManager::createTrianglePipeline() {

	PipelineState& state = pipelines_[PipelineType::Triangle];  // 先に登録

	// === SRVの設定 複数枚の設定をまとめて設定する設定する設定する設定する設定ｓｒsrsrsrrsrsrrrrrr
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0; // 0から始める
	descriptorRange[0].NumDescriptors = 1; // 一つ。ここを増やすと同時に描画できる数が増える？
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // SRVを使う。
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


	// === RootSignatureの設定
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT; // InputLayoutを使うことを許可

	// === Samplerの設定
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER; // L(
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
	staticSamplers[0].ShaderRegister = 0;// レジスタ番号0を使う
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

	// RootParameterの設定(追加時は随時長さを足す）
	D3D12_ROOT_PARAMETER rootParameters[3] = {};

	// 色
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].Descriptor.ShaderRegister = 0; // b0
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // 全てのシェーダーから見えるように
	
	// 座標
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[1].Descriptor.ShaderRegister = 0; // b0
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; // 全てのシェーダーから見えるように
	
	// 上のSRVの複数枚の設定の設定の（ｒｙ
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // DescriptorTable15
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShader5
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange; // Tableの中身の配列を指定
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange); // Table3

	descriptionRootSignature.pParameters = rootParameters;
	descriptionRootSignature.NumParameters = _countof(rootParameters);

	// シリアライズしてバイナリにする
	ID3DBlob* rootSignatureBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &rootSignatureBlob, &errorBlob);

	if (FAILED(hr))
	{
		/*Log(reinterpret_cast
			<char*>(errorBlob->GetBufferPointer()));*/
		assert(false);
	}

	// バイナリをもとに生成
	hr = device_->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(),
		rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&state.rootSignature));

	assert(SUCCEEDED(hr));
	// ===

	// === Pipelineの設定

	// InputLayoutの設定 (要は、ここでVertexShaderに頂点データのレイアウトをつたえる)
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[2] = {};

	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;//	 Vector4なので
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT; //		 Vector2なので
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;


	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	// BlendState0
	D3D12_BLEND_DESC blendDesc{};

	//すべての色要素を書き込む
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	//RasterizerState
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	//裏面(時計回り)を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	//三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	//Shaderをコンパイルする
	IDxcBlob* vertexShaderBlob = dxcCompiler_.CompileShader(L"./resources/Engine/Shader/Sprite.VS.hlsl",
		L"vs_6_0", dxcCompiler_.GetIDxcUtils(), dxcCompiler_.GetDxcCompiler(), dxcCompiler_.GetIncludeHandler());
	assert(vertexShaderBlob != nullptr);

	IDxcBlob* pixelShaderBlob = dxcCompiler_.CompileShader(L"./resources/Engine/Shader/Sprite.PS.hlsl",
		L"ps_6_0", dxcCompiler_.GetIDxcUtils(), dxcCompiler_.GetDxcCompiler(), dxcCompiler_.GetIncludeHandler());
	assert(pixelShaderBlob != nullptr);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = state.rootSignature; // RootSignature
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc; // InputLayout
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),
	vertexShaderBlob->GetBufferSize() };// VertexShader
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),
	pixelShaderBlob->GetBufferSize() };// PixelShader
	graphicsPipelineStateDesc.BlendState = blendDesc;// BlendState
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;// RasterizerState
	//書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	//利用するトポロジ(形状)のタイプ。三角形
	graphicsPipelineStateDesc.PrimitiveTopologyType =
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//どのように画面に色を打ち込むかの設定(気にしなくて良い)
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	//実際に生成
	hr = device_->CreateGraphicsPipelineState(&graphicsPipelineStateDesc,
		IID_PPV_ARGS(&state.graphicsPipelineState));
	assert(SUCCEEDED(hr));
	// ===

}

void PipelineStateManager::createModelPipeline(){

	PipelineState& state = pipelines_[PipelineType::Model];  // 先に登録

	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT; // InputLayoutを使うことを許可

	// === SRVの設定 複数枚の設定をまとめて設定する設定する設定する設定する設定ｓｒsrsrsrrsrsrrrrrr
	D3D12_DESCRIPTOR_RANGE descriptorRange[2] = {};
	descriptorRange[0].BaseShaderRegister = 0; // 0から始める
	descriptorRange[0].NumDescriptors = 1; // 一つ。ここを増やすと同時に描画できる数が増える？
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // SRVを使う。
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// シャドウマップ t1
	descriptorRange[1].BaseShaderRegister = 1;
	descriptorRange[1].NumDescriptors = 1;
	descriptorRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[1].OffsetInDescriptorsFromTableStart = 0;

	// === Samplerの設定
	D3D12_STATIC_SAMPLER_DESC staticSamplers[2] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER; // L(
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
	staticSamplers[0].ShaderRegister = 0;// レジスタ番号0を使う
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// シャドウマップ比較用 s1
	staticSamplers[1].Filter =
		D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	staticSamplers[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	staticSamplers[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	staticSamplers[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	staticSamplers[1].BorderColor =
		D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
	staticSamplers[1].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	staticSamplers[1].MinLOD = 0.0f;
	staticSamplers[1].MaxLOD = D3D12_FLOAT32_MAX;
	staticSamplers[1].ShaderRegister = 1;
	staticSamplers[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

	// RootParameterの設定(追加時は随時長さを足す）
	D3D12_ROOT_PARAMETER rootParameters[10] = {};

	// 色
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].Descriptor.ShaderRegister = 0; // b0
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // 全てのシェーダーから見えるように

	// 座標
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[1].Descriptor.ShaderRegister = 0; // b0
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; // 全てのシェーダーから見えるように

	// 上のSRVの複数枚の設定の設定の（ｒｙ
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // DescriptorTable15
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShader5
	rootParameters[2].DescriptorTable.pDescriptorRanges = &descriptorRange[0]; // Tableの中身の配列を指定
	rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;


	// DirectionalLight
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[3].Descriptor.ShaderRegister = 1; //レジスタ番号1を使う

	// SpotLight
	rootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[4].Descriptor.ShaderRegister = 2; //レジスタ番号2を使う

	// PointLight
	rootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[5].Descriptor.ShaderRegister = 3; //レジスタ番号3を使う


	// ボーンのスキニング情報
	rootParameters[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[6].Descriptor.ShaderRegister = 4;
	rootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	// アニメ有り無し
	rootParameters[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[7].Descriptor.ShaderRegister = 5;
	rootParameters[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	// 影行列
	rootParameters[8].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[8].Descriptor.ShaderRegister = 6;
	rootParameters[8].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;


	rootParameters[9].ParameterType =
		D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[9].DescriptorTable.pDescriptorRanges = &descriptorRange[1];
	rootParameters[9].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[9].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;


	descriptionRootSignature.pParameters = rootParameters;
	descriptionRootSignature.NumParameters = _countof(rootParameters);

	// レイアウト設定
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[5] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[2].SemanticName = "NORMAL";
	inputElementDescs[2].SemanticIndex = 0;
	inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	// ボーン情報
	inputElementDescs[3].SemanticName = "BLENDINDICES";
	inputElementDescs[3].SemanticIndex = 0;
	inputElementDescs[3].Format = DXGI_FORMAT_R32G32B32A32_UINT;
	inputElementDescs[3].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[4].SemanticName = "BLENDWEIGHT";
	inputElementDescs[4].SemanticIndex = 0;
	inputElementDescs[4].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[4].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;


	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	// シリアライズしてバイナリにする
	ID3DBlob* rootSignatureBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &rootSignatureBlob, &errorBlob);

	if (FAILED(hr))
	{
		//Log(reinterpret_cast
		//	<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}

	// バイナリをもとに生成
	hr = device_->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(),
		rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&state.rootSignature));

	assert(SUCCEEDED(hr));

	// BlendStat
	D3D12_BLEND_DESC blendDesc{};

	//すべての色要素を書き込む
	//blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	// BlendStat
	//D3D12_BLEND_DESC blendDesc{};

	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;

	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;

	blendDesc.RenderTarget[0].RenderTargetWriteMask =
		D3D12_COLOR_WRITE_ENABLE_ALL;

	//RasterizerState
	D3D12_RASTERIZER_DESC rasterizerDesc{};

	//裏面(時計回り)を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	//三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	//Shaderをコンパイルする
	IDxcBlob* vertexShaderBlob = dxcCompiler_.CompileShader(L"./resources/Engine/Shader/Model.VS.hlsl",
		L"vs_6_0", dxcCompiler_.GetIDxcUtils(), dxcCompiler_.GetDxcCompiler(), dxcCompiler_.GetIncludeHandler());
	assert(vertexShaderBlob != nullptr);

	IDxcBlob* pixelShaderBlob = dxcCompiler_.CompileShader(L"./resources/Engine/Shader/Model.PS.hlsl",
		L"ps_6_0", dxcCompiler_.GetIDxcUtils(), dxcCompiler_.GetDxcCompiler(), dxcCompiler_.GetIncludeHandler());
	assert(pixelShaderBlob != nullptr);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = state.rootSignature; // RootSignature
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc; // InputLayout
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),
	vertexShaderBlob->GetBufferSize() };// VertexShader
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),
	pixelShaderBlob->GetBufferSize() };// PixelShader
	graphicsPipelineStateDesc.BlendState = blendDesc;// BlendState
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;// RasterizerState
	//書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	//利用するトポロジ(形状)のタイプ。三角形
	graphicsPipelineStateDesc.PrimitiveTopologyType =
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//どのように画面に色を打ち込むかの設定(気にしなくて良い)
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	graphicsPipelineStateDesc.DepthStencilState = DirectXCommon::GetInstance()->GetDepthStencilDesc();
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	//実際に生成
	hr = device_->CreateGraphicsPipelineState(&graphicsPipelineStateDesc,
		IID_PPV_ARGS(&state.graphicsPipelineState));
	assert(SUCCEEDED(hr));

}


void PipelineStateManager::createParticlePipeline() {

	PipelineState& state = pipelines_[PipelineType::Particle];  // 先に登録

	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT; // InputLayoutを使うことを許可


	// === SRVの設定 複数枚の設定をまとめて設定する設定する設定する設定する設定ｓｒsrsrsrrsrsrrrrrr
	
	D3D12_DESCRIPTOR_RANGE particleRange[1] = {};
	particleRange[0].BaseShaderRegister = 0; // t0
	particleRange[0].NumDescriptors = 1;
	particleRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	particleRange[0].OffsetInDescriptorsFromTableStart =
		D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// テキスチャの設定？
	D3D12_DESCRIPTOR_RANGE textureRange[1] = {};
	textureRange[0].BaseShaderRegister = 1; // t1
	textureRange[0].NumDescriptors = UINT_MAX;
	textureRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	textureRange[0].OffsetInDescriptorsFromTableStart =
		D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// === Samplerの設定
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER; // L(
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
	staticSamplers[0].ShaderRegister = 0;// レジスタ番号0を使う
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

	// RootParameterの設定(追加時は随時長さを足す）
	D3D12_ROOT_PARAMETER rootParameters[3] = {};

	// 色
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].Descriptor.ShaderRegister = 0; // b0
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // 全てのシェーダーから見えるように

	// 座標
	rootParameters[1].ParameterType =
		D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;

	rootParameters[1].ShaderVisibility =
		D3D12_SHADER_VISIBILITY_VERTEX;

	rootParameters[1].DescriptorTable.pDescriptorRanges =
		particleRange;

	rootParameters[1].DescriptorTable.NumDescriptorRanges = 1;


	// テクスチャ
	rootParameters[2].ParameterType =
		D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;

	rootParameters[2].ShaderVisibility =
		D3D12_SHADER_VISIBILITY_PIXEL;

	rootParameters[2].DescriptorTable.pDescriptorRanges =
		textureRange;

	rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;


	descriptionRootSignature.pParameters = rootParameters;
	descriptionRootSignature.NumParameters = _countof(rootParameters);

	// レイアウト設定
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[2].SemanticName = "NORMAL";
	inputElementDescs[2].SemanticIndex = 0;
	inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;


	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	// シリアライズしてバイナリにする
	ID3DBlob* rootSignatureBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &rootSignatureBlob, &errorBlob);

	if (FAILED(hr))
	{
		//Log(reinterpret_cast
		//	<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}

	// バイナリをもとに生成
	hr = device_->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(),
		rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&state.rootSignature));

	assert(SUCCEEDED(hr));

	// BlendStat
	D3D12_BLEND_DESC blendDesc{};

	//すべての色要素を書き込む
	/*blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;*/


// BlendStat
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;

	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;

	blendDesc.RenderTarget[0].RenderTargetWriteMask =
		D3D12_COLOR_WRITE_ENABLE_ALL;

	//RasterizerState
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	//裏面(時計回り)を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	//三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	//Shaderをコンパイルする
	IDxcBlob* vertexShaderBlob = dxcCompiler_.CompileShader(L"./resources/Engine/Shader/Particle.VS.hlsl",
		L"vs_6_0", dxcCompiler_.GetIDxcUtils(), dxcCompiler_.GetDxcCompiler(), dxcCompiler_.GetIncludeHandler());
	assert(vertexShaderBlob != nullptr);

	IDxcBlob* pixelShaderBlob = dxcCompiler_.CompileShader(L"./resources/Engine/Shader/Particle.PS.hlsl",
		L"ps_6_0", dxcCompiler_.GetIDxcUtils(), dxcCompiler_.GetDxcCompiler(), dxcCompiler_.GetIncludeHandler());
	assert(pixelShaderBlob != nullptr);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = state.rootSignature; // RootSignature
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc; // InputLayout
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),
	vertexShaderBlob->GetBufferSize() };// VertexShader
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),
	pixelShaderBlob->GetBufferSize() };// PixelShader
	graphicsPipelineStateDesc.BlendState = blendDesc;// BlendState
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;// RasterizerState
	//書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	//利用するトポロジ(形状)のタイプ。三角形
	graphicsPipelineStateDesc.PrimitiveTopologyType =
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//どのように画面に色を打ち込むかの設定(気にしなくて良い)
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	graphicsPipelineStateDesc.DepthStencilState = DirectXCommon::GetInstance()->GetDepthStencilDesc();
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	//実際に生成
	hr = device_->CreateGraphicsPipelineState(&graphicsPipelineStateDesc,
		IID_PPV_ARGS(&state.graphicsPipelineState));
	assert(SUCCEEDED(hr));


	/* ブレンド用に、Graphicパイプラインは６つ生成する　*/

	for (int i = 0; i < (int)BlendMode::Count; i++) {

		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{};

		// 共通設定
		desc.pRootSignature = PipelineStateManager::GetInstance()->GetPipelineState(PipelineType::Particle).rootSignature;
		desc.InputLayout = inputLayoutDesc;
		desc.VS = { vertexShaderBlob->GetBufferPointer(),
					vertexShaderBlob->GetBufferSize() };
		desc.PS = { pixelShaderBlob->GetBufferPointer(),
					pixelShaderBlob->GetBufferSize() };

		desc.RasterizerState = rasterizerDesc;

		desc.NumRenderTargets = 1;
		desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		desc.SampleDesc.Count = 1;
		desc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

		desc.DepthStencilState =
			DirectXCommon::GetInstance()->GetDepthStencilDesc();
		desc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

		// ブレンドモードだけ変更
		desc.BlendState = Particle::GetBlendDesc((BlendMode)i);

		// PSO生成
		HRESULT hr = device_->CreateGraphicsPipelineState(
			&desc,
			IID_PPV_ARGS(&blendGraphicsPipelineState[i])
		);

		assert(SUCCEEDED(hr));
	}
}

void PipelineStateManager::createShadowPipeline()
{
	PipelineState& state = pipelines_[PipelineType::Shadow];

	// ============================================================
	// RootSignature
	// b0 : Shadow用WVP
	// ============================================================

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};

	rootSignatureDesc.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_ROOT_PARAMETER rootParameter{};

	rootParameter.ParameterType =
		D3D12_ROOT_PARAMETER_TYPE_CBV;

	rootParameter.Descriptor.ShaderRegister = 0; // b0
	rootParameter.Descriptor.RegisterSpace = 0;

	rootParameter.ShaderVisibility =
		D3D12_SHADER_VISIBILITY_VERTEX;

	rootSignatureDesc.pParameters = &rootParameter;
	rootSignatureDesc.NumParameters = 1;

	// ============================================================
	// RootSignature生成
	// ============================================================

	ID3DBlob* rootSignatureBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	HRESULT hr = D3D12SerializeRootSignature(
		&rootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		&rootSignatureBlob,
		&errorBlob);

	if (FAILED(hr))
	{
		assert(false);
	}

	hr = device_->CreateRootSignature(
		0,
		rootSignatureBlob->GetBufferPointer(),
		rootSignatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&state.rootSignature));

	assert(SUCCEEDED(hr));

	// ============================================================
	// InputLayout
	// ============================================================
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[1]{};

	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format =
		DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = 0;

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};

	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);
	//// ModelのVertexBufferに合わせる
	//D3D12_INPUT_ELEMENT_DESC inputElementDescs[5]{};

	//inputElementDescs[0].SemanticName = "POSITION";
	//inputElementDescs[0].SemanticIndex = 0;
	//inputElementDescs[0].Format =
	//	DXGI_FORMAT_R32G32B32A32_FLOAT;
	//inputElementDescs[0].AlignedByteOffset =
	//	D3D12_APPEND_ALIGNED_ELEMENT;

	//inputElementDescs[1].SemanticName = "TEXCOORD";
	//inputElementDescs[1].SemanticIndex = 0;
	//inputElementDescs[1].Format =
	//	DXGI_FORMAT_R32G32_FLOAT;
	//inputElementDescs[1].AlignedByteOffset =
	//	D3D12_APPEND_ALIGNED_ELEMENT;

	//inputElementDescs[2].SemanticName = "NORMAL";
	//inputElementDescs[2].SemanticIndex = 0;
	//inputElementDescs[2].Format =
	//	DXGI_FORMAT_R32G32B32_FLOAT;
	//inputElementDescs[2].AlignedByteOffset =
	//	D3D12_APPEND_ALIGNED_ELEMENT;

	//inputElementDescs[3].SemanticName = "BLENDINDICES";
	//inputElementDescs[3].SemanticIndex = 0;
	//inputElementDescs[3].Format =
	//	DXGI_FORMAT_R32G32B32A32_UINT;
	//inputElementDescs[3].AlignedByteOffset =
	//	D3D12_APPEND_ALIGNED_ELEMENT;

	//inputElementDescs[4].SemanticName = "BLENDWEIGHT";
	//inputElementDescs[4].SemanticIndex = 0;
	//inputElementDescs[4].Format =
	//	DXGI_FORMAT_R32G32B32A32_FLOAT;
	//inputElementDescs[4].AlignedByteOffset =
	//	D3D12_APPEND_ALIGNED_ELEMENT;

	//D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};

	//inputLayoutDesc.pInputElementDescs =
	//	inputElementDescs;

	//inputLayoutDesc.NumElements =
	//	_countof(inputElementDescs);

	// ============================================================
	// Rasterizer
	// ============================================================

	D3D12_RASTERIZER_DESC rasterizerDesc{};

	rasterizerDesc.FillMode =
		D3D12_FILL_MODE_SOLID;

	rasterizerDesc.CullMode =
		D3D12_CULL_MODE_NONE;

	//rasterizerDesc.DepthBias = 1000;
	//rasterizerDesc.SlopeScaledDepthBias = 1.5f;
	//rasterizerDesc.DepthBiasClamp = 0.0f;
	//rasterizerDesc.DepthClipEnable = TRUE;
//
//	rasterizerDesc.FrontCounterClockwise = FALSE;
//rasterizerDesc.DepthClipEnable = TRUE;
//rasterizerDesc.MultisampleEnable = FALSE;
//rasterizerDesc.AntialiasedLineEnable = FALSE;
//rasterizerDesc.ForcedSampleCount = 0;
//rasterizerDesc.ConservativeRaster =
//    D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	// ============================================================
	// Blend
	// ShadowMapはカラーを書かないので基本不要
	// ============================================================

	D3D12_BLEND_DESC blendDesc{};
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;

	blendDesc.RenderTarget[0].BlendEnable = FALSE;
	blendDesc.RenderTarget[0].RenderTargetWriteMask =
		D3D12_COLOR_WRITE_ENABLE_ALL;

	// ============================================================
	// Shader
	// ============================================================

	IDxcBlob* vertexShaderBlob =
		dxcCompiler_.CompileShader(
			L"./resources/Engine/Shader/Shadow.VS.hlsl",
			L"vs_6_0",
			dxcCompiler_.GetIDxcUtils(),
			dxcCompiler_.GetDxcCompiler(),
			dxcCompiler_.GetIncludeHandler());

	assert(vertexShaderBlob != nullptr);

	// ============================================================
	// PSO
	// ============================================================

	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};

	graphicsPipelineStateDesc.pRootSignature =
		state.rootSignature;

	graphicsPipelineStateDesc.InputLayout =
		inputLayoutDesc;

	graphicsPipelineStateDesc.VS =
	{
		vertexShaderBlob->GetBufferPointer(),
		vertexShaderBlob->GetBufferSize()
	};

	// PixelShaderは使用しない
	graphicsPipelineStateDesc.PS = {};

	graphicsPipelineStateDesc.BlendState =
		blendDesc;

	graphicsPipelineStateDesc.RasterizerState =
		rasterizerDesc;

	// カラーRTVは使わない
	graphicsPipelineStateDesc.NumRenderTargets = 0;

	// 深度だけ使う
	graphicsPipelineStateDesc.DSVFormat =
		DXGI_FORMAT_D32_FLOAT;

	graphicsPipelineStateDesc.PrimitiveTopologyType =
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	graphicsPipelineStateDesc.SampleDesc.Count = 1;

	graphicsPipelineStateDesc.SampleMask =
		D3D12_DEFAULT_SAMPLE_MASK;

	// ============================================================
	// Depth
	// ============================================================

	graphicsPipelineStateDesc.DepthStencilState =
	{};

	graphicsPipelineStateDesc.DepthStencilState.DepthEnable =
		TRUE;

	graphicsPipelineStateDesc.DepthStencilState.DepthWriteMask =
		D3D12_DEPTH_WRITE_MASK_ALL;

	graphicsPipelineStateDesc.DepthStencilState.DepthFunc =
		D3D12_COMPARISON_FUNC_LESS_EQUAL;

	// ============================================================
	// 作成
	// ============================================================

	hr = device_->CreateGraphicsPipelineState(
		&graphicsPipelineStateDesc,
		IID_PPV_ARGS(
			&state.graphicsPipelineState));

	assert(SUCCEEDED(hr));
}

void PipelineStateManager::createDebugLinePipeline() {

		PipelineState& state = pipelines_[PipelineType::DebugLine];  // 先に登録

		// === RootSignatureの設定
		D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
		descriptionRootSignature.Flags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT; // InputLayoutを使うことを許可

		// RootParameterの設定(追加時は随時長さを足す）
		D3D12_ROOT_PARAMETER rootParameters[1] = {};

		// 座標
		rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		rootParameters[0].Descriptor.ShaderRegister = 0; // b0
		rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; // 全てのシェーダーから見えるように

		descriptionRootSignature.pParameters = rootParameters;
		descriptionRootSignature.NumParameters = _countof(rootParameters);

		// シリアライズしてバイナリにする
		ID3DBlob* rootSignatureBlob = nullptr;
		ID3DBlob* errorBlob = nullptr;

		HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &rootSignatureBlob, &errorBlob);

		if (FAILED(hr))
		{
			/*Log(reinterpret_cast
				<char*>(errorBlob->GetBufferPointer()));*/
			assert(false);
		}

		// バイナリをもとに生成
		hr = device_->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(),
			rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&state.rootSignature));

		assert(SUCCEEDED(hr));
		// ===

		// === Pipelineの設定

		// InputLayoutの設定 (要は、ここでVertexShaderに頂点データのレイアウトをつたえる)
		D3D12_INPUT_ELEMENT_DESC inputElementDescs[2] = {};

		inputElementDescs[0].SemanticName = "POSITION";
		inputElementDescs[0].SemanticIndex = 0;
		inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;//	 Vector3なので
		inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

		inputElementDescs[1].SemanticName = "COLOR";
		inputElementDescs[1].SemanticIndex = 0;
		inputElementDescs[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;//	 Vector4なので
		inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;


		D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
		inputLayoutDesc.pInputElementDescs = inputElementDescs;
		inputLayoutDesc.NumElements = _countof(inputElementDescs);

		// BlendState0
		D3D12_BLEND_DESC blendDesc{};

		//すべての色要素を書き込む
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		//RasterizerState
		D3D12_RASTERIZER_DESC rasterizerDesc{};
		//裏面(時計回り)を表示しない
		rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
		//三角形の中を塗りつぶす
		rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

		//Shaderをコンパイルする
		IDxcBlob* vertexShaderBlob = dxcCompiler_.CompileShader(L"./resources/Engine/Shader/DebugLine.VS.hlsl",
			L"vs_6_0", dxcCompiler_.GetIDxcUtils(), dxcCompiler_.GetDxcCompiler(), dxcCompiler_.GetIncludeHandler());
		assert(vertexShaderBlob != nullptr);

		IDxcBlob* pixelShaderBlob = dxcCompiler_.CompileShader(L"./resources/Engine/Shader/DebugLine.PS.hlsl",
			L"ps_6_0", dxcCompiler_.GetIDxcUtils(), dxcCompiler_.GetDxcCompiler(), dxcCompiler_.GetIncludeHandler());
		assert(pixelShaderBlob != nullptr);

		D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
		graphicsPipelineStateDesc.pRootSignature = state.rootSignature; // RootSignature
		graphicsPipelineStateDesc.InputLayout = inputLayoutDesc; // InputLayout
		graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),
		vertexShaderBlob->GetBufferSize() };// VertexShader
		graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),
		pixelShaderBlob->GetBufferSize() };// PixelShader
		graphicsPipelineStateDesc.BlendState = blendDesc;// BlendState
		graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;// RasterizerState
		//書き込むRTVの情報
		graphicsPipelineStateDesc.NumRenderTargets = 1;
		graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		//利用するトポロジ(形状)のタイプ。線
		graphicsPipelineStateDesc.PrimitiveTopologyType =
			D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		//どのように画面に色を打ち込むかの設定(気にしなくて良い)
		graphicsPipelineStateDesc.SampleDesc.Count = 1;
		graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

		//実際に生成
		hr = device_->CreateGraphicsPipelineState(&graphicsPipelineStateDesc,
			IID_PPV_ARGS(&state.graphicsPipelineState));
		assert(SUCCEEDED(hr));
		// ===

}



void PipelineStateManager::createPostEffectPipeline() {


	PipelineState& state = pipelines_[PipelineType::PostEffect];  // 先に登録
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT; // InputLayoutを使うことを許可


	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0; // 3
	descriptorRange[0].NumDescriptors = 1; // 15
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // SRV5
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // Offset0


	// RootParameterの設定(追加時は随時長さを足す↓
	D3D12_ROOT_PARAMETER rootParameters[2] = {};

	// t0 s0 offscreenの、textureのデータ
	rootParameters[0].ParameterType =
		D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[0].ShaderVisibility =
		D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[0].DescriptorTable.pDescriptorRanges =
		descriptorRange;
	rootParameters[0].DescriptorTable.NumDescriptorRanges = 1; 

	// b1 ポストエフェクトパラメータ情報
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[1].Descriptor.ShaderRegister = 1; // b1
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

descriptionRootSignature.pParameters = rootParameters;
	descriptionRootSignature.NumParameters = _countof(rootParameters);

	D3D12_INPUT_ELEMENT_DESC inputElementDescs[2] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER; // L(
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
	staticSamplers[0].ShaderRegister = 0;// レジスタ番号0を使う
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

	// シリアライズしてバイナリにする
	ID3DBlob* rootSignatureBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &rootSignatureBlob, &errorBlob);

	if (FAILED(hr))
	{
		/*Log(reinterpret_cast
			<char*>(errorBlob->GetBufferPointer()));
		assert(false);*/
	}

	// バイナリをもとに生成
	ID3D12RootSignature* rootSignature = nullptr;
	hr = device_->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(),
		rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));

	assert(SUCCEEDED(hr));

	// BlendStat
	D3D12_BLEND_DESC blendDesc{};

	//すべての色要素を書き込む
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	//RasterizerState
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	//裏面(時計回り)を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	//三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	// ポストエフェクト
	IDxcBlob* postEffectVSBlob = dxcCompiler_.CompileShader(
		L"./resources/Engine/Shader/PostEffect.VS.hlsl",
		L"vs_6_0",
		dxcCompiler_.GetIDxcUtils(), dxcCompiler_.GetDxcCompiler(), dxcCompiler_.GetIncludeHandler());

	// ポストエフェクト
	IDxcBlob* postEffectPSBlob = dxcCompiler_.CompileShader(
		L"./resources/Engine/Shader/PostEffect.PS.hlsl",
		L"ps_6_0",
		dxcCompiler_.GetIDxcUtils(), dxcCompiler_.GetDxcCompiler(), dxcCompiler_.GetIncludeHandler());

	// ポストエフェクトの
	D3D12_GRAPHICS_PIPELINE_STATE_DESC postEffectDesc{};
	postEffectDesc.pRootSignature = rootSignature; // RootSignature
	postEffectDesc.InputLayout = inputLayoutDesc; // InputLayout
	postEffectDesc.VS = { postEffectVSBlob->GetBufferPointer(),
	postEffectVSBlob->GetBufferSize() };// VertexShader
	postEffectDesc.PS = { postEffectPSBlob->GetBufferPointer(),
	postEffectPSBlob->GetBufferSize() };// PixelShader
	postEffectDesc.BlendState = blendDesc;// BlendState
	postEffectDesc.RasterizerState = rasterizerDesc;// RasterizerState
	//書き込むRTVの情報
	postEffectDesc.NumRenderTargets = 1;
	postEffectDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	//利用するトポロジ(形状)のタイプ。三角形
	postEffectDesc.PrimitiveTopologyType =
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//どのように画面に色を打ち込むかの設定(気にしなくて良い)
	postEffectDesc.SampleDesc.Count = 1;
	postEffectDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	postEffectDesc.DepthStencilState.DepthEnable = false;
	postEffectDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;

	state.rootSignature = rootSignature;	

	hr = device_->CreateGraphicsPipelineState(
		&postEffectDesc,
		IID_PPV_ARGS(&state.graphicsPipelineState)
	);

}