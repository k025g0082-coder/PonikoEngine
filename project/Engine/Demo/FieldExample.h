#pragma once

/*　20261031

    フィールドのお試し

    1) エディタ側から、各オブジェクトのJSONを受け取る。
        位置情報なども入れる

    2) 位置情報など基本情報から、ギミッククラスで配置
        各ギミックの値の調整はIMGUIを参照できるようにする

   ..という一連のデモ

*/

#include "Scene.h"
#include "Camera.h"
#include "Model.h"

#include "Text.h"

#include <string>

#include <memory>

#include "DemoFieldObject.h"
#include "DemoGimmick.h"
#include "DemoFieldItem.h"

#include "DemoFieldMeteo.h"

#include "DemoPlayer.h"

#include "HierarchyNode.h"

#include <vector>
#include <algorithm>
#include <unordered_map>

class FieldExample :
    public Scene
{
public:

    FieldExample() : Scene("FieldExample") {}

    void Init();
    void Update(Input* input);
    void Draw();

    void FieldLoad();



    void FieldLayerLoad(std::string layerName);


    /* 　オブジェクトの生成　 */
    void CreateGameObject(const uint32_t& type, const uint32_t& subType, ModelNode& hierarchy);


    /* エディタで生成などされた際のラムダ式関数の登録 */
    void CreateFunctionRegion();
    void DeleteFunctionRegion();


    // 対応したFieldObjectをゲット
    static DemoFieldObject* GetUniqueIdToFieldObject(uint32_t uniqueId) {

        auto it = uniqueIdMap_.find(uniqueId);
        if (it != uniqueIdMap_.end()) {
            return it->second; // 見つかった場合は生ポインタを返す(.get())
        }
        return nullptr; // 見つからなかった場合は nullptr
    }

private:

    Camera camera_;

    Model* playerModel_ = nullptr;
    Model* itemModel_ = nullptr;
    Model* blockModel_ = nullptr;

    Model* meteoModel_ = nullptr;

    // プレイヤー
    std::unique_ptr <DemoPlayer> player_;


    // 生成時、ユニークＩＤを記録
    void RecordUniqueId(DemoFieldObject* object);

    // フィールドのUniqueIDとそのインスタンスの対応表
    static std::unordered_map<uint32_t, DemoFieldObject*> uniqueIdMap_;

    


    // ブロック等のリスト
    std::vector<std::unique_ptr<DemoFieldObject>> blocks_;

    // ギミックのリスト
    std::vector<std::unique_ptr<DemoGimmick>> gimmicks_;

    // アイテムのリスト
    std::vector<std::unique_ptr<DemoFieldItem>> items_;


    /* 隕石ｓ */
    std::vector<DemoFieldMeteo> meteos_;

    void EmitMeteo();

    AABB emitMeteoRangeAABB;
    float emitMeteoElapsedTimer = 0.0f;

    // 落下中
    ParticleParam meteoEmitParam;

    // 落下時
    ParticleParam meteoBreakEmitParam;


    // ナビ用文字
    Text naviText;


    // FieldObjectリスト - もしdeleteフラグがtrueなら削除
    template<typename T>
    void DeleteJudgeFieldObject(std::vector<std::unique_ptr<T>>& list) {

        std::erase_if(
            list, [](std::unique_ptr<T>& obj) {

                if (obj->IsDelete()) {

                    auto* model = dynamic_cast<ModelNode*>(obj.get()->GetHierarchy());

                    model->gameObject_ = nullptr; // 削除前にヒエラルキ側のGameObjectをnullにしてから
                    model->isView_ = false;       // ヒエラルキの表示禁止（描画禁止）
                    return true;
                }

                return false;
            }
        );
    }

    // リスト削除用           
    template<typename T>
    bool DeleteObject(
        std::vector<std::unique_ptr<T>>& objects,
        const ModelNode& hierarchy) {

        auto it = std::find_if(
            objects.begin(),
            objects.end(),
            [&hierarchy](const std::unique_ptr<T>& obj) {
                return obj->GetHierarchy() == &hierarchy;
            }
        );

        if (it == objects.end()) {
            return false;
        }

        
        /* 記録用マップからも削除 */
        DemoFieldObject* fieldObject = it->get();

        auto itt = uniqueIdMap_.find(fieldObject->GetUniqueID());
        if (itt != uniqueIdMap_.end())// 存在すれば？
            uniqueIdMap_.erase(itt);


        objects.erase(it);
        
        return true;
    }
};

