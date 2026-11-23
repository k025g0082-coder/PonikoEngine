
#include "UIElement.h"
#include "AssetsPanel.h"

#include "ImGuiManager.h"
#include "SceneComponentLoader.h"

#include <misc/cpp/imgui_stdlib.h>

void UIElementTexture::Initialize()
{
	/*
		Spriteの一斉読み込み
	*/

	// AssetPanelからテキスチャを貰う
	sprite_.Initialize(AssetsPanel::GetInstance()->GethierarchyTextureResourceList()[pathID_]);
	sprite_.SetAnchor(Sprite::anchor::Center);

	// SRT
	sprite_.GetScreenTransform().translation_ = position_;
	sprite_.GetScreenTransform().scale_ = scale_;
	sprite_.GetScreenTransform().rotation_ = rotation_;

	// もし０なら
	if (sprite_.GetScreenTransform().scale_ == Vector3{ 0,0,0 }) {

		sprite_.GetScreenTransform().scale_ = { 1.0f,1.0f,1.0f };
		scale_ = { 1.0f,1.0f,1.0f };

	}
}

void UIElementTexture::Update()
{

}
void UIElementTexture::Draw()
{
	sprite_.Draw();
}

//
void UIElementTexture::Reset()
{
	UIElement::Reset();//

	// 初期のSRTに戻す
	sprite_.GetScreenTransform().translation_ = position_;
	sprite_.GetScreenTransform().rotation_    = rotation_;
	sprite_.GetScreenTransform().scale_       = scale_;

}

// 保存
void UIElementTexture::Serialize(nlohmann::json& j) const
{
	// 基底のとほぼ一緒である
	UIElement::Serialize(j);
}

// 読み取り
void UIElementTexture::Deserialize(const nlohmann::json & j)
{
	// 基底のとほぼ一緒である
	UIElement::Deserialize(j);
}



void UIElementText::Initialize()
{
	//
	text_.Init();

	// SRT
	text_.GetScreenTransform().translation_ = position_;
	text_.GetScreenTransform().scale_ = scale_;
	text_.GetScreenTransform().rotation_ = rotation_;

	// もし０なら
	if (text_.GetScreenTransform().scale_ == Vector3{ 0,0,0 }) {

		text_.GetScreenTransform().scale_ = { 1.0f,1.0f,1.0f };
		scale_ = { 1.0f,1.0f,1.0f };

	}


	// もしAddElementで追加された場合
	if (text_.GetFontLayout().fontName.empty()) {

		text_.setFont("default");
		text_.SetFontSize(10);

		text = "Text";
	}
}

void UIElementText::Update()
{
	// text_.GetScreenTransform().UpdateMatrix();
}

void UIElementText::Draw()
{
	// 文字表示
	text_.drawString(text, TextAnchor::Center);
}

void UIElementText::Reset()
{
	UIElement::Reset();

	// テキストの方のテクスチャのSRTも戻す
	text_.GetScreenTransform().translation_ = position_;
	text_.GetScreenTransform().scale_ = scale_;
	text_.GetScreenTransform().rotation_ = rotation_;

}

void UIElementText::ImGuiView()
{
#ifdef USE_IMGUI
	
	ImGui::SeparatorText("Text");

	// 文字変更画面
	ImGui::InputText(" ", &text);
	text_.ImGuiView();

#endif
}

// 保存
void UIElementText::Serialize(nlohmann::json& j) const
{
	// 基底
	UIElement::Serialize(j);


	j["text"] = TextDatasToJson(text_.GetTextDatas());
	

}

// 読み取り
void UIElementText::Deserialize(const nlohmann::json & j)
{
	// 基底
	UIElement::Deserialize(j);

	//
	text_.Init();

	// 文字情報の読み取り
	if (j.contains("text")) {

		TextDatas data = JsonToTextDatas(j["text"]);

		text_.SetTextDatas(data);//
		text_.setFont(data.fontName);

		// よみこんだテキストの文字列にする
		text = text_.GetTextDatas().text;

	}
	else {
		text_.setFont("default");
		text = "default";
	}

}

