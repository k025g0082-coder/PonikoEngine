#include "GamePad.h"

#include "WinApp.h"

// シングルトン
GamePad* GamePad::GetInstance(){
	static GamePad instance;
	return &instance;
}

void GamePad::Initialize()
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

// おした瞬間
bool GamePad::TriggerButton(WORD button)
{
	return
		(state_.Gamepad.wButtons & button) &&
		!(prevState_.Gamepad.wButtons & button);
};

// おされている
bool GamePad::PushButton(WORD button)
{
	return
		(state_.Gamepad.wButtons & button) &&
		(prevState_.Gamepad.wButtons & button);
};

// 左スティック　０～１．０ｆ
Vector2 GamePad::GetLeftStick()
{
	Vector2 result{};

	result.x = state_.Gamepad.sThumbLX / 32767.0f;
	result.y = state_.Gamepad.sThumbLY / 32767.0f;

	return result;
};

// 右スティック　０～１．０ｆ
Vector2 GamePad::GetRightStick()
{
	Vector2 result{};

	result.x = state_.Gamepad.sThumbRX / 32767.0f;
	result.y = state_.Gamepad.sThumbRY / 32767.0f;

	return result;
};