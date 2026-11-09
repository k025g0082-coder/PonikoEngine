#pragma once


#include "AABB.h"
#include "Model.h"
#include "Transform.h"

#include "Camera.h"



#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include <memory>
#include <unordered_map>
#include <utility>
#include <optional>

class GameObject;

// タイプ
enum class HierarchyType {
	kModel
};

/*  
	Hierarchyのノードのひな型
*/

class HierarchyNode
{
public:
	virtual ~HierarchyNode() = default;

	// タイプ
	HierarchyType type_;


	// 名前
	std::string name_ = "new_Node";

	// 属してるレイヤーの名前
	std::string layerName_; 


	// WorldTransform, ScreenTransformクラスの始祖。newしてキャストして使うw
	std::unique_ptr<BaseTransform> transform_ = nullptr;


	/* ツリー化するための構造 */
	HierarchyNode* parent_ = nullptr;						// 親
	std::vector<std::unique_ptr<HierarchyNode>> children_;  // 子たち

	// 見えるか
	bool isView_ = false;

	/*
		必ず実装
	*/
	virtual void Update() = 0;		// 各更新
	virtual void Draw(Camera& camera) = 0;		// 描画

	virtual void ImGuiView() = 0;   // ImGuiのView　あれば


	// 自分自身をJSONオブジェクトに変換して返す
	virtual nlohmann::json ToJson() const;

	// JSONオブジェクトから自分自身の値を復元する
	virtual void FromJson(const nlohmann::json& j);


	/*
		実装するかは選べる
	*/
	virtual void ChangeHierarchyLayer(bool b); // レイヤーを切り替えた際の処理


	// 自身のコピー（ディープコピー）を生成して返す
	virtual std::unique_ptr<HierarchyNode> Clone() const = 0;

};

/*
	モデル
*/
class ModelNode : public HierarchyNode {
public:

	// モデルのポインタ
	Model* modelResource_ = nullptr;

	// 判定用
	AABB aabb_;


	std::string modelId_;   // 連携させるモデルのID


	// シーン側と連携するゲームオブジェクトのポインタ（持つ場合）
	GameObject* gameObject_ = nullptr;


	// gameObjectが持つデータ。名前変えたいけど短いからこれでいいや
	nlohmann::json json_;

	void Update() override;
	void Draw(Camera& camera) override;
	void ImGuiView() override;


	// 自分自身をJSONオブジェクトに変換して返す
	virtual nlohmann::json ToJson() const;

	// JSONオブジェクトから自分自身の値を復元する
	virtual void FromJson(const nlohmann::json& j);


	// レイヤー切り替えた際
	void ChangeHierarchyLayer(bool b) override;


	std::unique_ptr<HierarchyNode> Clone() const override;
};



/*
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
*/