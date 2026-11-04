#pragma once

#include "AABB.h"
#include "Model.h"
#include "Transform.h"

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

class GameObject;


template <typename Type>

/*  Hierarchyのひな型  */
struct Hierarchy {

	HierarchyType type_;
	
	std::string name_;
	std::string layerName_; // 属してるレイヤーの名前

	std::string id_;   // IDなどあれば。モデルは↓のと連携する

	
	// オブジェクトのアドレスのポインタ
	Type* object = nullptr;

	// 判定用
	AABB aabb_; 

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


//reiya-t
enum class HierarchyLayerType {

	// モデルレイヤー
	kModelLayer, 

	// キャンバスレイヤー
	kCanvasLayer

};
static int currentSelectedLayerType_ = (int)HierarchyLayerType::kModelLayer;

// 
const char* kLayerTypeIcons[];
const char* kLayerTypeNames[];


// レイヤーデータ
struct HierarchyLayer {

	HierarchyLayerType layerType_ = HierarchyLayerType::kModelLayer;
	std::string layerName_;

	bool isView_ = true;
	bool isAlignGameScene = false;// シーン側のゲームオブジェクトとの連携中のレイヤーか。

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
	Hierarchy<Model>* GetSelectedHierarchyModel() {
		if (selectedModelIndex_.has_value()) {
			return hierarchyModels_.at(selectedModelIndex_.value()).get();
		}
		else {
			return nullptr;
		}
	}

	// シーンのJsonパスで保存
	void HierarchySave(const std::string jsonPath);

	// シーンのJsonパスで読込
	void HierarchyLoad(const std::string jsonPath);


	/* 読み取り専用 */
	const std::vector<std::unique_ptr<Hierarchy<Model>>>& GetHierarchyModels() const { return hierarchyModels_; }
	const std::vector<ModelResource>& GetHierarchyModelResources() const { return hierarchyModelResouces_; }

	/// <summary>
	/// 対象のレイヤー名からヒエラルキをレイヤーから一覧取得. ないとき/SetUseGameObjectLayer()がされていない場合は空
	/// </summary>
	/// <param name="layerName">レイヤー名</param>
	/// <returns></returns>
	std::vector<Hierarchy<Model>*>& GetHierarchyModelLayer(std::string layerName){ 

		// 該当レイヤー検索
		auto it = std::find_if(
			hierarchyLayers_.begin(),
			hierarchyLayers_.end(),
			[&layerName](const HierarchyLayer& l) {
				return l.layerName_ == layerName;
			}
		);

		// みつかんなかった
		if (it == hierarchyLayers_.end()) {
			static std::vector<Hierarchy<Model>*> empty;
			return empty;
		}

		auto itt = hierarchyModelToLayer_.find(layerName);

		if (itt == hierarchyModelToLayer_.end()) {
			static std::vector<Hierarchy<Model>*> empty;
			return empty;
		}

		return itt->second;
	}

	/// <summary>
	/// シーン側でヒエラルキーを取り出すのを有効化します
	/// </summary>
	/// <param name="layerName">レイヤー名</param>
	void SetUseGameObjectLayer(std::string layerName) {

		// 該当レイヤー検索
		auto it = std::find_if(
			hierarchyLayers_.begin(),
			hierarchyLayers_.end(),
			[&layerName](const HierarchyLayer& l) {
				return l.layerName_ == layerName;
			}
		);

		// 有効化
		if (it != hierarchyLayers_.end()) {

			it->isAlignGameScene = true;
		}

		// ---
		auto itt = std::find(
			alignHierarchyLayerNames_.begin(),
			alignHierarchyLayerNames_.end(),
			layerName
		);

		// なかったら名前一覧に追加
		if (itt == alignHierarchyLayerNames_.end()) {
			alignHierarchyLayerNames_.push_back(layerName);
		}


	};

	std::unordered_map<std::string, Model*>& GethierarchyModelResourceList() { return hierarchyModelResourceList_; }

	/* レイヤー一覧 */
	const std::vector<HierarchyLayer>& GetHierarchyLayers()const  { return hierarchyLayers_; }	


	/* レイヤー名->属しているモデルヒエラルキーのアドレス群 */
	std::unordered_map<
		std::string,
		std::vector<Hierarchy<Model>*>> hierarchyModelToLayer_;


	// ヒエラルキー - モデル追加. レイヤー名とヒエラルキ
	void AddHierarchyModel(std::string layerName, Hierarchy<Model>& h);

	// ヒエラルキー - モデル削除
	void DeleteHierarchyModel(int index = -1);

	// レイヤーとそれに紐づいたヒエラルキの削除 - (モデルしか対応してない)
	void DeleteHierarchyLayer(const std::string& layerName);


	/*
	　シーン側との連携。
	 　シーン側で、ヒエラルキの追加や削除を行う場合、ゲーム側で予め登録する関数のラムダ式を登録しておくと、ヒエラルキパネル側で自動的に呼ばれる
	   これによりヒエラルキーリストが更新された際の、シーン側のゲームオブジェクト生成削除処理をエディタ側で発火できる！
	*/

	std::function<void (const uint32_t& type, const uint32_t& subType, Hierarchy<Model>& hierarchy)> gameObjectAdd_ = nullptr;
	std::function<void (const Hierarchy<Model>& hierarchy)> gameObjectDelete_ = nullptr;

	// ADD とかー＞　std::function createGameObject(type,subType)    
	// けす　　ー＞　std::function deleteGameObject(ヒエラルキ
	//  					ameOvjectにヒエラルキのあどれす持たせる　でそれを検索とかで一致した奴を削除


	// ゲームオブジェクトの追加をチェックする
	void GameObjectAddCheck();
	void GameObjectDeleteCheck(Hierarchy<Model>* hierarchy);


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
	void ChangeHierarchyModelLayer(bool b, std::string layerName);

	/// <summary>
	/// 連携を宣言したレイヤー名一覧のリストを取得
	/// </summary>
	/// <returns></returns>
	std::vector<std::string>* getAlignHierarchyLayerNames() { return &alignHierarchyLayerNames_; }


	// いま選択されてるレイヤー名
	std::string GetSelectedCurrentLayerName() { return selectedCurrentLayer_; }

private:

	// レイヤー追加時のウインドウ
	void AddLayerPopupWindow();

	// ロード時にクリア
	void HierarchyClear();


	// 選択中のヒエラルキモデル番号
	std::optional<int> selectedModelIndex_;

	// モデル
	std::vector<std::unique_ptr<Hierarchy<Model>>> hierarchyModels_;

	//* 各モデルのパスやIDをまとめたもの
	std::vector<ModelResource> hierarchyModelResouces_;


	//* 上のオブジェクトをパス名で登録しておく
	std::unordered_map<std::string, Model*> hierarchyModelResourceList_;

	std::vector<HierarchyLayer> hierarchyLayers_;	      // レイヤー一覧
	std::vector<std::string>    alignHierarchyLayerNames_;// 連携しているレイヤー名一覧

	std::string selectedCurrentLayer_;// 現在選択されているレイヤー名


	// エディターのグリッド
	Grid* grid = nullptr;

};

