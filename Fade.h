#pragma once
#include "Sprite.h"

class Fade {
public:
	void Initialize();
	void Update();
	void Draw();

	// フェードの状態
	enum class Status {
		None, // フェードなし
		FadeIn,  // フェードイン中
		FadeOut, // フェードアウト中
	};
	Status status_ = Status::None;

	// フェード開始
	void Start(Status status, float dration);
	void Stop();

	// 再生ちゅうか
	bool IsPlayFading() { return isFading_; }

	// 終了判定
	bool IsFinished() const;

private:
	Sprite sprite;

	float duration_ = 0.0f; // 計測時間
	float counter_ = 0.0f;  // 経過時間カウンター

	bool isFading_ = false;  // 再生ちゅうか
};