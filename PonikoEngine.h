#pragma once
#include "WinApp.h"
#include "DirectXCommon.h"
#include "ImGuiManager.h"
#include "PipelineStateManager.h"
#include "ImGuiManager.h"
#include "EffekseerManager.h"
#include "SoundManager.h"

#include "Triangle.h"
#include "Sprite.h"
#include "Camera.h"
#include "LightingManager.h"
#include "EffectManager.h"

#include "PostEffectManager.h"

#include "DebugLine.h"

#include "Model.h"
#include "Text.h"

#include "Input.h"

#include "Game.h"
#include "Particle.h"

#include "Timer.h"


namespace PonikoEngine {

		/// <summary>
		/// エンジンの初期化
		/// </summary>
		/// <param name="title">ウィンドウタイトル</param>
		void Initialize(const wchar_t* title = L"ここにタイトルを入力test");

		/// <summary>
	/// エンジンの終了処理
	/// </summary>
		void Finalize();

		/// <summary>
	/// エンジンのUpdate
	/// </summary>
		void Update();

		/// <summary>
		/// エンジンのDraw
		/// </summary>
		void Draw();
};