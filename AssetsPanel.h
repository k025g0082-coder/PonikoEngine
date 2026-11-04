#pragma once
#include "SceneViewport.h"

#include "Texture.h"

class HierarchyPanel;


/* 
	モデルのパスやidなどのリソース情報
*/
struct ModelResource {

	std::string pathId_;     // パスおよびIDの意味を持つ

	Model* model_ = nullptr; // モデルの実体
	std::string name_;		 // 名前表示用

	bool isView_ = true;	 // 表示非表示

};


/*
	テクスチャーのパスやidなどのリソース情報
*/
struct TextureResource {

	std::string pathId_;     // パスおよびIDの意味を持つ

	Texture* texture_;
	std::string name_;		 // 名前表示用

};


/* エディタのアセット管理パネル */
class AssetsPanel
{
public:

	void Initialize();
	void PanelView(bool& show);

	void AssetsLoad();
	void AssetsSave();

	void Update();
	void Draw();

	// タグー＞モデルのアドレス のリスト
	std::unordered_map<std::string, Model*>& GethierarchyModelResourceList() { return hierarchyModelResourceList_; }
	
	std::vector<ModelResource>& GetHierarchyModelResources() { return hierarchyModelResouces_; }

	
	// モデルリソース - 追加	
	void AddModelResource(ModelResource& r) {
		hierarchyModelResouces_.push_back(r);
		hierarchyModelResourceList_[r.pathId_] = r.model_;
	}

	// モデルリソース - 削除
	void DeleteModelResource(int index) {
		hierarchyModelResourceList_.erase(hierarchyModelResouces_.at(index).pathId_);
		hierarchyModelResouces_.erase(hierarchyModelResouces_.begin() + index);
	}


	// スプライトリソース - 追加
	void AddSpriteResource(TextureResource& r) {
		hierarchyTextureResouces_.push_back(r);
		hierarchyTextureResourceList_[r.pathId_] = r.texture_;
	}

	// スプライトリソース - 削除
	void DeleteSpriteResource(int index) {
		hierarchyTextureResourceList_.erase(hierarchyTextureResouces_.at(index).pathId_);
		hierarchyTextureResouces_.erase(hierarchyTextureResouces_.begin() + index);
	}

	// ヒエラルキパネルのアドレスをセット
	void SetHierarchyPanel(HierarchyPanel* h) { hierarchyPanel_ = h; }


	// 選択されているモデルのリソース
	ModelResource selectedHierarchyResource_;


private:

	/* 画面上にドロップされたobjの読み取り,ヒエラルキとしてついでに追加 */
	void AddDropModel();

	/* 画面上にドロップされたpngスプライトの読み取り,Assetsの素材として追加 */
	void AddDropSprite();


	HierarchyPanel* hierarchyPanel_ = nullptr;

	/* 選択しているモデルのview */
	SceneViewport selectedModelViewport_;
	


	//* 各モデルのパスやIDをまとめたもの
	std::vector<ModelResource> hierarchyModelResouces_;
	std::vector<TextureResource> hierarchyTextureResouces_;

	//* 上のオブジェクトをパス名で登録しておく
	std::unordered_map<std::string, Model*> hierarchyModelResourceList_;
	std::unordered_map<std::string, Texture*> hierarchyTextureResourceList_;
	

	/* よみこむJSONのパス */
	std::string jsonPath;


	bool* showView;

};

