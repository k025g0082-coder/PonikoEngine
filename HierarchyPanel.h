#pragma once

#include "Model.h"
#include "Transform.h"

#include "GameObject.h"

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include <memory>
#include <unordered_map>
#include <utility>
#include <optional>


/* エディタのヒエラルキパネル */

// タイプ
enum class HierarchyType {
	kModel
};


template <typename Type>

/*  Hierarchyのひな型  */
struct Hierarchy {

	HierarchyType type_;
	std::string name_;
	std::string id_;   // IDなどあれば。モデルは↓のと連携する

	
	// オブジェクトのアドレスのポインタ
	Type* object = nullptr;

	// ゲームオブジェクトのポインタ（持つ場合）
	GameObject* gameObject_ = nullptr;

	// gameObjectが持つデータ。名前変えたいけど短いからこれでいいや
	nlohmann::json json_;


	// WorldTransform, ScreenTransformクラスの始祖。newしてキャストして使うw
	std::unique_ptr<BaseTransform> transform_ = nullptr;

	// 見えるか
	bool isView_ = false;

};

/*
	モデルは管理idと登録されたヒエラルキのモデルとで繋げることにする
*/



//* モデルのパスやidなどのリソース情報
struct ModelResource {

	std::string pathId_;     // パスおよびIDの意味を持つ

	Model* model_ = nullptr; // モデルの実体
	std::string name_;		 // 名前表示用

	bool isView_ = true;	 // 表示非表示

};

class Grid;

class HierarchyPanel
{
public:

	void Initialize(const std::string jsonPath ,Grid& g);

	// 常に更新
	void Update();
	void Draw();

	/* ImGuiパネル */
	void ViewPanel(bool& show);


	// 選択中のモデルのIndex.  nullが使いたいだけ
	void SetSelectedHierarchyModelIndex(std::optional<int> index) {
		if (index.has_value()) {
			selectedModelIndex_ = index;
		}
		else {
			selectedModelIndex_ = std::nullopt;
		}
	}

	// シーンのJsonパスで保存
	void HierarchySave(const std::string jsonPath);

	// シーンのJsonパスで読込
	void HierarchyLoad(const std::string jsonPath);


	/* 読み取り専用 */
	const std::vector<Hierarchy<Model>>& GetHierarchyModels() const { return hierarchyModels_; }
	const std::vector<ModelResource>& GetHierarchyModelResources() const { return hierarchyModelResouces_; }

	// GameObjectセッター
	void SetHierarchyModels(int index, GameObject* obj) { hierarchyModels_.at(index).gameObject_ = obj; }
	

	std::unordered_map<std::string, Model*>& GethierarchyModelResourceList() { return hierarchyModelResourceList_; }

	// ヒエラルキー - モデル追加
	void AddHierarchyModel(Hierarchy<Model>& h) { hierarchyModels_.push_back(std::move(h)); }

	// ヒエラルキー - モデル削除
	void DeleteHierarchyModel() { 
		hierarchyModels_.erase(hierarchyModels_.begin() + selectedModelIndex_.value()); 
		selectedModelIndex_ = std::nullopt;
	}

	void DeleteHierarchyModel(int index) { hierarchyModels_.erase(hierarchyModels_.begin() + index); }


	// ヒエラルキー - モデルのペースト ペーストされたヒエラルキモデルのワールドのポインタを返す
	WorldTransform* PasteHierarchyModel();


	// モデルリソース - 追加
	void AddModelResource(ModelResource& r) {
		hierarchyModelResouces_.push_back(r);
		hierarchyModelResourceList_[r.pathId_] = r.model_;
	}


	// アセット削除（ここに描くなAssetPanelにかけ）
	void DeleteHierarchyResouce(int index);


	// モデル　表示非表示切り替え
	void ChangeHierarchyModelView(bool b, int index);

private:

	// 選択中のヒエラルキモデル番号
	std::optional<int> selectedModelIndex_;


	// モデル
	std::vector<Hierarchy<Model>> hierarchyModels_;

	//* 各モデルのパスやIDをまとめたもの
	std::vector<ModelResource> hierarchyModelResouces_;

	//* 上のオブジェクトをパス名で登録しておく
	std::unordered_map<std::string, Model*> hierarchyModelResourceList_;


	// ロード時にクリア
	void HierarchyClear();


	// エディターのグリッド
	Grid* grid = nullptr;

};

