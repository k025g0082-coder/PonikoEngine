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

#include <string>

#include <memory>

#include "DemoFieldObject.h"
#include "DemoGimmick.h"

#include "DemoPlayer.h"

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
    void CreateGameObject(const uint32_t& type, const uint32_t& subType, Hierarchy<Model>& hierarchy);


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
    Model* blockModel_ = nullptr;
    Model* goalModel_ = nullptr;

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

    // リスト削除用のテンプレート
    //template <typename T>
    //bool DeleteObject(
    //    std::vector<std::unique_ptr<T>>& objects,
    //    const Hierarchy<Model>& hierarchy) {

    //    auto it = std::find_if(
    //        objects.begin(),
    //        objects.end(),
    //        [&hierarchy](const std::unique_ptr<T>& obj) {
    //            return obj->GetHierarchy() == &hierarchy;
    //        }
    //    );

    //    if (it == objects.end()) {
    //        return false;
    //    }

    //    objects.erase(it);
    //    return true;
    //}
    
    // リスト削除用           
    template<typename T>
    bool DeleteObject(
        std::vector<std::unique_ptr<T>>& objects,
        const Hierarchy<Model>& hierarchy) {

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

