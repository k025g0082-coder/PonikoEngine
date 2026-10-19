#pragma once

#include "Model.h"

// これ外部ライブラリ
#include "ImGuizmo.h"

#include "DirectionalLight.h"
#include "SpotLight.h"
#include "PointLight.h"

#include "Camera.h"

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

private:

	// エディターモードか
	bool isEditorMode_ = false;

	// デバッグ用
	Camera camera_;


	// Hierarchyタブ
	void HierarchyTab(bool& show);

	// タイプ
	enum class HierarchyType {
		kModel
	};

	template <typename Type>

	/* Hierarchy */
	struct Hierarchy {

		HierarchyType type_;
		std::string name_;

		// オブジェクトのアドレスのポインタ
		Type* object = nullptr;

	};

	// モデル
	std::vector<Hierarchy<Model>> hierarchyModels_;

	// 選択中モデル
	Model* selectedModel_ = nullptr;


	//
	void LightingManagerTab(bool& show);

	// Editor.h
	ImGuizmo::OPERATION operation_ = ImGuizmo::TRANSLATE;

	Vector3 ExtractScale(const Matrix4x4& m);
	Vector3 ExtractRotation(const Matrix4x4& m, const Vector3& scale);
};

