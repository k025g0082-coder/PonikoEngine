#pragma once

#include "Model.h"
#include "Transform.h"

#include "imgui.h"

// これ外部ライブラリ
#include "ImGuizmo.h"

#include "EffectSequencer.h"

#include "DirectionalLight.h"
#include "SpotLight.h"
#include "PointLight.h"

#include "Camera.h"

#include "SceneViewport.h"

#include <string>
#include <vector>

#include <memory>
#include <unordered_map>

class Editor
{
public:

	static Editor* GetInstance();

	void Initialize();

	// エンジン内のSceneの更新
	void Update();

	// エンジン内のSceneの描画
	void Draw();


	// タイプ
	enum class HierarchyType {
		kModel
	};

	template <typename Type>

	/* Hierarchy */
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

private:

	// エディターモードか
	bool isEditorMode_ = false;

	// デバッグ用
	Camera camera_;


	// Hierarchyタブ
	void HierarchyTab(bool& show);

	// シーンのJsonパスで保存
	void HierarchyLoad(const std::string jsonPath);
	void HierarchySave(const std::string jsonPath);

	ImGuizmo::OPERATION operation_ = ImGuizmo::TRANSLATE;


	// 今読み込んでるシーンのJsonパス
	std::string currentSceneJsonPath;
	std::string currentSceneName_;

	// モデル
	std::vector<Hierarchy<Model>> hierarchyModels_;

	//* 各モデルのパスやIDをまとめたもの
	std::vector<ModelResource> hierarchyModelResouces_;

	//* 上のオブジェクトをパス名で登録しておく
	std::unordered_map<std::string, Model*> hierarchyModelResourceList_;


	// 選択中のモデルのワールドトランスフォーム
	WorldTransform* selectedModelWorldTransform_ = nullptr;


	// との距離
	float selectedModelLength_ = 0.0f;

	// との球面座標系の色イロ
	float selectedSphereTheta_;
	float selectedSpherePhi_;

	// 選択中のモデルのView更新
	void SelectModelUpdateView();

	// 非選択時のカメラ移動
	void FreeMoveCamera();


	//　ライティング用
	void LightingManagerTab(bool& show);


	//　アセット一覧用
	void AssetsTab(bool& show);


	float camDistance = 8.0f;
	void EffectEditorTimeLine();

	/* エフェクト専用のScene部屋 */
	SceneViewport effectViewport_;


	/* タイムラインの */
	EffectSequencer effectSequencer_;

	Vector3 ExtractScale(const Matrix4x4& m);
	Vector3 ExtractRotation(const Matrix4x4& m, const Vector3& scale);
};

