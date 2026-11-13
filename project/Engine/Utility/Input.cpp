#include "Input.h"

#include "WinApp.h"

#include "Game.h"

#include "Editor.h"


// シングルトン
Input* Input::GetInstance(){
	static Input instance;
	return &instance;
}

void Input::Initialize()
{
	state_ = {};
	prevState_ = {};

	// キーボードの初期化
	// DirectInputの初期化
	DirectInput8Create(
		WinApp::GetInstance()->GetWNDCLASS().hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8,
		(void**)&directInput, nullptr);

	// キーボードデバイスの生成
	directInput->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);

	//入力データ形式のセット
	keyboard->SetDataFormat(&c_dfDIKeyboard);//標準形式

	//排他制御レベルのセット
	keyboard->SetCooperativeLevel(
		WinApp::GetInstance()->GetHWND()
		, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
};

void Input::Update() {

	
	// マウス座標を、現在のViewの大きさから計算( 1280x720 からSceneのRenderTargetの縦横幅を引いて求めてる )
	//float scaleX = 
	//	static_cast<float>(WinApp::GetInstance()->GetClientWidth() / Game::GetInstance()->GetGameScreenWidth());
	//
	//float scaleY = 
	//	static_cast<float>(WinApp::GetInstance()->GetClientHeight() / Game::GetInstance()->GetGameScreenHeight());

	//mousePositionX_ = static_cast<int>(
	//	(tmpMousePositionX_ - Game::GetInstance()->GetGameScreenOffset().x) * scaleX);

	//mousePositionY_ = static_cast<int>(
	//	(tmpMousePositionY_ - Game::GetInstance()->GetGameScreenOffset().y) * scaleY);


	// 左クリ - 押された瞬間：今フレームON && 前フレームOFF
	leftTriggerButton_ = (leftButton_ && !prevLeftButton_);

	// 左クリ - 離された瞬間：今フレームOFF && 前フレームON
	leftReleaseButton_ = (!leftButton_ && prevLeftButton_);

	/* 左クリ - 前フレームの状態を更新 */
	prevLeftButton_ = leftButton_;


	// 右クリ - 押された瞬間：今フレームON && 前フレームOFF
	rightTriggerButton_ = (rightButton_ && !prevRightButton_);

	// 右クリ - 離された瞬間：今フレームOFF && 前フレームON
	rightReleaseButton_ = (!rightButton_ && prevRightButton_);

	/* 右クリ - 前フレームの状態を更新 */
	prevRightButton_ = rightButton_;


	// キーボード
	keyboard->Acquire();

	memcpy(preKey, key, sizeof(key));

	keyboard->GetDeviceState(sizeof(key), key);

	// ゲームパッド
	prevState_ = state_;

	DWORD result = XInputGetState(0, &state_);

	if (result != ERROR_SUCCESS)
	{
		state_ = {};
	}

	mouseWheel_ = 0;
};

void Input::Finalize() {
	if (keyboard) {
		keyboard->Unacquire();
		keyboard->Release();
		keyboard = nullptr;
	}
	if (directInput) {
		directInput->Release();
		directInput = nullptr;
	}
};

// おした瞬間
bool Input::TriggerButton(WORD button)
{
	return
		(state_.Gamepad.wButtons & button) &&
		!(prevState_.Gamepad.wButtons & button);
};

// おされている
bool Input::PushButton(WORD button)
{
	return
		(state_.Gamepad.wButtons & button) &&
		(prevState_.Gamepad.wButtons & button);
};

// 左スティック　０～１．０ｆ
Vector2 Input::GetLeftStick()
{
	Vector2 result{};

	result.x = state_.Gamepad.sThumbLX / 32767.0f;
	result.y = state_.Gamepad.sThumbLY / 32767.0f;

	return result;
};

// 右スティック　０～１．０ｆ
Vector2 Input::GetRightStick()
{
	Vector2 result{};

	result.x = state_.Gamepad.sThumbRX / 32767.0f;
	result.y = state_.Gamepad.sThumbRY / 32767.0f;

	return result;
}
void Input::SetMousePosition(int x, int y)
{
	mousePositionX_ = x;
	mousePositionY_ = y;

	tmpMousePositionX_ = x;
	tmpMousePositionY_ = y;
	
	// もしエディターモードなら
	if (Editor::IsEditorMode())
		EditorViewAspectPosition();
}

void Input::EditorViewAspectPosition()
{

#ifdef USE_IMGUI

	ImVec2 mousePos = Editor::GetInstance()->GetMousePos();
	ImVec2 viewPos = Editor::GetInstance()->GetMainViewPos();
	ImVec2 viewSize = Editor::GetInstance()->GetMainViewSize();

	// マウスが乗ってるか
	bool insideMainView =
		mousePos.x >= viewPos.x &&
		mousePos.x <= viewPos.x + viewSize.x &&
		mousePos.y >= viewPos.y &&
		mousePos.y <= viewPos.y + viewSize.y;

	if (insideMainView) {

		// MainViewの左上を原点(0,0)とした座標
		float localX = mousePos.x - viewPos.x;
		float localY = mousePos.y - viewPos.y;

		// 1280x720基準の座標に変換
		float aspectX = 1280.0f / viewSize.x;
		float aspectY = 720.0f / viewSize.y;

		mousePositionX_ = static_cast<int>(localX * aspectX);
		mousePositionY_ = static_cast<int>(localY * aspectY);
	}
	else {
		mousePositionX_ = 0;
		mousePositionY_ = 0;
	}

#endif

}
