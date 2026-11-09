#include "Particle.h"

#include "WinApp.h"

#include "PipelineStateManager.h"

#include "DirectXCommon.h"
#include "DirectXUtility.h"

#include "ModelLoader.h"

#include "Matrix.h"

#define _USE_MATH_DEFINES
#include "math.h"

Particle* Particle::GetInstance() {
    static Particle instance;
    return &instance;
}

void Particle::Initialize() {
	
    ID3D12Device* device_ = DirectXCommon::GetDevice().Get();

   


    /* 残骸？*/
    

    //modelData = modeldata;

    //instancingResource =
    //DirectXUtility::CreateBufferResource(DirectXCommon::GetInstance()->GetDevice().Get(), sizeof(ParticleData) * kNumInstance);


    //instancingResource->Map(0, nullptr, reinterpret_cast<void**>(&instancingData));
    //
    ////単位行列を書きこんでおく
    //for (uint32_t index = 0; index < kNumInstance; ++index) {
    //    instancingData[index].WVP = MakeIdentity4x4();
    //    instancingData[index].World = MakeIdentity4x4();
    //    instancingData[index].color = Vector4{ 1.0f,1.0f,1.0f,1.0f };

    //}

 
    //D3D12_SHADER_RESOURCE_VIEW_DESC instancingSrvDesc{};

    //instancingSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
    //instancingSrvDesc.Shader4ComponentMapping =
    //    D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    //instancingSrvDesc.ViewDimension =
    //    D3D12_SRV_DIMENSION_BUFFER;

    //instancingSrvDesc.Buffer.FirstElement = 0;
    //instancingSrvDesc.Buffer.NumElements = kNumInstance;
    //instancingSrvDesc.Buffer.StructureByteStride =
    //    sizeof(TransformationMatrix);
    //instancingSrvDesc.Buffer.Flags =
    //    D3D12_BUFFER_SRV_FLAG_NONE;


    //UINT descriptorSize =
    //    DirectXCommon::GetInstance()->GetDevice()
    //    ->GetDescriptorHandleIncrementSize(
    //        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
    //    );

    //uint32_t index = DirectXCommon::GetInstance()->GetAllocateSrv();


    //instancingSrvHandleCPU =
    //    DirectXUtility::GetCPUDescriptorHandle(
    //        DirectXCommon::GetInstance()->GetSrvHeap().Get(),
    //        descriptorSize,
    //        index
    //    );
    //instancingSrvHandleGPU =
    //    DirectXUtility::GetGPUDescriptorHandle(
    //        DirectXCommon::GetInstance()->GetSrvHeap().Get(),
    //        descriptorSize,
    //        index
    //    );

    //DirectXCommon::GetInstance()->GetDevice().Get()->CreateShaderResourceView(
    //    instancingResource.Get(),
    //    &instancingSrvDesc,
    //    instancingSrvHandleCPU
    //);




    //// マテリアル用のリソース
    //materialResource = DirectXUtility::CreateBufferResource(DirectXCommon::GetInstance()->GetDevice().Get(), sizeof(Material));

    //materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
    //materialData->color = { 1.0f,1.0f,1.0f,1.0f }; // 白色
    //materialData->enableLighting = false; // ライティングはOFFにする
    //materialData->uvTransform = MakeIdentity4x4();

    //// 頂点データ
    //indexResource = DirectXUtility::CreateBufferResource(DirectXCommon::GetInstance()->GetDevice().Get(), sizeof(uint32_t) * modelData.indices.size());

    //// リソースの先頭のアドレスから使う
    //indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();

    ////使用するリソースのサイズはインデックスmodelData.indices.size()つ分のサイズ
    //indexBufferView.SizeInBytes =
    //    static_cast<UINT>(sizeof(uint32_t) * modelData.indices.size());//インデックスはuint32_tとする

    //indexBufferView.Format = DXGI_FORMAT_R32_UINT;

    //uint32_t* indexData = nullptr;

    //indexResource->Map(
    //    0,
    //    nullptr,
    //    reinterpret_cast<void**>(&indexData)
    //);

    //// コピー
    //std::memcpy(
    //    indexData,
    //    modelData.indices.data(),
    //    sizeof(uint32_t) * modelData.indices.size()
    //);

    //indexResource->Unmap(0, nullptr);



    //texture = new Texture();
    //texture->Initialize(modelData.materials.textureFilePath);

    ////頂点リソースを作る
    //vertexResource = DirectXUtility::CreateBufferResource(
    //    DirectXCommon::GetInstance()->GetDevice().Get(), sizeof(ModelVertexData) * modelData.vertices.size());

    //vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
    //vertexBufferView.SizeInBytes =
    //    UINT(sizeof(ModelVertexData) * modelData.vertices.size());
    //vertexBufferView.StrideInBytes = sizeof(ModelVertexData);

    //ModelVertexData* vertexData = nullptr;
    //vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));


    //std::memcpy(
    //    vertexData,
    //    modelData.vertices.data(),
    //    sizeof(ModelVertexData) * modelData.vertices.size()
    //);

    //vertexResource->Unmap(0, nullptr);

    // srt初期化
    //for (uint32_t i = 0; i < kNumInstance; ++i)
    //{
    //    emits[i] = MakeNewParticle({0,0});
    //}

}

