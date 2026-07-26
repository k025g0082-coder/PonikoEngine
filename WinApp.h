#pragma once
#include "Windows.h"
#include <cstdint>

/// <summary>
/// ウインドウ管理クラス
/// </summary>
class WinApp
{
public:

	// シングルトン
	// 処理はｃｐｐに書く
	static WinApp* GetInstance();

	void Initialize(const wchar_t* title, int32_t clientWidth, int32_t clientHeight);
	void Finalize();

	HWND GetHWND() const{ return hwnd_; }

	int32_t GetClientWidth() const { return clientWidth; }
	int32_t GetClientHeight() const { return clientHeight; }

	// メッセージを転送する
	bool ProcessMessage();
private:

	WinApp() = default;

	// ウインドウプロシージャ
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	WNDCLASS wc_{};
	HWND hwnd_ = nullptr;

	// 大きさ
	int32_t clientWidth;
	int32_t clientHeight;

};

