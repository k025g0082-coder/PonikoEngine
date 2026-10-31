#pragma once

#include "GameObject.h"

class DemoPlayer : public GameObject
{
public:

    // ★ GameObject のコンストラクタをそのまま引き継ぐ
    using GameObject::GameObject;

    void Update() override;
    void Draw() override;

    void ImGuiView() override;

private:

    Vector3 velocity_;// 移動量

};