// パーティクル用のモデル登録関数
void Particle::RegionParticleModel(std::string tag,std::string path) {

    // すでにおなじタグがあればリターン
    if (particleModelDatas_.contains(tag)) {
        return;
    }

    // すでに同じモデルを読み込んでいたらリターン
    for (const auto& [tag, data] : particleModelDatas_) {
        if (data.modelPath== path) {
            return;
        }
    }

    ParticleModelData* data = &particleModelDatas_[tag];

    data->modelPath = path;

    data->modelData = ModelLoader::LoadObjModel(path);

    data->instancingResource =
        DirectXUtility::CreateBufferResource(DirectXCommon::GetInstance()->GetDevice().Get(), sizeof(ParticleData) * kNumInstance);

    data->instancingResource->Map(0, nullptr, reinterpret_cast<void**>(&data->instancingData));

    // 頂点データ
    data->indexResource = DirectXUtility::CreateBufferResource(DirectXCommon::GetInstance()->GetDevice().Get(), sizeof(uint32_t) * data->modelData.indices.size());

    // リソースの先頭のアドレスから使う
    data->indexBufferView.BufferLocation = data->indexResource->GetGPUVirtualAddress();

    //使用するリソースのサイズはインデックスmodelData.indices.size()つ分のサイズ
    data->indexBufferView.SizeInBytes =
        static_cast<UINT>(sizeof(uint32_t) * data->modelData.indices.size());//インデックスはuint32_tとする

    data->indexBufferView.Format = DXGI_FORMAT_R32_UINT;

    uint32_t* indexData = nullptr;

    data->indexResource->Map(
        0,
        nullptr,
        reinterpret_cast<void**>(&indexData)
    );

    // コピー
    std::memcpy(
        indexData,
        data->modelData.indices.data(),
        sizeof(uint32_t) * data->modelData.indices.size()
    );

      D3D12_SHADER_RESOURCE_VIEW_DESC instancingSrvDesc{};

        instancingSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
        instancingSrvDesc.Shader4ComponentMapping =
            D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        instancingSrvDesc.ViewDimension =
            D3D12_SRV_DIMENSION_BUFFER;

        instancingSrvDesc.Buffer.FirstElement = 0;
        instancingSrvDesc.Buffer.NumElements = kNumInstance;
        instancingSrvDesc.Buffer.StructureByteStride =
            sizeof(ParticleData);
        instancingSrvDesc.Buffer.Flags =
            D3D12_BUFFER_SRV_FLAG_NONE;

    // SRV（GPUとかに必要な奴）登録
    UINT descriptorSize =
        DirectXCommon::GetInstance()->GetDevice()
        ->GetDescriptorHandleIncrementSize(
            D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
        );

    // 今のサイズに合わせて
    uint32_t index = DirectXCommon::GetInstance()->GetAllocateSrv();

    // ハンドルゲットだぜ！
    data->instancingSrvHandleCPU =
        DirectXUtility::GetCPUDescriptorHandle(
            DirectXCommon::GetInstance()->GetSrvHeap().Get(),
            descriptorSize,
            index
        );
    data->instancingSrvHandleGPU =
        DirectXUtility::GetGPUDescriptorHandle(
            DirectXCommon::GetInstance()->GetSrvHeap().Get(),
            descriptorSize,
            index
        );

    DirectXCommon::GetInstance()->GetDevice().Get()->CreateShaderResourceView(
        data->instancingResource.Get(),
        &instancingSrvDesc,
        data->instancingSrvHandleCPU
    );



    data->indexResource->Unmap(0, nullptr);

    // テキスチャ
    data->texture = new Texture();
    data->texture->Initialize(data->modelData.materials.textureFilePath);

    //頂点リソースを作る
    data->vertexResource = DirectXUtility::CreateBufferResource(
        DirectXCommon::GetInstance()->GetDevice().Get(), sizeof(ModelVertexData) * data->modelData.vertices.size());

    data->vertexBufferView.BufferLocation = data->vertexResource->GetGPUVirtualAddress();
    data->vertexBufferView.SizeInBytes =
        UINT(sizeof(ModelVertexData) * data->modelData.vertices.size());
    data->vertexBufferView.StrideInBytes = sizeof(ModelVertexData);

    ModelVertexData* vertexData = nullptr;
    data->vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

    // マテリアル用のリソース
    materialResource = DirectXUtility::CreateBufferResource(DirectXCommon::GetInstance()->GetDevice().Get(), sizeof(Material));

    materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
    materialData->color = { 1.0f,1.0f,1.0f,1.0f }; // 白色
    materialData->enableLighting = false; // ライティングはOFFにする
    materialData->uvTransform = MakeIdentity4x4();

    std::memcpy(
        vertexData,
        data->modelData.vertices.data(),
        sizeof(ModelVertexData) * data->modelData.vertices.size()
    );

    data->vertexResource->Unmap(0, nullptr);

    //単位行列を書きこんでおく
    for (uint32_t index = 0; index < kNumInstance; ++index) {
        data->instancingData[index].WVP = MakeIdentity4x4();
        data->instancingData[index].World = MakeIdentity4x4();
        data->instancingData[index].color = Vector4{ 1.0f,1.0f,1.0f,1.0f };
        data->instancingData[index].textureIndex = 0;
    }

};

