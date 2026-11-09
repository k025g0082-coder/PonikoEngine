#pragma once
#include "AssetsPanel.h"
#include "HierarchyPanel.h"
#include "Sprite.h"


#include <imgui.h>

// これ外部ライブラリ
#include "ImGuizmo.h"

#include "UISequencer.h"



/* ここに必要な構造体があります */
#include "UIData.h"


#include "Vector.h"

#include <vector>
#include <string>


//	UIを組み立てするクラス。


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
	void UpdateElementAnimation(UIElement* element);



	// 指定タグのアニメ再生
	static void PlayAnimationElement(
		std::string canvasName,
		std::string elementName,
		std::string elementAnimationTag
	);



	// 選択されているUIElementのアニメーションビュー
	void SelectedElementAnimationView();


	//
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

};

