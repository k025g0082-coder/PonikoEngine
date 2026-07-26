#include "ModelLoader.h"

#include "Model.h"

#include "DirectXUtility.h"
#include <algorithm>

uint32_t ModelLoader::boneCount = 0;

ModelData ModelLoader::LoadObjModel(std::string meshPath) {

    // インポーターを作成
    Assimp::Importer importer;

    // 読み込む条件のフラグ？
    unsigned int flag;
    flag = aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_GenNormals |
        //aiProcess_CalcTangentSpace |
        //aiProcess_FlipUVs;
        aiProcess_MakeLeftHanded;         // ← 追加
        //\aiProcess_FlipWindingOrder;         // ← 追加

    // ファイルパス
   // std::string path = "resources/Models/" + modelName + "/" + modelName + ".obj";

    // ファイルを読み込む
    const aiScene* scene = importer.ReadFile(meshPath, flag);

    if (scene == nullptr || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || scene->mRootNode == nullptr)
        if (scene == nullptr || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || scene->mRootNode == nullptr)
        {
            return {};
        }

    ModelData modelData;
    aiMesh* mesh = scene->mMeshes[0]; // モデルデータ.

    /* こっから頂点読み取り */
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        ModelVertexData vertex{};

        // 座標 (地味に軸変換。)
        vertex.position = {
            -mesh->mVertices[i].x,
            mesh->mVertices[i].y,
            -mesh->mVertices[i].z,
            
        };
        /* アニメの時、fbxはこっちつかってた
         vertex.position = {
            mesh->mVertices[i].x,
           -mesh->mVertices[i].z,
            mesh->mVertices[i].y,
          
        };
        */

        vertex.position.w = 1.0f;// これがないと方向ベクトル扱いになって～～～ゴミ!

        // 法線
        if (mesh->HasNormals())
        {
            vertex.normal = {
                mesh->mNormals[i].x,
                mesh->mNormals[i].z,
                mesh->mNormals[i].y, 
};
        }

        // UV
        if (mesh->mTextureCoords[0])
        {
            vertex.texcoord = {
                mesh->mTextureCoords[0][i].x,
                mesh->mTextureCoords[0][i].y
            };
        }

        modelData.vertices.push_back(vertex);

      /*  Vector3 min = { FLT_MAX, FLT_MAX, FLT_MAX };
        Vector3 max = { -FLT_MAX,-FLT_MAX,-FLT_MAX };

        for (auto& v : modelData.vertices)
        {
            min.x = (std::min)(min.x, v.position.x);
            min.y = (std::min)(min.y, v.position.y);
            min.z = (std::min)(min.z, v.position.z);

            max.x = (std::max)(max.x, v.position.x);
            max.y = (std::max)(max.y, v.position.y);
            max.z = (std::max)(max.z, v.position.z);
        }*/

       /* DirectXUtility::Log("min %f %f %f\n", min.x, min.y, min.z);
        DirectXUtility::Log("max %f %f %f\n", max.x, max.y, max.z);*/
    }

    // 管理番号取得
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];

        for (unsigned int j = 0; j < face.mNumIndices; j++)
        {
            modelData.indices.push_back(
                face.mIndices[j]
            );
        }
    }

    // テキスチャパス取得
    aiMaterial* material =
        scene->mMaterials[mesh->mMaterialIndex];

    aiString texturePath;

    if (material->GetTexture(
        aiTextureType_DIFFUSE,
        0,
        &texturePath
    ) == AI_SUCCESS)
    {
        std::string path = texturePath.C_Str();

        // cube.png -> resource/Models/ ~ に文字結合する
        std::string directory =
            meshPath.substr(0, meshPath.find_last_of("/\\"));

        modelData.material.textureFilePath =
            directory + "/" + texturePath.C_Str();
    }


    for (unsigned int i = 0; i < scene->mNumMaterials; i++)
    {
        aiMaterial* mat = scene->mMaterials[i];

        aiString name;
        mat->Get(AI_MATKEY_NAME, name);

        DirectXUtility::Log(
            "Material %d : %s\n",
            i,
            name.C_Str()
        );

        DirectXUtility::Log(
            "Diffuse count = %d\n",
            mat->GetTextureCount(aiTextureType_DIFFUSE)
        );
    }

    DirectXUtility::Log(
        "vertex %d index %d\n",
        modelData.vertices.size(),
        modelData.indices.size()
    );

    printf("Material count : %d\n", scene->mNumMaterials);

    printf("Diffuse count : %d\n",
        material->GetTextureCount(aiTextureType_DIFFUSE));

    // ボーンがあったら
    if (mesh->mNumBones > 0) {

        // フラグON
        modelData.isSkinning = true;

        // 骨読込　インデックス、メッシュ、格納っするvector
        LoadBones(0,mesh,&modelData);

        // 親子関係読み取り
        LoadNode(
            scene->mRootNode,
            -1,
            &modelData
        );

        // アニメ読み取り
        LoadAnimation(scene,&modelData);

    }

    // 終わり
    return modelData;
}