// パーティクル用のテキスチャ登録関数
void Particle::RegionParticleTexture(std::string tag, std::string path) {

    // すでにおなじタグがあればリターン
    if (particleTextureDatas_.contains(tag)) {
        return;
    }

    // すでに同じテキスチャを読み込んでいたらリターン
    for (const auto& [tag, data] : particleTextureDatas_) {
        if (data->texturePath_ == path) {
            return;
        }
    }

    Texture* data = new Texture();
    data->Initialize(path);

    particleTextureDatas_[tag] = data;
}


void Particle::Update() {

    if (camera_ == nullptr) return;

    for (auto& [tag, data] : particleModelDatas_) {
        // ブレンド範囲をクリア
        for (auto& r : data.blendRanges) {
            r.start = 0;
            r.count = 0;
        }

        int instanceIndex = 0;

        // ブレンドモードごとに詰める（順番固定）
        for (int blend = 0; blend < (int)BlendMode::Count; ++blend) {   // BlendMode の数に合わせる
            data.blendRanges[blend].start = instanceIndex;

            for (uint32_t i = 0; i < kNumMaxInstances; ++i) {
                if (!emits[i].isActive) continue;
                if (emits[i].tag != tag) continue;
                if (static_cast<int>(emits[i].blendType) != blend) continue;
                if (instanceIndex >= kNumInstance) break;

                // カメラ目線
                if (emits[i].isBillboard)
                    emits[i].transform.rotation_ = camera_->GetRotation();

                emits[i].update();

                // 定数バッファに書き込み
                data.instancingData[instanceIndex].color = emits[i].color;
                Matrix4x4 wvp = Multiply(
                    emits[i].transform.matWorld_,
                    Multiply(camera_->GetViewMatrix(), camera_->GetProjectionMatrix())
                );
                data.instancingData[instanceIndex].WVP = wvp;
                data.instancingData[instanceIndex].World = emits[i].transform.matWorld_;
                data.instancingData[instanceIndex].textureIndex =
                    particleTextureDatas_[emits[i].textureTag]->GetSrvIndex();

                instanceIndex++;
            }
            data.blendRanges[blend].count = instanceIndex - data.blendRanges[blend].start;
        }

        data.currentDrawNum = instanceIndex;
    }

    /* 以前の*/
    
    /*
    
    // カメラ未設定なら帰る
    if (camera_ == nullptr) return;

    for (auto& [tag, data] : particleModelDatas_) {

        int instanceIndex = 0;

        // まいかいinstancingDataは、０～からなので
        for (uint32_t i = 0; i < kNumMaxInstances; ++i)
        {
            if (!emits[i].isActive)
                continue;

            if (emits[i].tag != tag)
                continue;

            if (instanceIndex >= kNumInstance)
                break;


            // カメラ目線
            if (emits[i].isBillboard)
                emits[i].transform.rotation_ = camera_->GetRotation();

            emits[i].update();

            // 定数バッファに
            data.instancingData[instanceIndex].color = emits[i].color;

            Matrix4x4 worldViewProjectionMatrix = Multiply(emits[i].transform.matWorld_, Multiply(camera_->GetViewMatrix(), camera_->GetProjectionMatrix()));

            data.instancingData[instanceIndex].WVP = worldViewProjectionMatrix;
            data.instancingData[instanceIndex].World = emits[i].transform.matWorld_;
            data.instancingData[instanceIndex].textureIndex = particleTextureDatas_[emits[i].textureTag]->GetSrvIndex();

            instanceIndex++;

        }
        data.currentDrawNum = instanceIndex;
    }
    */
}

