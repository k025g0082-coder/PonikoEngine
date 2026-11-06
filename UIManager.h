#pragma once
#include "AssetsPanel.h"
#include "HierarchyPanel.h"
#include "Sprite.h"

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
	std::string groupName_ = "Main";   //   グループの名前。

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
	void Initialize();


	// Elementロード
	void LoadElementData();

	// Elementセーブ
	void SaveElementData();


	// UIの更新。- PonikoEngine.cppの方にかいてる
	void UpdateElements();

	// UIの描画。- PonikoEngine.cppの方にかいてる（Spriteの描画順の都合上20261105）
	void DrawElements();

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

	std::string jsonPath;

	// エディタータイムライン表示
	void UIEditorTimeLine();
	UISequencer sequencer_;





	// マウスからエレメントを選択
	void JudgeMouseSelectElement(UIElement& e,int index);

	// 選択しているエレメントを移動
	void MoveSelectElement();


	/*
		UIElementのリストを含む、UIData
	*/
	// UIDataのリスト
	std::vector<UIData> elementDatas_;


	// 現在選択されているレイヤー名
	//std::string selectedCanvasLayerName_;


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

