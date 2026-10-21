#pragma once

#include "Model.h"

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
		std::string path_; // モデルなどのあればパス

		// オブジェクトのアドレスのポインタ
		Type* object = nullptr;

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

	// 選択中モデル
	Model* selectedModel_ = nullptr;


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

	float camDistance = 8.0f;
	void EffectEditorTimeLine();

	/* エフェクト専用のScene部屋 */
	SceneViewport effectViewport_;


	/* タイムラインの */
	EffectSequencer effectSequencer_;

	Vector3 ExtractScale(const Matrix4x4& m);
	Vector3 ExtractRotation(const Matrix4x4& m, const Vector3& scale);
};

