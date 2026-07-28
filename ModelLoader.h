#pragma once

#include <assimp/Importer.hpp>      // Assimpのインポーター
#include <assimp/scene.h>           // シーンデータ構造
#include <assimp/postprocess.h>     // 後処理フラグ

#include <string>
#include <vector>

#include "Bone.h"
#include "Model.h"
#include "Animation.h"

struct ModelData; // 前方宣言ｗ

class ModelLoader
{
public:

    // importer から
    // aiMeah,　aiMaterial,　aiIndices　でModelData 取得
    static ModelData LoadObjModel(std::string modelPath);

   // 変換 aiMatrix4x4->Matrix4x4
    static Matrix4x4 ConvertMatrix(const aiMatrix4x4& mat)
    {

    Matrix4x4 r{};

    r.m[0][0] = mat.a1;
    r.m[0][1] = mat.b1;
    r.m[0][2] = mat.c1;
    r.m[0][3] = mat.d1;

    r.m[1][0] = mat.a2;
    r.m[1][1] = mat.b2;
    r.m[1][2] = mat.c2;
    r.m[1][3] = mat.d2;

    r.m[2][0] = mat.a3;
    r.m[2][1] = mat.b3;
    r.m[2][2] = mat.c3;
    r.m[2][3] = mat.d3;

    r.m[3][0] = mat.a4;
    r.m[3][1] = mat.b4;
    r.m[3][2] = mat.c4;
    r.m[3][3] = mat.d4;

    return r;
    };

    // 入れ替え座標変換用の配列
    static Matrix4x4 ConvertYZ()
    {
        Matrix4x4 m = MakeIdentity4x4();

        m.m[1][1] = 0;
        m.m[1][2] = 1;

        m.m[2][1] = 1;
        m.m[2][2] = 0;

        return m;
    }

private:

    // 骨読込　インデックス、メッシュ、格納っするvector
    static void LoadBones(uint32_t meshIndex, aiMesh* mesh, ModelData* modelData);

    // 親子関係読み取り
    static void LoadNode(aiNode* node, int parentIndex, ModelData* modelData);

    // アニメ読み取り
    static void LoadAnimation(const aiScene* scene, ModelData* modelData);

    
    static uint32_t boneCount;
};

