#pragma once

#include <Windows.h>


#define DIRECTINPUT_VERSION 0x0800 //バージョン指定
#include <dinput.h>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")


#include <Xinput.h>

#pragma comment(lib, "xinput.lib")

#include "Vector.h"


// 凄
class GamePad
{
public:    
	// シングルトン
	static GamePad* GetInstance();

	void Initialize();

	void Update() {

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
	};

	// おした瞬間
// おした瞬間
	bool TriggerButton(WORD button);

	// おされている
	bool PushButton(WORD button);

	// 左スティック　０～１．０ｆ
	Vector2 GetLeftStick();

	// 右スティック　０～１．０ｆ
	Vector2 GetRightStick();

	// キーボードトリガー処理
	bool PushKey(BYTE keyNum) { return key[keyNum]; }
	bool ReleasedKey(BYTE keyNum) { return !key[keyNum]; }

	bool TriggerKey(BYTE keyNum) {return (key[keyNum] && !preKey[keyNum]);}
	bool ReleaseKey(BYTE keyNum) {return (!key[keyNum] && preKey[keyNum]);}

private:
	XINPUT_STATE state_{};
	XINPUT_STATE prevState_{};


	BYTE key[256] = {};
	BYTE preKey[256] = {};

	IDirectInput8* directInput = nullptr;
	IDirectInputDevice8* keyboard = nullptr;
};

