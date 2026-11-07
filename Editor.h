#pragma once

#include "Model.h"
#include "Transform.h"

#include <imgui.h>

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

	/* モデル選択モード */
	enum class ModelHierarchyMode {
		kSelectMode, // 個別にサイズを調整できる
		kCreateMode, // ドラッグで一気に配置
	};


	HierarchyPanel& GetHierarchyPanel() { return hierarchyPanel_; }


	// エンジン閉じる際の設定
	void SaveIniFile();
	void LoadIniFile();

	/// <summary>
	/// 特定のシーンのヒエラルキー情報を読み込む
	/// </summary>
	/// <param name="path">シーンの相対パス。例 resources/scene/OOXX.json</param>
	static void LoadScene(std::string path);


	/* ラッパパパァｯｯ!!!　 */


	/// <summary>
	/// 連携を宣言したレイヤー名一覧のリストを取得
	/// </summary>
	/// <returns></returns>
	static std::vector<std::string>* GetAlignHierarchyLayerNames() {
		return hierarchyPanel_.getAlignHierarchyLayerNames();
	}

	/// <summary>
	/// 対象のレイヤー名からヒエラルキをレイヤーから一覧取得. ないとき/SetUseGameObjectLayer()がされていない場合は空
	/// </summary>
	/// <param name="layerName">レイヤー名</param>
	/// <returns></returns>
	static std::vector<Hierarchy<Model>*>& GetHierarchyModelLayer(std::string layerName) {
		return hierarchyPanel_.GetHierarchyModelLayer(layerName);
	}


	/// <summary>
	/// マウスがメインビューの左上を原点0,0として、矩形内ににあるかを返す
	/// </summary>
	/// <param name="min">左上の座標</param>
	/// <param name="max">右上の座標</param>
	/// <returns>あるか</returns>
	bool IsMainViewInsideMouse(Vector2 min,Vector2 max) {
		return  maiViewLocalMouseX >= min.x &&
				maiViewLocalMouseX <= max.x &&
				mainViewLocalMouseY >= min.y &&
				mainViewLocalMouseY <= max.y;
	}


	// 1280pxを基にした時の現在の画面比率をかえす
	float GetMainViewRatio()  { return mainViewSize.x / 1280.0f; };
	
private:


	ModelHierarchyMode currentModelHierarchyMode_ = ModelHierarchyMode::kSelectMode;

	//* モデルSRT操作モード
	ImGuizmo::OPERATION operation_ = ImGuizmo::TRANSLATE;


	// エディターモードか
	bool isEditorMode_ = false;

	// 判定用のAABB描画するか
	bool isDrawAABB_ = true;
	

	// マウス座標
	ImVec2 mouse;

	// ウィンドウ内の描画開始位置（左上のスクリーン座標）
	ImVec2 mainViewPos;  

	// 利用可能な幅と高さ
	ImVec2 mainViewSize;  

	// ウインドウの｛０，０｝から見たローカル座標
	float maiViewLocalMouseX;
	float mainViewLocalMouseY;

	// ウインドウ大きさ
	float mainViewWidth;
	float mainViewHeight;


	/*
		デバッグカメラ
	*/
	Camera camera_;

	float camDistance = 8.0f;

	Vector3 tmpCameraPosition_ = {0,0,-50.0f};
	Vector3 tmpCameraRotation_;

	// 非選択時のカメラ移動
	void FreeMoveCamera();


	/* 
		各パネル
	*/
	static HierarchyPanel hierarchyPanel_;
	AssetsPanel assetsPanel_;

	static Grid grid;
	

	/*
		メインとなる各操作
	*/

	// ウインドウ上部
	void TopMenuVar();


	// 矢印描画＆選択しているののWorldTransformを変形
	void DrawArrowToSelectedTransform();

	// 便利な四角
	void DrawViewManipulate();


	// スクリーン上からレイを飛ばしてヒエラルキを選択
	void UpdateHierarchySelection();

	// 各操作
	void OperationKeyBoard();
	void OperationLeftClick();
	void OperationRightClick();
	void OperationWheel();




	/*
		仮のタブたち
	*/

	//
	void PropartyTab();

	//　ライティング用
	void LightingManagerTab(bool& show);

	//　アセット一覧用
	void AssetsTab(bool& show);

	// エフェクトエディタ
	void EffectEditorTimeLine();
	EffectSequencer effectSequencer_;


	// 選択中のモデルのView更新
	void SelectModelUpdateView();

	// 選択中のモデルのワールドトランスフォーム
	WorldTransform* selectedModelWorldTransform_ = nullptr;

	// 距離
	float selectedModelLength_ = 0.0f;

	// との球面座標系の色イロ
	float selectedSphereTheta_;
	float selectedSpherePhi_;


	// 今読み込んでるシーンのJsonパス
	static std::string currentSceneJsonPath;
	std::string currentSceneName_;

	
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

