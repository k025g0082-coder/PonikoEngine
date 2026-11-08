#pragma once
#include "AssetsPanel.h"
#include "HierarchyPanel.h"
#include "Sprite.h"


#include <imgui.h>

// これ外部ライブラリ
#include "ImGuizmo.h"

#include "UISequencer.h"


#include "Vector.h"

#include <vector>
#include <string>


//	UIを組み立てするクラス。


enum class UIType {
	kNone,
	kButton,
};

struct UIElement {

	UIType type_ = UIType::kNone;  //	タイプ
	Sprite sprite_;				   //	スプライト

	std::string pathID_;			   //   アセット参照用

	std::string name_;                 //   エレメントの名前。
	std::string groupName_ = "Main";   //   グループの名前。

	UIElementAnimationDatas animationDatas_; // アニメーションのデータ？

	// SRT
	Vector3 position_;
	Vector3 scale_;
	Vector3 rotation_;

};

/* 一つのキャンバス内全てのUIデータ */
struct UIData {

	// 名前　キャンバスレイヤーの名前からもらう（予定）
	std::string name_;

	// 表示非表示
	bool isView_ = true;		  

	// UIElementたち
	std::vector<UIElement> elements_;

};


/*  
	シーン内UIやシーケンサ情報 
*/
struct SceneUIData {

	// UIElementのリストを含む、UIDataのリスト
	std::vector<UIData> elementDatas_;

	// シーケンサー
	UISequencer sequencer_;

};


//
class UIManager
{
public:
	/* （エディター内では）ｼﾝｸﾞﾙﾄﾝ */
	static UIManager* GetInstance();

	/// <summary>
	/// テキスチャからUIElementをつくる
	/// </summary>
	/// <param name="canvasLayerName">現在選択されているLayerName</param>
	/// <param name="texturePathId">TextureResourceのpathId</param>
	void AddUIElement(std::string canvasLayerName,std::string pathId);


	// 初期化
	void Initialize(std::string sceneName);


	// Elementロード
	void LoadElementData(std::string sceneName);

	// Elementセーブ
	void SaveElementData();

	/// <summary>
	/// シーン切り替えの際に
	/// </summary>
	/// <param name="sceneName">シーン名</param>
	void SwitchSceneUIData(std::string sceneName);



	// UIの更新。- PonikoEngine.cppの方にかいてる
	void UpdateElements();

	// UIの描画。- PonikoEngine.cppの方にかいてる（Spriteの描画順の都合上20261105）
	void DrawElements();


	
	// アニメーション補間
	void UpdateElementAnimation(UIElementAnimationDatas& animeDatas);



	// 指定タグのアニメ再生
	static void PlayAnimationElement(
		std::string canvasName,
		std::string elementName,
		std::string elementAnimationTag
	);



	// 選択されているUIElementのアニメーションビュー
	void SelectedElementAnimationView();


	//
	void SetAssetPanel(AssetsPanel* instance) { assetPanel_ = instance; }
	void SetHierarchyPanel(HierarchyPanel* instance) { hierarchyPanel_ = instance; }


	// 選択中のエレメント
	UIElement* GetSelectedElement() {
		return selectedElemet_;
	}

	//// タイムライン表示非表示
	//void SetViewTimeline(bool b) { isShowTimeline_ = b; }

private:

	// データを読み書きするＪｓｏｎＰａｔｈ
	std::string jsonPath;


	// 現在のシーンのUIData
	static SceneUIData* currentSceneUIData_;


	/*
		シーン名ー＞シーンのUIデータ のマップ
	*/
	std::unordered_map <
		std::string, SceneUIData> sceneUIDatas_;


	// エディタータイムライン表示
	void UIEditorTimeLine();

	// マウスからエレメントを選択
	void JudgeMouseSelectElement(UIElement& e,int index);

	// 選択しているエレメントを移動
	void MoveSelectElement();



	// 現在選択されているエレメントのアニメーションのタグ名
	std::string currentSelectAnimationTag;


	// 選択中のエレメント
	UIElement* selectedElemet_ = nullptr;


	// 選択中のをドラッグ中か
	bool isDragSelectedElement_ = false;


	// 削除予定エレメントのindex
	int deleteElementIndex_ = -1;
	
	//
	Vector2 mousePos;
	Vector2 oldMousePos;

	//
	bool inside = false;


	HierarchyPanel* hierarchyPanel_ = nullptr;
	AssetsPanel* assetPanel_ = nullptr;

};