// 骨読込　インデックス、メッシュ、格納っするvector
void ModelLoader::LoadBones(uint32_t meshIndex, aiMesh* mesh, ModelData* modelData) {

    // メッシュに登録されてる骨ぶん
    for (uint32_t bone = 0; bone < mesh->mNumBones; bone++) {

        aiBone* aiBoneData = mesh->mBones[bone];

        std::string boneName = aiBoneData->mName.C_Str();

        uint32_t boneIndex;

        // 同名のボーンがあるか端まで検索
        auto it = modelData->boneMap.find(boneName);

        if (it == modelData->boneMap.end()) {

            // 見つかったら
            boneIndex = boneCount++;

            // ボーン登録
            modelData->boneMap[boneName] = boneIndex;

            modelData->bones.emplace_back();

            modelData->bones[boneIndex].name = boneName;

            modelData->bones[boneIndex].offsetMatrix =
                ConvertMatrix(aiBoneData->mOffsetMatrix);

            modelData->boneInfos.emplace_back();

            modelData->boneInfos[boneIndex].offsetMatrix =
                ConvertMatrix(aiBoneData->mOffsetMatrix);
        }
        else {

            boneIndex = it->second;
        }

        // 重み登録
        for (uint32_t weight = 0;
            weight < aiBoneData->mNumWeights;
            weight++) {

            const aiVertexWeight& vertexWeight =
                aiBoneData->mWeights[weight];

            uint32_t vertexIndex =
                //meshEntries[meshIndex].baseVertex +
                vertexWeight.mVertexId;

            // ボーン追加
            modelData->vertices[vertexIndex].boneWeight.AddBoneData(
                boneIndex,
                vertexWeight.mWeight);
        }

        // コピー
        for (size_t i = 0; i < modelData->vertices.size(); i++)
        {
            //modelData->vertices[i].boneWeight = modelData->vertices[i].boneWeight;
        }
    }
}
//
//void ModelLoader::LoadNode(aiNode* node, int parentIndex, ModelData* modelData) {
//
//    std::string boneName = node->mName.C_Str();
//
//    int currentParent = parentIndex;
//
//    auto it = modelData->boneMap.find(boneName);
//
//    if (it != modelData->boneMap.end())
//    {
//        // このノードはボーン
//        uint32_t boneIndex = it->second;
//
//        if (boneIndex >= modelData->bones.size())
//        {
//            return;
//        }
//
//        modelData->bones[boneIndex].parentIndex = parentIndex;
//
//        modelData->bones[boneIndex].localMatrix =
//            ConvertMatrix(node->mTransformation);
//
//        // 初期姿勢
//        modelData->bones[boneIndex].localBindMatrix = modelData->bones[boneIndex].localMatrix;
//
//        // 子供も登録する？
//        if (parentIndex != -1)
//        {
//            modelData->bones[parentIndex].children.push_back(boneIndex);
//        }
//
//        currentParent = boneIndex;
//
//        Vector3 pos = {
//           modelData->bones[boneIndex].localMatrix.m[3][0],
//            modelData->bones[boneIndex].localMatrix.m[3][1],
//            modelData->bones[boneIndex].localMatrix.m[3][2]
//                };
//
//      /*  DirectXUtility::Log("local %f %f %f\n",
//            
//            pos.x,
//            pos.y,
//            pos.z);*/
//        DirectXUtility::Log("NODE\n");
//        for (int r = 0; r < 4; r++)
//        {
//            DirectXUtility::Log("%f %f %f %f\n",
//                modelData->bones[boneIndex].localMatrix.m[r][0],
//                modelData->bones[boneIndex].localMatrix.m[r][1],
//                modelData->bones[boneIndex].localMatrix.m[r][2],
//                modelData->bones[boneIndex].localMatrix.m[r][3]);
//        }
//    }
//
//    // 子供を読むため、再帰
//    for (uint32_t i = 0; i < node->mNumChildren; i++)
//    {
//        LoadNode(node->mChildren[i], currentParent, modelData);
//    }
//}