void Particle::Draw()
{

    if (camera_ == nullptr) return;

    ID3D12GraphicsCommandList* commandList =
        DirectXCommon::GetInstance()->GetCommandList().Get();

    commandList->SetGraphicsRootSignature(
        PipelineStateManager::GetInstance()->GetPipelineState(PipelineType::Particle).rootSignature
    );

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    ID3D12DescriptorHeap* heaps[] = {
        DirectXCommon::GetInstance()->GetSrvHeap().Get()
    };
    commandList->SetDescriptorHeaps(1, heaps);

    for (auto& [tag, data] : particleModelDatas_) {
        if (data.currentDrawNum <= 0) continue;

        commandList->IASetVertexBuffers(0, 1, &data.vertexBufferView);
        commandList->IASetIndexBuffer(&data.indexBufferView);

        commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
        commandList->SetGraphicsRootDescriptorTable(1, data.instancingSrvHandleGPU);
        commandList->SetGraphicsRootDescriptorTable(
            2,
            DirectXCommon::GetInstance()->GetSrvHeap()->GetGPUDescriptorHandleForHeapStart()
        );

        // ブレンドモードごとに描画
        for (int blend = 0; blend < (int)BlendMode::Count; ++blend) {

            const auto& range = data.blendRanges[blend];
            if (range.count == 0) continue;

            // ここで PSO を切り替え
            commandList->SetPipelineState(
                PipelineStateManager::GetInstance()->GetParticleBlendPipelineState(
                    static_cast<BlendMode>(blend)
                )
            );

            commandList->DrawIndexedInstanced(
                UINT(data.modelData.indices.size()),
                range.count,          // インスタンス数
                0,                    // StartIndexLocation
                0,                    // BaseVertexLocation
                range.start           // ★ BaseInstance（ここが重要）
            );
        }
    }

    /* 以前の*/

    /*
    // カメラ未設定なら帰る
    if (camera_ == nullptr) return;

    ID3D12GraphicsCommandList* commandList =
        DirectXCommon::GetInstance()->GetCommandList().Get();

    //RootSignatureを設定。PSOに設定しているけど別途設定が必要
    commandList->SetGraphicsRootSignature(PipelineStateManager::GetInstance()->GetPipelineState(PipelineType::Particle).rootSignature);

    // PSOを設定
    //commandList->SetPipelineState(PipelineStateManager::GetInstance()->GetPipelineState(PipelineType::Particle).graphicsPipelineState);
    // 
  // ６つのブレンドモードから、PSOを設定
        commandList->SetPipelineState(PipelineStateManager::GetInstance()->GetParticleBlendPipelineState(BlendMode::kBlendModeMultily));

    commandList->IASetPrimitiveTopology(
        D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST
    );

    ID3D12DescriptorHeap* heaps[] = {
        DirectXCommon::GetInstance()->GetSrvHeap().Get()
    };

    commandList->SetDescriptorHeaps(1, heaps);
    
    // モデルごとに1回だけ描画
    for (auto& [tag, data] : particleModelDatas_)
    {
        if (data.currentDrawNum <= 0)
            continue;

      

        commandList->IASetVertexBuffers(
            0, 1, &data.vertexBufferView
        );

        commandList->IASetIndexBuffer(
            &data.indexBufferView
        );

        commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());

        commandList->SetGraphicsRootDescriptorTable(
            1,
            data.instancingSrvHandleGPU
        );
        
        commandList->SetGraphicsRootDescriptorTable(
            2,
            
            DirectXCommon::GetInstance()
            ->GetSrvHeap()
            ->GetGPUDescriptorHandleForHeapStart()
        );

        commandList->DrawIndexedInstanced(
            UINT(data.modelData.indices.size()),
            data.currentDrawNum,
            0,
            0,
            0
        );
    }
    */
}

