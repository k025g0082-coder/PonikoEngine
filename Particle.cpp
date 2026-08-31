#include "Particle.h"

#include "WinApp.h"

#include "PipelineStateManager.h"

#include "DirectXCommon.h"
#include "DirectXUtility.h"

#include "Matrix.h"

#define _USE_MATH_DEFINES
#include "math.h"

void Particle::Initialize(ModelData modeldata) {
	
    modelData = modeldata;

    instancingResource =
    DirectXUtility::CreateBufferResource(DirectXCommon::GetInstance()->GetDevice().Get(), sizeof(ParticleData) * kNumInstance);


    instancingResource->Map(0, nullptr, reinterpret_cast<void**>(&instancingData));
    
    //単位行列を書きこんでおく
    for (uint32_t index = 0; index < kNumInstance; ++index) {
        instancingData[index].WVP = MakeIdentity4x4();
        instancingData[index].World = MakeIdentity4x4();
        instancingData[index].color = Vector4{ 1.0f,1.0f,1.0f,1.0f };

    }

 
    D3D12_SHADER_RESOURCE_VIEW_DESC instancingSrvDesc{};

    instancingSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
    instancingSrvDesc.Shader4ComponentMapping =
        D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    instancingSrvDesc.ViewDimension =
        D3D12_SRV_DIMENSION_BUFFER;

    instancingSrvDesc.Buffer.FirstElement = 0;
    instancingSrvDesc.Buffer.NumElements = kNumInstance;
    instancingSrvDesc.Buffer.StructureByteStride =
        sizeof(TransformationMatrix);
    instancingSrvDesc.Buffer.Flags =
        D3D12_BUFFER_SRV_FLAG_NONE;


    UINT descriptorSize =
        DirectXCommon::GetInstance()->GetDevice()
        ->GetDescriptorHandleIncrementSize(
            D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
        );

    uint32_t index = DirectXCommon::GetInstance()->GetAllocateSrv();


    instancingSrvHandleCPU =
        DirectXUtility::GetCPUDescriptorHandle(
            DirectXCommon::GetInstance()->GetSrvHeap().Get(),
            descriptorSize,
            index
        );
    instancingSrvHandleGPU =
        DirectXUtility::GetGPUDescriptorHandle(
            DirectXCommon::GetInstance()->GetSrvHeap().Get(),
            descriptorSize,
            index
        );

    DirectXCommon::GetInstance()->GetDevice().Get()->CreateShaderResourceView(
        instancingResource.Get(),
        &instancingSrvDesc,
        instancingSrvHandleCPU
    );




    // マテリアル用のリソース
    materialResource = DirectXUtility::CreateBufferResource(DirectXCommon::GetInstance()->GetDevice().Get(), sizeof(Material));

    materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
    materialData->color = { 1.0f,1.0f,1.0f,1.0f }; // 白色
    materialData->enableLighting = false; // ライティングはOFFにする
    materialData->uvTransform = MakeIdentity4x4();

    // 頂点データ
    indexResource = DirectXUtility::CreateBufferResource(DirectXCommon::GetInstance()->GetDevice().Get(), sizeof(uint32_t) * modelData.indices.size());

    // リソースの先頭のアドレスから使う
    indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();

    //使用するリソースのサイズはインデックスmodelData.indices.size()つ分のサイズ
    indexBufferView.SizeInBytes =
        static_cast<UINT>(sizeof(uint32_t) * modelData.indices.size());//インデックスはuint32_tとする

    indexBufferView.Format = DXGI_FORMAT_R32_UINT;

    uint32_t* indexData = nullptr;

    indexResource->Map(
        0,
        nullptr,
        reinterpret_cast<void**>(&indexData)
    );

    // コピー
    std::memcpy(
        indexData,
        modelData.indices.data(),
        sizeof(uint32_t) * modelData.indices.size()
    );

    indexResource->Unmap(0, nullptr);



    texture = new Texture();
    texture->Initialize(modelData.materials.textureFilePath);

    //頂点リソースを作る
    vertexResource = DirectXUtility::CreateBufferResource(
        DirectXCommon::GetInstance()->GetDevice().Get(), sizeof(ModelVertexData) * modelData.vertices.size());

    vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
    vertexBufferView.SizeInBytes =
        UINT(sizeof(ModelVertexData) * modelData.vertices.size());
    vertexBufferView.StrideInBytes = sizeof(ModelVertexData);

    ModelVertexData* vertexData = nullptr;
    vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));


    std::memcpy(
        vertexData,
        modelData.vertices.data(),
        sizeof(ModelVertexData) * modelData.vertices.size()
    );

    vertexResource->Unmap(0, nullptr);



    // srt初期化
    for (uint32_t i = 0; i < kNumInstance; ++i)
    {
        emits[i] = MakeNewParticle({0,0});
    }

}

