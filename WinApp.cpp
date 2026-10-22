#include "WinApp.h"
#include "ImGuiManager.h"

#include "FileDropManager.h"

#include "Input.h"

bool WinApp::isDropFile = false;

// imguiで必要奴
#ifdef USE_IMGUI
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

LRESULT CALLBACK WinApp::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

#ifdef USE_IMGUI

		// imguiの儀式
		if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam)) {
			return true;
		}
#endif

		switch (message)
		{
		case WM_MOUSEMOVE:

			// マウス座標セット
			Input::GetInstance()->SetMousePosition(LOWORD(lParam), HIWORD(lParam));

			break;
		case WM_MOUSEWHEEL:

			// ホイール
			Input::GetInstance()->SetMouseWheel(GET_WHEEL_DELTA_WPARAM(wParam));

			break;

		// それぞれ右左クリック押された、判定など
		case WM_LBUTTONDOWN:
			Input::GetInstance()->SetLeftButton(true);
			break;

		case WM_LBUTTONUP:
			Input::GetInstance()->SetLeftButton(false);
			break;

		case WM_RBUTTONDOWN:
			Input::GetInstance()->SetRightButton(true);
			break;

		case WM_RBUTTONUP:
			Input::GetInstance()->SetRightButton(false);
			break;

		/* ウインドウにドロップされたファイルのパスをゲット */
		case WM_DROPFILES: {

			if (isDropFile) {

				HDROP drop = reinterpret_cast<HDROP>(wParam);

				UINT count = DragQueryFileW(drop, 0xFFFFFFFF, nullptr, 0);

				for (UINT i = 0; i < count; ++i) {
					wchar_t path[MAX_PATH];

					DragQueryFileW(drop, i, path, MAX_PATH);

					// ファイルパスクラスに追加。
					FileDropManager::AddFilePath(std::wstring(path));

				}

				DragFinish(drop);
			}

			return 0;
		}

		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		return 0;

};

WinApp* WinApp::GetInstance()
{
	static WinApp instance;
	return &instance;
};

void WinApp::Initialize(const wchar_t* title, int32_t clientW, int32_t clientH) {

	clientWidth = clientW;
	clientHeight = clientH;

	// 登録
	wc_ = {};
	wc_.lpfnWndProc = WndProc;
	wc_.lpszClassName = title;
	wc_.hInstance = GetModuleHandle(nullptr);
	wc_.hCursor = LoadCursor(nullptr, IDC_ARROW);
	if (!RegisterClass(&wc_)) {
		return;
	};

	// サイズ設定
	RECT wrc = { 0, 0, clientWidth, clientHeight };

	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, FALSE);

	// ウインドウ作成
	hwnd_ = CreateWindow(
		wc_.lpszClassName,
		title,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wrc.right - wrc.left,
		wrc.bottom - wrc.top,
		nullptr, nullptr,
		wc_.hInstance,
		nullptr
	);

	// ウインドウ表示
	ShowWindow(hwnd_, SW_SHOW);

	// ウインドウにファイルのドロップの有効化
	DragAcceptFiles(hwnd_, TRUE);


	if (!hwnd_)
	{
		return;
	}

};

void WinApp::Finalize() {

	CloseWindow(hwnd_); 

	//DestroyWindow(hwnd_);
	//UnregisterClass(wc_.lpszClassName, wc_.hInstance);
};

bool WinApp::ProcessMessage()
{
	MSG msg{};

	if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		if (msg.message == WM_QUIT)
		{
			return true;
		}
	}

	return false;
}