// 生成
void Particle::MakeNewParticle(std::string tag, std::string textureTag, Vector3 pos, int num, ParticleParam param) {

    auto itm = particleModelDatas_.find(tag);

    if (itm == particleModelDatas_.end()) {
        return;
    }

    auto it = particleTextureDatas_.find(textureTag);

    if (it == particleTextureDatas_.end()) {
        return;
    }


    // secondはその値の右側（値側）を取得
    ParticleModelData* data = &itm->second;

    // １モデルの限界描画数超えてたら　リターン
    if (data->currentDrawNum +  num > kNumInstance) {
        return;
    }


    // 描画に使うテキスチャのアドレス登録
    data->texture = particleTextureDatas_[textureTag];


    // 描画数足すｗ
    data->currentDrawNum += num;

    int count = 0;

    for (int i = 0; i < kNumMaxInstances; i++)
    {
        if (emits[i].isActive) continue;

        emits[i].isActive = true;

        emits[i].ease.Reset();
        emits[i].timer = 0;

        // タグ名登録
        emits[i].tag = tag;
        emits[i].textureTag = textureTag;
        
        // 大きさランダム
        float random = DirectXUtility::RandomFloat(param.scaleAmpMin, param.scaleAmpMax);

        emits[i].transform.scale_ = Vector3(random, random, random);
        emits[i].transform.rotation_ = { 0,0,0 };
        emits[i].transform.translation_ = pos;

        emits[i].gravity = param.gravity;

        emits[i].isBillboard = param.isBilliboard;
        emits[i].isDilute = param.isDilute;

        emits[i].color = param.color;

        emits[i].blendType = param.blendMode_;


        // なんか１．０ｆでも速すぎるのでこうする
        float speed = param.speed * 0.1f;
        float speedAmp= param.speedAmp * 0.1f;

        emits[i].speed = param.speed + DirectXUtility::RandomFloat(-speedAmp,speedAmp);

        // ランダム
        emits[i].velocity = {
            DirectXUtility::RandomFloat(-1.0f, 1.0f),
            DirectXUtility::RandomFloat(-1.0f, 1.0f),
            DirectXUtility::RandomFloat(-1.0f, 1.0f)
        };

        // 方向ベクトルを正規化
        emits[i].direction = Normalize(emits[i].velocity);

        emits[i].init();

        // もし規定の数以上ならbreak
        count++;
        if (count >= num)
            break;
    }


}


