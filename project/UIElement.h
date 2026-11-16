#pragma once
#include "Sprite.h"
#include "Text.h"

#include "UIElementAnimation.h"

#include <memory>
#include <string>
#include <vector>


enum class UIType {
    kTexture,
    kText,
};

/*
    基底
*/
class UIElement {
public:

    virtual void Initialize() = 0;
    virtual void Update() = 0;
    virtual void Draw() = 0;


    // エレメントの横幅
    virtual float GetWidth() = 0;

    // エレメントの縦幅
    virtual float GetHeight() = 0;


    // エレメントの場所
    virtual Vector2 GetCenterPosition() = 0;

    // ドラッグ時のマウスの移動の加算
    virtual void SetElapsedDragVelocity(const Vector2& v) = 0;


    // スクリーンのトランスフォーム
    virtual ScreenTransform& GetScreenTransform() = 0;


    // エレメントの大きさ
    virtual Vector2 GetScale() = 0;


    // 選択時のIMGUI画面
    virtual void ImGuiView() = 0;




    UIType type_ = UIType::kTexture;  //	タイプ

    std::string pathID_;			   //   アセット参照用

    std::string name_;                 //   エレメントの名前。
    std::string groupName_ = "Main";   //   グループの名前。


    /*
        アニメーション
    */

    /// <summary>
    /// 指定アニメーションを再生
    /// </summary>
    /// <param name="animeTag"></param>
    void PlayAnimation(std::string animeTag);


    UIElementAnimationDatas animationDatas_; // アニメーションのデータ

    // SRT
    Vector3 position_;
    Vector3 scale_;
    Vector3 rotation_;

    /// <summary>
    /// 目的のエレメント名から、ルートから子を再帰的に検索してそのアドレスを返す
    /// </summary>
    /// <param name="childElementName"></param>
    /// <returns></returns>
    UIElement* FindChild(std::string childElementName);

    /*
        新しい親を設定
    */
    void SetParent(UIElement* NewParent);

    // 親のアドレス
    UIElement* parent_ = nullptr;

    // 子のアドレス帳
    std::vector<UIElement*> childrens_;

    // 親の名前(データ輸送用,実際の表示には使わない)
    std::string parentName_;



    // どうしてもキャストさせる必要があったためここに定義しておき実装は派生で描くとする
    virtual void SetText(std::string t) { t; }


    // 保存
    virtual void Serialize(nlohmann::json& j) const = 0;

    // 読み込み
    virtual void Deserialize(const nlohmann::json& j) = 0;

};

// テキスチャー
class UIElementTexture : public UIElement {
public:

    void Initialize() override;
    void Update() override;
    void Draw()   override;

    float GetWidth() override {
        return static_cast<float>(sprite_.GetImageWidth());
    }
    float GetHeight() override {
        return static_cast<float>(sprite_.GetImageHeight());
    }

    Vector2 GetCenterPosition() override {
        return Vector2{ 
            sprite_.GetScreenTransform().matWorld_.m[3][0],
            sprite_.GetScreenTransform().matWorld_.m[3][1]
        };
    }

    void SetElapsedDragVelocity(const Vector2& v) override {
        sprite_.GetScreenTransform().translation_.x += v.x;
        sprite_.GetScreenTransform().translation_.y += v.y;
    };
    

    Vector2 GetScale() override {
        return Vector2{
            sprite_.GetScreenTransform().scale_.x,
            sprite_.GetScreenTransform().scale_.y 
        };
    }



    ScreenTransform& GetScreenTransform()override {
        return sprite_.GetScreenTransform();
    }

    // 選択時のIMGUI画面
    void ImGuiView() override{};


    // 保存
    void Serialize(nlohmann::json& j) const override;

    // 読み込み
    void Deserialize(const nlohmann::json& j) override;

private:
    Sprite sprite_;
};

// もじ
class UIElementText : public UIElement {
public:
    void Initialize() override;
    void Update() override;
    void Draw()   override;

    float GetWidth() override {
        return static_cast<float>(text_.GetTextWidth(text));
    }
    float GetHeight() override {
        return static_cast<float>(text_.GetTextHeight(text));
    }

    Vector2 GetCenterPosition() override {

        return Vector2{
            text_.GetScreenTransform().matWorld_.m[3][0],
            text_.GetScreenTransform().matWorld_.m[3][1],
        };

    }
    void SetElapsedDragVelocity(const Vector2& v) override {
        text_.GetScreenTransform().translation_.x += v.x;
        text_.GetScreenTransform().translation_.y += v.y;
    };

    Vector2 GetScale() override {
        return {1.0f,1.0f};
    }


    ScreenTransform& GetScreenTransform() override {
        return text_.GetScreenTransform();
    }

    // 選択時のIMGUI画面
    void ImGuiView()override;

    /// <summary>
    /// 表示させる文字セット
    /// </summary>
    /// <param name="t">文字列</param>
    void SetText(std::string t) override {
        text = t;
    }

    // 保存
    void Serialize(nlohmann::json& j) const override;

    // 読み込み
    void Deserialize(const nlohmann::json& j) override;

private:

    Text text_;
    fontLayout layout;

    std::string text;


};

/* 一つのキャンバス内全てのUIデータ */
struct UIData {

    // 名前　キャンバスレイヤーの名前からもらう（予定）
    std::string name_;

    // 表示非表示
    bool isView_ = true;

    // UIElementたち
    std::vector<std::unique_ptr<UIElement>> elements_;

};
