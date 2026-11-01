#pragma once

#include <nlohmann/json.hpp>

#include "GameObject.h"

#include "Model.h"
#include "Camera.h"

#include "WorldTransform.h"

/// <summary>
/// フィールド上のオブジェクトとしての基底クラス
/// </summary>
/// 

class DemoFieldObject : public GameObject{
public:

    // 必要な情報だけGameObjectに渡す
    DemoFieldObject(Model* model, Camera* camera, const GameObjectData& data_, nlohmann::json* json)
        : GameObject(model, camera, data_)
    {
        // タイプを設定
        type_ = static_cast<Type>(data_.objectID);
		subType_ = data_.subObjectID;

        // JSONのアドレスを取得
        json_ = json;

    };

    virtual ~DemoFieldObject() = default;

    virtual void Initialize() = 0;//
    virtual void Update() = 0;
    virtual void Draw() = 0;

    // GameObject) これももちろん
    virtual void ImGuiView() = 0;

    /* ImGuiウインドウ開始*/
    void BeginImGui();

    /* ImGuiウインドウ終了 */
    void EndImGui();

    // JSON更新
    void UpdateObjectTypeJson();

    /* オブジェクト種類 */
    enum class Type {

        kBlock,
        kGimmick,

        count
    };

    /* ギミック種類名を取得 */
    static const char* GetFieldObjectTypeName(Type type) {
        return fieldObjectTypeName[(int)type];
    }

protected:

    // ギミックの種類名
    static const char* fieldObjectTypeName[(int)Type::count];

    // 識別用タイプ
    Type type_;

    // 識別２．ギミックの種類など
    uint32_t subType_;


    /* ヒエラルキーのJSONのアドレス。データ管理よう. */
    nlohmann::json* json_ = nullptr;

private:
    bool showImGuiView = false;
};

/* ブロックなど静的なもの */
class DemoBlock : public DemoFieldObject {
public:
    
     using DemoFieldObject::DemoFieldObject;

     void Initialize();
     void Update();
     void Draw();

    // GameObject) これももちろん
     void ImGuiView();

private:
};