#pragma once
#include "WinApp.h"
#include "DirectXCommon.h"

#include "Triangle.h"
#include "Sprite.h"
#include "Camera.h"
#include "DirectionalLight.h"
#include "Model.h"

namespace PonikoEngine {

		/// <summary>
		/// エンジンの初期化
		/// </summary>
		/// <param name="title">ウィンドウタイトル</param>
		void Initialize(const wchar_t* title = L"ここにタイトルを入力");
		/// <summary>
		/// エンジンのアプデ
		/// </summary>
		bool Update();
		/// <summary>
	/// エンジンの描画
	/// </summary>
		bool Draw();
		/// <summary>
	/// エンジンの終了処理
	/// </summary>
		void Finalize();
};