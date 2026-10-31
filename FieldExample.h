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

class FieldExample :
    public Scene
{
public:
    void Init();
    void Update(Input* input);
    void Draw();

    void FieldLoad();

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


};

