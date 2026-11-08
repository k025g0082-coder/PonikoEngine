#pragma once

#include "AABB.h"
#include "Model.h"
#include "Transform.h"

//
#include "HierarchyNode.h"


#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include <memory>
#include <unordered_map>
#include <utility>
#include <optional>



/* エディタのヒエラルキパネル */



class GameObject;
class AssetsPanel;
//
//template <typename Type>
//
///*  Hierarchyのひな型  */
//struct Hierarchy {
//
//	HierarchyType type_;
//	
//	std::string name_;
//	std::string layerName_; // 属してるレイヤーの名前
//
//	std::string id_;   // IDなどあれば。モデルは↓のと連携する
//
//	
//	// オブジェクトのアドレスのポインタ
//	Type* object = nullptr;
//
//	// 判定用
//	AABB aabb_; 
//
//	// ゲームオブジェクトのポインタ（持つ場合）
//	GameObject* gameObject_ = nullptr;
//
//	// gameObjectが持つデータ。名前変えたいけど短いからこれでいいや
//	nlohmann::json json_;
//
//
//	// WorldTransform, ScreenTransformクラスの始祖。newしてキャストして使うw
//	std::unique_ptr<BaseTransform> transform_ = nullptr;
//
//	// 見えるか
//	bool isView_ = false;
//
//};

/*
	モデルは管理idと登録されたヒエラルキのモデルとで繋げることにする
*/

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



class EditorGrid;

class HierarchyPanel
{
public:

	void Initialize(const std::string sceneName ,EditorGrid& g);

	// 常に更新
	void Update();
	void Draw();

	/* ImGuiパネル */
	void ViewPanel(bool& show);


	// 選択中のヒエラルキセット  nullが使いたいだけ
	void SetSelectedHierarchyModelIndex(std::optional<HierarchyNode*> modelNode) {

		if (modelNode.has_value()) {
			selectedHierarchyNode_ = modelNode;
		}
		else {
			selectedHierarchyNode_ = std::nullopt;
		}
	}

	// 選択中のヒエラルキ
	HierarchyNode* GetSelectedHierarchy() {
		if (selectedHierarchyNode_.has_value()) {
			return selectedHierarchyNode_.value();
		}
		else {
			return nullptr;
		}
	}

	// 設定されたJsonパスで保存
	void HierarchySave();

	// シーン名から読込
	void HierarchyLoad(const std::string sceneName);


	/* 読み取り専用 */
	const std::vector<std::unique_ptr<HierarchyNode>>& GetHierarchys() const { return hierarchys_; }

	/// <summary>
	/// 対象のレイヤー名からヒエラルキをレイヤーから一覧取得. ないとき/SetUseGameObjectLayer()がされていない場合は空
	/// </summary>
	/// <param name="layerName">レイヤー名</param>
	/// <returns></returns>
	std::vector<HierarchyNode*>& GetHierarchyModelLayer(std::string layerName){ 

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
			static std::vector<HierarchyNode*> empty;
			return empty;
		}

		auto itt = hierarchyToLayer_.find(layerName);

		if (itt == hierarchyToLayer_.end()) {
			static std::vector<HierarchyNode*> empty;
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

	/* レイヤー一覧 */
	const std::vector<HierarchyLayer>& GetHierarchyLayers()const  { return hierarchyLayers_; }	


	/* レイヤー名->属しているヒエラルキーのアドレス群 */
	std::unordered_map<
		std::string,
		std::vector<HierarchyNode*>> hierarchyToLayer_;


	// ヒエラルキー - 追加. レイヤー名とヒエラルキ
	void AddHierarchy(std::string layerName, std::unique_ptr<HierarchyNode> h);


	// ヒエラルキー - モデル削除
	bool DeleteHierarchyNode(HierarchyNode* target);

	// レイヤーとそれに紐づいたヒエラルキの削除 - (モデルしか対応してない)
	void DeleteHierarchyLayer(const std::string& layerName);


	/*
	　シーン側との連携。
	 　シーン側で、モデルヒエラルキの追加や削除を行う場合、ゲーム側で予め登録する関数のラムダ式を登録しておくと、ヒエラルキパネル側で自動的に呼ばれる
	   これによりヒエラルキーリストが更新された際の、シーン側のゲームオブジェクト生成削除処理をエディタ側で発火できる！
	*/

	std::function<void (const uint32_t& type, const uint32_t& subType, ModelNode& hierarchy)> gameObjectAdd_ = nullptr;
	std::function<void (ModelNode& hierarchy)> gameObjectDelete_ = nullptr;

	// ADD とかー＞　std::function createGameObject(type,subType)    
	// けす　　ー＞　std::function deleteGameObject(ヒエラルキ
	//  					ameOvjectにヒエラルキのあどれす持たせる　でそれを検索とかで一致した奴を削除


	// ゲームオブジェクトの追加をチェックする
	void GameObjectAddCheck();
	void GameObjectDeleteCheck(HierarchyNode* hierarchy);


	// ヒエラルキー - モデルのペースト ペーストされたヒエラルキモデルのワールドのポインタを返す
	WorldTransform* PasteHierarchyModel();




	/// <summary>
	/// モデルリソースに対応したヒエラルキーモデルを一斉削除
	/// もし削除したヒエラルキーにGameObjectとれんけいしたのがあった場合false
	/// </summary>
	/// <param name="index"> リソースのindex番号 </param>
	/// <returns>
	/// 削除成功ならtrue。GameObject連携があればfalse
	/// </returns>
	//void 
	// 
	// Hierarchy(int index);

	// モデル　表示非表示切り替え
	void ChangeHierarchyModelView(bool b, int index);
	void ChangeHierarchyLayer(bool b, std::string layerName);

	/// <summary>
	/// 連携を宣言したレイヤー名一覧のリストを取得
	/// </summary>
	/// <returns></returns>
	std::vector<std::string>* getAlignHierarchyLayerNames() { return &alignHierarchyLayerNames_; }

	// いま選択されてるレイヤー名
	std::string GetSelectedCurrentLayerName() { return selectedCurrentLayer_; }
	HierarchyLayerType GetSelectedCurrentLayerType() { return selectedCurrentLayerType_; }

private:

	// ロードセーブ行うパス
	std::string jsonPath_;


	// レイヤー追加時のウインドウ
	void AddLayerPopupWindow();

	// ロード時にクリア
	void HierarchyClear();


	// 選択しているヒエラルキIndex
	std::optional<int> selectedHierarchyIndex_;


	// 選択中のヒエラルキ
	std::optional<HierarchyNode*> selectedHierarchyNode_;


	// ヒエラルキーｓ
	std::vector<std::unique_ptr<HierarchyNode>> hierarchys_;



	std::vector<HierarchyLayer> hierarchyLayers_;	      // レイヤー一覧
	std::vector<std::string>    alignHierarchyLayerNames_;// 連携しているレイヤー名一覧

	// レイヤーが、以前開いていたかを記録する
	std::unordered_map<std::string, bool> previousLayerIsOpen;


	std::string selectedCurrentLayer_;// 現在選択されているレイヤー名
	HierarchyLayerType selectedCurrentLayerType_; // 現在選択されているレイヤータイプ

	std::string previousLayer_;// 前のレイヤー名


	// エディターのグリッド
	EditorGrid* grid = nullptr;

};

