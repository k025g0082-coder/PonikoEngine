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

// 各パネル
#include "HierarchyPanel.h"
#include "AssetsPanel.h"

// ...


#include "Matrix.h"

#include "Plane.h"

#include <string>
#include <vector>

#include <memory>
#include <unordered_map>


/* グリッドの設定まとめ */
class ImGuizmoGrid {

public:
	Matrix4x4 worldMatrix = MakeIdentity4x4();
	float gridSize = 10.0f;
	static float squareSize;
	const unsigned int kSubdivision = 1;

	Vector3 translation_;
	Vector3 rotation_;

	bool isSnap_ = false;

	// 判定用のplane
	Plane plane_;

	void MatrixUpdate();
	void ImGuiView();

	// マスの大きさで割ったのを返す
	static Vector3 snapGrid(Vector3 v);

};


class Editor
{
public:

	static Editor* GetInstance();

	void Initialize();

	// エンジン内のSceneの更新
	void Update();

	// エンジン内のSceneの描画
	void Draw();


	/* モデル選択モード */
	enum class ModelHierarchyMode {
		kSelectMode, // 個別にサイズを調整できる
		kCreateMode, // ドラッグで一気に配置
	};

private:

	// エディターモードか
	bool isEditorMode_ = false;

	// デバッグ用
	Camera camera_;

	/* 各パネル */
	HierarchyPanel hierarchyPanel_;
	AssetsPanel assetsPanel_;

	ImGuizmoGrid grid;
	
	//* モデルSRT操作モード
	ImGuizmo::OPERATION operation_ = ImGuizmo::TRANSLATE;

	// 今読み込んでるシーンのJsonパス
	std::string currentSceneJsonPath;
	std::string currentSceneName_;


	ModelHierarchyMode currentModelHierarchyMode_ = ModelHierarchyMode::kSelectMode;


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




	/* タイムラインの */
	EffectSequencer effectSequencer_;

	Vector3 ExtractScale(const Matrix4x4& m);
	Vector3 ExtractRotation(const Matrix4x4& m, const Vector3& scale);
};

