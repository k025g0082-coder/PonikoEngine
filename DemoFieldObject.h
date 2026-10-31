#pragma once

#include <nlohmann/json.hpp>

#include "GameObject.h"

#include "Model.h"
#include "Camera.h"

#include "WorldTransform.h"

/* 共通構造体 */
struct FieldObjectData {

    GameObjectData coreData_;// 基本

    //uint32_t linkID = 0;

    //nlohmann::json parameters; // ギミック固有のプロパティ

};

/// <summary>
/// フィールド上のオブジェクトとしての基底クラス
/// </summary>
/// 

class DemoFieldObject : public GameObject{
public:

    // 必要な情報だけGameObjectに渡す
    DemoFieldObject(Model* model, Camera* camera, const FieldObjectData& data_)
        : GameObject(model, camera, data_.coreData_)
    {
        // 何か必要な処理などあれば
    };

    virtual ~DemoFieldObject() = default;

    virtual void Initialize() = 0;//
    virtual void Update() = 0;
    virtual void Draw() = 0;

    // GameObject) これももちろん
    virtual void ImGuiView() = 0;


    /* オブジェクト種類 */
    enum class ObjectType {
        kBlock,
        kGimmick,
    };

protected:

};

/* ブロックなど静的なもの */
class DemoBlock : public DemoFieldObject {
public:
    
    // 基底のコンストラクタ
    using DemoFieldObject::DemoFieldObject;


    //void Initialize(Model* model, Camera* camera, const FieldData data_) override;
    //void Update() override;
    //void Draw() override;
private:
};