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
class Input
{
public:    

	// シングルトン
	static Input* GetInstance();


	// 初期化
	void Initialize();

	// 更新
	void Update();

	// 終了処理
	void Finalize();


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
	
	// なにかボタンが押されたら
	bool PushAnyButton()
	{
		return state_.Gamepad.wButtons != 0 &&
			prevState_.Gamepad.wButtons == 0;
	}

private:

	//Input() = default;
	//~Input() = default;

	//// コピーコンストラクタを禁止
	//Input(const Input&) = delete;
	//Input& operator=(const Input&) = delete;


	XINPUT_STATE state_{};
	XINPUT_STATE prevState_{};


	BYTE key[256] = {};
	BYTE preKey[256] = {};

	IDirectInput8* directInput = nullptr;
	IDirectInputDevice8* keyboard = nullptr;
};

