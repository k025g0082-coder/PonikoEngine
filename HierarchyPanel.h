#pragma once

#include "Model.h"
#include "Transform.h"

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

	// WorldTransform, ScreenTransformクラスの始祖。newしてキャストして使うw
	std::unique_ptr<BaseTransform> transform_ = nullptr;

};

/*
	モデルは管理idと登録されたヒエラルキのモデルとで繋げることにする
*/



//* モデルのパスやidなどのリソース情報
struct ModelResource {

	std::string pathId_;     // パスおよびIDの意味を持つ

	Model* model_ = nullptr; // モデルの実体
	std::string name_;		 // 名前表示用

};

class ImGuizmoGrid;

class HierarchyPanel
{
public:

	void Initialize(const std::string jsonPath ,ImGuizmoGrid& g);

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


	std::unordered_map<std::string, Model*>& GethierarchyModelResourceList() { return hierarchyModelResourceList_; }

	// ヒエラルキー - モデル追加
	void AddHierarchyModel(Hierarchy<Model>& h) { hierarchyModels_.push_back(std::move(h)); }

	// ヒエラルキー - モデル削除
	void DeleteHierarchyModel(int index) { hierarchyModels_.erase(hierarchyModels_.begin() + index); }


	// モデルリソース - 追加
	void AddModelResource(ModelResource& r) {
		hierarchyModelResouces_.push_back(r);
		hierarchyModelResourceList_[r.pathId_] = r.model_;
	}


	// アセット削除（ここに描くなAssetPanelにかけ）
	void DeleteHierarchyResouce(int index);

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
	ImGuizmoGrid* grid = nullptr;

};