/*
	アニメ再生
*/
void UIElement::PlayAnimation(std::string animeTag)
{
	// もしそのアニメーション名が含まれていたら
	if (animationDatas_.animations_.contains(animeTag)) {

		// 再生中に
		animationDatas_.animations_[animeTag].isPlaying = true;
		animationDatas_.animations_[animeTag].frame = 0.0f;// 再生時間はその都度リセット

		animationDatas_.currentTag_ = animeTag;//

	}

}

void UIElement::ResetAnimation()
{
	if (animationDatas_.currentTag_.empty())return;

	//// リセット
	//animationDatas_.currentTag_.clear();
	animationDatas_.animations_[animationDatas_.currentTag_].isPlaying = false;
	animationDatas_.animations_[animationDatas_.currentTag_].frame = 0.0f;

	// SRTを元のに戻す

}

// 子エレメント検索
UIElement* UIElement::FindChild(std::string childElementName)
{
	for (UIElement* child : childrens_) {

		// オリジナルの名前（element＿１の＿１とかついてない）の一致したらそのアドレスを返す
		if (child->originalElementName_ == childElementName) return child;

		// 孫以降も再帰で探す
		if (auto* found = child->FindChild(childElementName)) return found;

	}
	return nullptr;
	
}

/*
	親設定
*/
void UIElement::SetParent(UIElement* NewParent)
{
	/*
		すでに親がいたら、旧親の children_ から自分を外す
	*/
	if (parent_) {
		auto& siblings = parent_->childrens_;
		siblings.erase(std::remove(siblings.begin(), siblings.end(), this), siblings.end());
	}


	/* 親設定 */
	parent_ = NewParent;

	if (parent_) {

		/* 親対象のエレメントに子として自分のアドレスを追加 */
		parent_->childrens_.push_back(this);

		/* ScreenTransformも親子設定にする */
		GetScreenTransform().parent_ = &parent_->GetScreenTransform();

		// スクリーン上の座標ー＞親の相対座標に（座標引くだけ）
		GetScreenTransform().translation_ -= parent_->GetScreenTransform().translation_;

	}
	else {
		parent_ = nullptr;
	}
}

// 保存(共通)
void UIElement::Serialize(nlohmann::json& j) const
{
	j["type"] = type_;
	j["name"] = name_;
	j["originalName"] = originalElementName_;

	// 親がいた場合、親の名前
	if (parent_) {
		j["parent"] = parent_->name_;
	}

	j["pathID"] = pathID_;
	j["groupName"] = groupName_;

	// アニメデータ  (変換時のto_json,from_jsonはヘッダー内に定義してるので使える)
	j["animationData"] = animationDatas_;

	j["position"] = Vector3ToJson(position_);
	j["scale"] = Vector3ToJson(scale_);
	j["rotation"] = Vector3ToJson(rotation_);
}

// 読み取り(共通)
void UIElement::Deserialize(const nlohmann::json & j)
{
	if (j.contains("type"))
		type_ = j.at("type").get<UIType>();

	if (j.contains("name"))
		name_ = j.at("name").get<std::string>();

	if (j.contains("originalName"))
		originalElementName_ = j.at("originalName").get<std::string>();

	if (j.contains("parent"))
		parentName_ = j.at("parent").get<std::string>();

	if (j.contains("pathID"))
		pathID_ = j.at("pathID").get<std::string>();

	if (j.contains("groupName"))
		groupName_ = j.at("groupName").get<std::string>();

	if (j.contains("animationData"))
		animationDatas_ = j.at("animationData").get<UIElementAnimationDatas>();

	if (j.contains("position"))
		position_ = JsonToVector3(j.at("position"));

	if (j.contains("scale"))
		scale_ = JsonToVector3(j.at("scale"));

	if (j.contains("rotation"))
		rotation_ = JsonToVector3(j.at("rotation"));
}

void UIElement::Reset()
{
	//
	if (!animationDatas_.animations_.empty()) {
		animationDatas_.currentTag_ = animationDatas_.animations_.begin()->first;
	}
	else {
		animationDatas_.currentTag_ = "";
	}

	// 全アニメリセット
	for (auto& anime : animationDatas_.animations_) {
		anime.second.isPlaying = false;
		anime.second.frame = 0.0f;
	}

	// 子供も再帰的にリセット
	for (auto& child : childrens_) {
		child->Reset();
	}
}