void Particle::Update(Camera camera) {

    // srt更新
    for (uint32_t i = 0; i < kNumInstance; ++i)
    {

        // カメラ目線
        if (emits[i].isBillboard)
            emits[i].transform.rotation_ = camera.GetRotation();


        emits[i].update();

        instancingData[i].color = emits[i].color;

        // 新しく
        if (!emits[i].isActive)
            emits[i] = MakeNewParticle({ 0,0 });

        Matrix4x4 worldViewProjectionMatrix = Multiply(emits[i].transform.matWorld_, Multiply(camera.GetViewMatrix(), camera.GetProjectionMatrix()));
        
        instancingData[i].WVP = worldViewProjectionMatrix;
        instancingData[i].World = emits[i].transform.matWorld_;

    }

}

void Particle::Draw() {

    ID3D12GraphicsCommandList* commandList = DirectXCommon::GetInstance()->GetCommandList().Get();

    commandList->IASetVertexBuffers(0, 1, &vertexBufferView);

    //RootSignatureを設定。PSOに設定しているけど別途設定が必要
    commandList->SetGraphicsRootSignature(PipelineStateManager::GetInstance()->GetPipelineState(PipelineType::Particle).rootSignature);
    // PSOを設定
    commandList->SetPipelineState(PipelineStateManager::GetInstance()->GetPipelineState(PipelineType::Particle).graphicsPipelineState);


    commandList->IASetIndexBuffer(&indexBufferView);

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  
    ID3D12DescriptorHeap* heaps[] = {
    DirectXCommon::GetInstance()->GetSrvHeap().Get()
    };
    
    commandList->SetDescriptorHeaps(1, heaps);

    commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
    commandList->SetGraphicsRootDescriptorTable(1, instancingSrvHandleGPU);

    // SRVDescriptorTableの先頭を設定。２，はRootParametor[2]の２
    commandList->SetGraphicsRootDescriptorTable(2, texture->GetTextureSrvHandleGPU());


    commandList->DrawIndexedInstanced(
        UINT(modelData.indices.size()),
        kNumInstance,
        0,
        0,
        0
    );
}

// 生成
ParticleEmit Particle::MakeNewParticle(Vector3 pos) {
    
    ParticleEmit emit;
    emit.transform.scale_ = { 1.0f,1.0f,1.0f };
    emit.transform.rotation_ = { 0,0,0 };
    emit.transform.translation_ = pos;

    emit.color ={ DirectXUtility::RandomFloat(0, 1.0f), DirectXUtility::RandomFloat(0, 1.0f),0,1.0f };// aka 

    emit.speed = DirectXUtility::RandomFloat(0, 0.1f);

    // ランダム
    emit.velocity = {
        DirectXUtility::RandomFloat(-1.0f, 1.0f),
        DirectXUtility::RandomFloat(-1.0f, 1.0f),
        DirectXUtility::RandomFloat(-1.0f, 1.0f)
    };

    // 方向ベクトルを正規化
    emit.direction = Normalize(emit.velocity);

    emit.init();

    return emit;
}