void ModelLoader::LoadNode(aiNode* node, int parentIndex, ModelData* modelData)
{
    std::string boneName = node->mName.C_Str();
    int currentParent = parentIndex;

    auto it = modelData->boneMap.find(boneName);
    if (it != modelData->boneMap.end())
    {
        uint32_t boneIndex = it->second;

        if (boneIndex < modelData->bones.size())
        {
            auto& bone = modelData->bones[boneIndex];
            bone.parentIndex = parentIndex;
            bone.localMatrix = ConvertMatrix(node->mTransformation);

            bone.localBindMatrix = bone.localMatrix; // バインドポーズ

            if (parentIndex != -1)
            {
                modelData->bones[parentIndex].children.push_back(boneIndex);
            }
            else
            {
                // ルートボーンとして記録（必要なら）
               // modelData->rootBoneIndices.push_back(boneIndex);
            }

            currentParent = static_cast<int>(boneIndex);
        }
    }

    // 子供は必ず読む
    for (uint32_t i = 0; i < node->mNumChildren; i++)
    {
        LoadNode(node->mChildren[i], currentParent, modelData);
    }
}

// アニメ読み取り
void ModelLoader::LoadAnimation(const aiScene* scene, ModelData* modelData) {

    for (uint32_t i = 0; i < scene->mNumAnimations; i++)
    {
        aiAnimation* animation = scene->mAnimations[i];

        Animation anim;

        anim.name =
            animation->mName.C_Str();
        anim.duration =
            (float)animation->mDuration;
        anim.ticksPerSecond =
            (float)animation->mTicksPerSecond;

        // ここで各ノードのアニメを全部取得できる
        // root, arm, leg など
        for (uint32_t j = 0;
            j < animation->mNumChannels;
            j++)
        {
            aiNodeAnim* channel =
                animation->mChannels[j];

            NodeAnimation nodeAnim;

            nodeAnim.boneName =
                channel->mNodeName.C_Str();

            // 1)　位置キー取得
            for (uint32_t k = 0;
                k < channel->mNumPositionKeys;
                k++)
            {
                aiVectorKey key =
                    channel->mPositionKeys[k];

                KeyframeVector3 pos;

                pos.time =
                    (float)key.mTime;
               
                /*
                pos.value =
                {
                    key.mValue.x,
                    key.mValue.z,
                    key.mValue.y,
                };     
                */

                pos.value =
                {
                    key.mValue.x,  key.mValue.z, 
                    key.mValue.y,
                  
                  

                };

                nodeAnim.translations.push_back(pos);
            }

            // 2)　拡大キー取得
            for (uint32_t k = 0;
                k < channel->mNumScalingKeys;
                k++)
            {
                aiVectorKey key =
                    channel->mScalingKeys[k];

                KeyframeVector3 pos;

                pos.time =
                    (float)key.mTime;

                pos.value =
                {
                    key.mValue.x,
                    key.mValue.y,
                    key.mValue.z
                };

                nodeAnim.scales.push_back(pos);
            }

            // 3)　回転キー取得
            for (uint32_t k = 0;
                k < channel->mNumRotationKeys;
                k++)
            {
                aiQuatKey key = channel->mRotationKeys[k];

                KeyframeQuaternion pos;

                pos.time =
                    (float)key.mTime;

              /*  pos.value =
                {
                    key.mValue.x,
                    key.mValue.z,
                    key.mValue.y,
                    key.mValue.w
                }; */
                pos.value =
                {
                    key.mValue.x,
                    key.mValue.y,
                    key.mValue.z,

                    key.mValue.w
                };
                DirectXUtility::Log(
                    "%s", channel->mNodeName.C_Str());
                DirectXUtility::Log(
                    "key rot = %f %f %f %f\n",
              
                    key.mValue.x,
                    key.mValue.y,
                    key.mValue.z,
                    key.mValue.w
                );

                nodeAnim.rotations.push_back(pos);
            }

            // ボーン一本分のを追加
            anim.nodeAnimations.push_back(nodeAnim);
        }

        // すべてのアニメを追加
        modelData->animations.push_back(anim);
    }
}