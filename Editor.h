#pragma once

#include "Model.h"
#include "Transform.h"

#include "imgui.h"

// これ外部ライブラリ
#include "ImGuizmo.h"

#include "EffectSequencer.h"


#include "Camera.h"

// 各パネル
#include "HierarchyPanel.h"
#include "AssetsPanel.h"



// ...

#include "EasingVec.h"

#include "Matrix.h"

#include "Plane.h"

#include <string>
#include <vector>

#include <memory>
#include <unordered_map>


/* グリッドの設定まとめ */
class Grid {

public:

	Matrix4x4 rotationMatrix = MakeIdentity4x4();

	// 縦
	int vertical_   = 10;

	// 横
	int horizontal_ = 10;

	static float gridWidth_;
	static float gridHeight_;

	//　？？
	static float gridDepth_;

	static Vector3 translation_;


Vector3 rotation_;

	bool isSnap_ = false;
	static bool isCenter_;

	// 判定用のplane(縦横)
	PlaneArea plane_;

	void Update();
	void AddLine();

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

	/// <summary>
	/// 特定のシーンのヒエラルキー情報を読み込む
	/// </summary>
	/// <param name="path">シーンの相対パス。例 resources/scene/OOXX.json</param>
	static void LoadScene(std::string path);


	// エンジン閉じる際の設定
	void SaveIniFile();
	void LoadIniFile();

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
	static HierarchyPanel hierarchyPanel_;
	AssetsPanel assetsPanel_;

	static Grid grid;
	
	//* モデルSRT操作モード
	ImGuizmo::OPERATION operation_ = ImGuizmo::TRANSLATE;

	// 今読み込んでるシーンのJsonパス
	static std::string currentSceneJsonPath;
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

	// カワイイスイッチ
	bool ToggleSwitch(const char* label, bool* value);

	// 通知？
	struct Notification {

		/// <summary>
		/// 通知
		/// </summary>
		/// <param name="text">文字</param>
		/// <param name="Size">ウインドウサイズ</param>
		void PopText(const char* text, ImVec2 Size = { 240.0f, 30.0f });
		void Draw();

		const char* text = "";
		
		EasingVec ease;

		ImVec2 size = { 240.0f, 0.0f };
		ImVec2 endSize_ = { 240.0f, 30.0f };

		bool isPop_ = false;
		bool isClose = false;

		float timer_ = 0.0f;

		// 秒
		float easeTime = 0.3f;
		float easeEndTime = 1.5f;

		ImVec4 backCol_ = ImVec4(40, 40, 45, 230);
		ImVec4 textCol_ = ImVec4(0, 0, 0, 230);
	};
	Notification notification_;

	void DrawNotification(const char* text);


	void SaveIniFileToVector3(Vector3 v, const char* janre, const char* tag);
	void LoadIniFileToVector3(Vector3& v, const char* janre, const char* tag);

	bool ini = false;
};

