#pragma once

#include <windows.h>

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_win32.h"
#include "externals/imgui/imgui_impl_dx12.h"
#endif

class ImGuiManager
{
public:
	// シングルトン
	static ImGuiManager* GetInstance();

	// 初期化
	void Initialize();

	// 受付開始
	void Begin();

	// 受付終了
	void End();

	// 描画
	void Draw();

	// 終了処理
	void Finalize();

private:
	ImGuiManager() = default;

};

