#pragma once
#include "BaseFactory.h"
#include <string>

class Scene; // 前方宣言

// シーン専用の Factory（Key: std::string, Core: Scene, 引数なし）
using SceneFactory = BaseFactory<std::string, Scene>;

// 自動登録用のヘルパークラス
template <typename T>
class SceneRegisterer {
public:
    SceneRegisterer(const std::string& name) {
        // Factoryに「名前」と「new T(name) するラムダ」を登録
        SceneFactory::Register(name, []() {
            return std::make_unique<T>();
            });
    }
};

// 【 勝手に登録される 】シーンの.cppの末尾に書く自動登録マクロ
#define REGISTER_SCENE(ClassName) \
    namespace { \
        static SceneRegisterer<ClassName> reg_##ClassName(#ClassName); \
    }