#pragma once
#include "WinApp.h"
#include "DirectXCommon.h"
#include "ImGuiManager.h"
#include "PipelineStateManager.h"
#include "ImGuiManager.h"

#include "Triangle.h"
#include "Sprite.h"
#include "Camera.h"
#include "DirectionalLight.h"
#include "Model.h"
#include "Text.h"



namespace PonikoEngine {

		/// <summary>
		/// エンジンの初期化
		/// </summary>
		/// <param name="title">ウィンドウタイトル</param>
		void Initialize(const wchar_t* title = L"ここにタイトルを入力");

		/// <summary>
	/// エンジンの終了処理
	/// </summary>
		void Finalize();
};