// 生成
void Particle::MakeNewParticle(std::string tag, std::string textureTag, AABB aabb, int num, ParticleParam param) {

    // ランダム
    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_real_distribution<float> distX(aabb.min.x, aabb.max.x);
    std::uniform_real_distribution<float> distY(aabb.min.y, aabb.max.y);
    std::uniform_real_distribution<float> distZ(aabb.min.z, aabb.max.z);

    auto itm = particleModelDatas_.find(tag);

    if (itm == particleModelDatas_.end()) {
        return;
    }

    auto it = particleTextureDatas_.find(textureTag);

    if (it == particleTextureDatas_.end()) {
        return;
    }


    // secondはその値の右側（値側）を取得
    ParticleModelData* data = &itm->second;

    // １モデルの限界描画数超えてたら　リターン
    if (data->currentDrawNum + num > kNumInstance) {
        return;
    }


    // 描画に使うテキスチャのアドレス登録
    data->texture = particleTextureDatas_[textureTag];


    // 描画数足すｗ
    data->currentDrawNum += num;

    int count = 0;

    for (int i = 0; i < kNumMaxInstances; i++)
    {
        if (emits[i].isActive) continue;

        emits[i].isActive = true;

        emits[i].ease.Reset();
        emits[i].timer = 0;

        // タグ名登録
        emits[i].tag = tag;
        emits[i].textureTag = textureTag;

        // 大きさランダム
        float random = DirectXUtility::RandomFloat(param.scaleAmpMin, param.scaleAmpMax);

        emits[i].transform.scale_ = Vector3(random, random, random);
        emits[i].transform.rotation_ = { 0,0,0 };
        emits[i].transform.translation_ = { distX(gen), distY(gen), distZ(gen) }; // ランダム位置に設定

        emits[i].gravity = param.gravity;

        emits[i].isBillboard = param.isBilliboard;
        emits[i].isDilute = param.isDilute;

        emits[i].color = param.color;

        emits[i].blendType = param.blendMode_;

        // なんか１．０ｆでも速すぎるのでこうする
        float speed = param.speed * 0.1f;
        float speedAmp = param.speedAmp * 0.1f;

        emits[i].speed = param.speed + DirectXUtility::RandomFloat(-speedAmp, speedAmp);

        // ランダム
        emits[i].velocity = {
            DirectXUtility::RandomFloat(-1.0f, 1.0f),
            DirectXUtility::RandomFloat(-1.0f, 1.0f),
            DirectXUtility::RandomFloat(-1.0f, 1.0f)
        };

        // 方向ベクトルを正規化
        emits[i].direction = Normalize(emits[i].velocity);

        emits[i].init();

        // もし規定の数以上ならbreak
        count++;
        if (count >= num)
            break;
    }


}


D3D12_BLEND_DESC Particle::GetBlendDesc(BlendMode mode) {

    D3D12_BLEND_DESC desc{};

    // 全色要素を書き込む
    desc.RenderTarget[0].RenderTargetWriteMask =
        D3D12_COLOR_WRITE_ENABLE_ALL;

    auto& rt = desc.RenderTarget[0];

    // 基本設定
    rt.BlendEnable = TRUE;
    rt.BlendOp = D3D12_BLEND_OP_ADD;
    rt.BlendOpAlpha = D3D12_BLEND_OP_ADD;

    rt.SrcBlendAlpha = D3D12_BLEND_ONE;
    rt.DestBlendAlpha = D3D12_BLEND_ZERO;

    switch (mode) {
    case BlendMode::kBlendModeNone:
        rt.BlendEnable = FALSE;
        break;

    case BlendMode::kBlendModeNormal:
        rt.SrcBlend = D3D12_BLEND_SRC_ALPHA;
        rt.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
        break;

    case BlendMode::kBlendModeAdd:
        rt.SrcBlend = D3D12_BLEND_SRC_ALPHA;
        rt.DestBlend = D3D12_BLEND_ONE;
        break;
    case BlendMode::kBlendModeSubtract:
        // 減算合成
        rt.SrcBlend = D3D12_BLEND_SRC_ALPHA;
        rt.DestBlend = D3D12_BLEND_ONE;
        rt.BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;
        break;

    case BlendMode::kBlendModeMultily:
        // 乗算合成
        rt.SrcBlend = D3D12_BLEND_ZERO;
        rt.DestBlend = D3D12_BLEND_SRC_COLOR;
        rt.BlendOp = D3D12_BLEND_OP_ADD;
        break;

    case BlendMode::kBlendModeScreen:
        // スクリーン合成
        rt.SrcBlend = D3D12_BLEND_ONE;
        rt.DestBlend = D3D12_BLEND_INV_SRC_COLOR;
        rt.BlendOp = D3D12_BLEND_OP_ADD;
        break;

        // ....
    }

    return desc;
}



void Particle::Finalize() {
	for (auto& [tag, data] : particleModelDatas_) {
		delete data.texture;
	}
	for (auto& [tag, data] : particleTextureDatas_) {
		delete data;
	}
}
