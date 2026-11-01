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

class FieldExample :
    public Scene
{
public:

    void Init();
    void Update(Input* input);
    void Draw();

    void FieldLoad();


    /* 　オブジェクトの生成　 */
    void CreateGameObject(const uint32_t& type, const uint32_t& subType, Hierarchy<Model>& hierarchy);


    /* エディタで生成などされた際のラムダ式関数の登録 */
    void CreateFunctionRegion();
    void DeleteFunctionRegion();

private:

    Camera camera_;

    Model* playerModel_ = nullptr;
    Model* blockModel_ = nullptr;
    Model* goalModel_ = nullptr;

    // プレイヤー
    std::unique_ptr <DemoPlayer> player_;


    // ブロック等のリスト
    std::vector<std::unique_ptr<DemoFieldObject>> blocks_;

    // ギミックのリスト
    std::vector<std::unique_ptr<DemoGimmick>> gimmicks_;


    // 上ののさくじょ用のテンプレート
    template <typename T>
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

        objects.erase(it);
        return true;
    }
};

