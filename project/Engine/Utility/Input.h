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

	void SetMousePosition(int x, int y);

	// マウス座標取得
	Vector2 GetMousePosition() {
		return Vector2{
			static_cast<float> (mousePositionX_),
			static_cast<float> (mousePositionY_),
		};
	}

	// 左クリック
	void SetLeftButton(bool isDown) { 
		
		leftButton_ = isDown;

	}

	bool GetLeftMouseButtonDown() const { return leftButton_; }

	// 右クリック
	void SetRightButton(bool isDown) {

		rightButton_ = isDown;

	}

	bool GetRightMouseButtonDown() const { return rightButton_; }


	// 押した瞬間 (Trigger)
	bool GetLeftMouseButtonTrigger() const { return leftTriggerButton_; }
	bool GetRightMouseButtonTrigger() const { return rightTriggerButton_; }

	// 離した瞬間 (Release)
	bool GetLeftMouseButtonRelease() const { return leftReleaseButton_; }
	bool GetRightMouseButtonRelease() const { return rightReleaseButton_; }


	// ホイール操作
	int GetMouseWheel() const { return mouseWheel_; }

	void SetMouseWheel(int value) {
		mouseWheel_ = value;
	}

private:

	//Input() = default;
	//~Input() = default;

	//// コピーコンストラクタを禁止
	//Input(const Input&) = delete;
	//Input& operator=(const Input&) = delete;

	int mousePositionX_ = 0;
	int mousePositionY_ = 0;

	int tmpMousePositionX_ = 0;
	int tmpMousePositionY_ = 0;


	bool leftButton_ = false;
	bool prevLeftButton_ = false;   // 前の状態

	bool leftTriggerButton_ = false;// 押された瞬間
	bool leftReleaseButton_ = false;// 離された瞬間


	bool rightButton_ = false;
	bool prevRightButton_ = false;	 // 前の状態

	bool rightTriggerButton_ = false;// 押された瞬間
	bool rightReleaseButton_ = false;// 離された瞬間


	// エディター内のメインビューに合わせた座標
	void EditorViewAspectPosition();


	int mouseWheel_ = 0;

	XINPUT_STATE state_{};
	XINPUT_STATE prevState_{};

	BYTE key[256] = {};
	BYTE preKey[256] = {};

	IDirectInput8* directInput = nullptr;
	IDirectInputDevice8* keyboard = nullptr;
};

