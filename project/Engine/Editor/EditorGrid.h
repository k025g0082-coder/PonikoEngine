#pragma once

#include "Matrix.h"

#include "Plane.h"
#include <vector>


/* グリッドの設定まとめ */
class EditorGrid {

public:

	Matrix4x4 rotationMatrix = MakeIdentity4x4();

	// 縦
	int vertical_ = 10;

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

