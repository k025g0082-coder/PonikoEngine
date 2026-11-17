#pragma once
#include "SceneViewport.h"

#include <unordered_map>
#include <memory>

class HierarchyPanel;

#include "Model.h"
#include "Texture.h"
#include "UIElement.h"

/* 
	モデルのパスやidなどのリソース情報
*/
struct ModelResource {

	std::string pathId_;     // パスおよびIDの意味を持つ

	std::unique_ptr <Model> model_; // モデルの実体
	std::string name_;		 // 名前表示用

	bool isView_ = true;	 // 表示非表示


	// パネルの所属フォルダ
	std::string folderName_ = "Main";

};


/*
	テクスチャーのパスやidなどのリソース情報
*/
struct TextureResource {

	std::string pathId_;     // パスおよびIDの意味を持つ

	std::unique_ptr <Texture> texture_; // 実体
	std::string name_;		 // 名前表示用


	// パネルの所属フォルダ
	std::string folderName_ = "Main";

};

// DragDropPayload用。　なんか２つ以上登録できないらしいので仕方ない
struct DragTextureResource {
	int index;
	char pathId[256];		// pathID。構造体とかstd::stringはわたせないらしい。
};


/*
	UIPrefabのパスやidなどのリソース情報
*/
struct UIElementResource {

	std::string pathId_;     // パスおよびIDの意味を持つ

	std::unique_ptr < UIElement > element_; // 実体
	std::string name_;		       // エレメント名


	// パネルの所属フォルダ
	std::string folderName_ = "Main";

};

// DragDropPayload用。　なんか２つ以上登録できないらしいので仕方ない
struct DragUIElementResource {
	int index;
	char name[256];		// prefabの名前。構造体とかstd::stringはわたせないらしい。
};

/* エディタのアセット管理パネル */
class AssetsPanel
{
public:

	// ｼﾝｸﾞﾙﾄﾝ
	static AssetsPanel* GetInstance();

	AssetsPanel(const AssetsPanel&) = delete;            // コピー禁止
	AssetsPanel& operator=(const AssetsPanel&) = delete; // コピー代入禁止


	void Initialize();
	void PanelView(bool& show);

	void AssetsLoad();
	void AssetsSave();

	void Update();
	void Draw();

	// タグー＞モデルのアドレス のリスト
	std::unordered_map<std::string, Model*>& GethierarchyModelResourceList() { return hierarchyModelResourceList_; }
	std::unordered_map<std::string, Texture*>& GethierarchyTextureResourceList() { return hierarchyTextureResourceList_; }
	
	std::vector<ModelResource>& GetHierarchyModelResources() { return hierarchyModelResouces_; }
	std::vector<TextureResource>& GetHierarchyTextureResources() { return hierarchyTextureResouces_; }
	std::vector<UIElementResource>& GetHierarchyUIElementResources() { return hierarchyUIElementResouces_; }

	
	// モデルリソース - 追加	
	void AddModelResource(ModelResource& r) {
		hierarchyModelResouces_.push_back(std::move(r));// 構造体内にunique_ptrがあるためコピー不可。moveする
		hierarchyModelResourceList_[r.pathId_] = r.model_.get();// アドレス
	}

	// モデルリソース - 削除
	void DeleteModelResource(int index) {
		hierarchyModelResourceList_.erase(hierarchyModelResouces_.at(index).pathId_);
		hierarchyModelResouces_.erase(hierarchyModelResouces_.begin() + index);
	}


	// Textureリソース - 追加
	void AddTextureResource(TextureResource& r) {
		hierarchyTextureResouces_.push_back(std::move(r));// 構造体内にunique_ptrがあるためコピー不可。moveする
		hierarchyTextureResourceList_[r.pathId_] = r.texture_.get();// アドレス
	}

	// Textureリソース - 削除
	void DeleteTextureResource(int index) {
		hierarchyTextureResourceList_.erase(hierarchyTextureResouces_.at(index).pathId_);
		hierarchyTextureResouces_.erase(hierarchyTextureResouces_.begin() + index);
	}


	// UIElementリソース - 追加	
	void AddUIElementResource(UIElementResource& e) {
		hierarchyUIElementResouces_.push_back(std::move(e));// 構造体内にunique_ptrがあるためコピー不可。moveする
		hierarchyUIElementResourceList_[e.pathId_] = e.element_.get();// アドレス
	}


	// ヒエラルキパネルのアドレスをセット
	void SetHierarchyPanel(HierarchyPanel* h) { hierarchyPanel_ = h; }


	// 選択されているモデルのリソース
	ModelResource selectedHierarchyResource_;


private:

	AssetsPanel() = default;
	~AssetsPanel() = default;


	// 画面上にドロップされたobjの読み取り,ヒエラルキとしてついでに追加
	void AddDropModel();

	// 画面上にドロップされたpngスプライトの読み取り,Assetsの素材として追加
	void AddDropSprite();


	// モデル一覧
	void ViewModelTreeNode();
	void ViewModelLeafTreeNode(int index, const ModelResource& resource);

	// テキスチャ一覧
	void ViewTextureTreeNode();

	void AddModelLayerPopupWindow();

	// UIプレふぁぶ一覧
	void ViewUIPrefabTreeNode();



	/* ヒエラルキの追加 */
	void AddHierarchyModel(const ModelResource& resource);


	// 一覧表示で使うボタンの開始
	void PushButtonStyle();

	// 一覧表示で使うボタンの終了
	void PopButtonStyle();


	HierarchyPanel* hierarchyPanel_ = nullptr;

	/* 選択しているモデルのview */
	SceneViewport selectedModelViewport_;


	//* 各モデルのパスやIDをまとめたもの
	std::vector<ModelResource> hierarchyModelResouces_;
	std::vector<TextureResource> hierarchyTextureResouces_;
	std::vector<UIElementResource> hierarchyUIElementResouces_;

	//* 上のオブジェクトをパス名->実体へのポインタ
	std::unordered_map<std::string, Model*> hierarchyModelResourceList_;
	std::unordered_map<std::string, Texture*> hierarchyTextureResourceList_;
	std::unordered_map<std::string, UIElement*> hierarchyUIElementResourceList_;
	
	std::vector<std::string> modelFolderNames_ = {
		"Main",
		"Enemy",
		"Boss",
		"Other",
	};

	std::vector<std::string> textureFolderNames_ = {
		"Main",
		"UI",
		"Effect",
	};

	std::vector<std::string> UIElementFolderNames_ = {
		"Main",
		"Title",
		"Field",
		"Other",
	};

	/* よみこむJSONのパス */
	std::string jsonPath;


	bool* showView;

};

