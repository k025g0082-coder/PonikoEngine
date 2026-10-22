#pragma once

#include "HierarchyPanel.h"
#include "SceneViewport.h"

/* エディタのアセット管理パネル */

class AssetsPanel
{
public:

	void Initialize();
	void PanelView(bool& show);

	void Update();
	void Draw();

	// ヒエラルキパネルのアドレスをセット
	void SetHierarchyPanel(HierarchyPanel& h) { hierarchyPanel_ = &h; }

	ModelResource selectedHierarchyResource_;

private:

	/* 画面上にドロップされたobjの読み取り,ヒエラルキとしてついでに追加 */
	void AddDropModel();

	HierarchyPanel* hierarchyPanel_ = nullptr;

	/* 選択しているモデルのview */
	SceneViewport selectedModelViewport_;
	
	
	bool* showView;

};

