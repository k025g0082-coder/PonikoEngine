#include "Fade.h"
#include "WinApp.h"

#include <algorithm>

#include "DirectXUtility.h"

void Fade::Initialize() {

	sprite.Initialize("resources/black100x100.png");
	sprite.SetSize(
		WinApp::GetInstance()->GetClientWidth(),
		WinApp::GetInstance()->GetClientHeight());
	sprite.SetPos({ 0.0f, 0.0f, 0.0f });

};

void Fade::Update() {

	sprite.UpdateScreenTransform();

	// フェード状態による分岐
	switch (status_) {
	case Status::None:
		// 何もしない
		break;
	case Status::FadeIn:
		// フェードイン中の更新処理(次ページ以降で実装)

		// 1フレーム分の秒数をカウントアップ
		counter_ += 1.0f / 60.0f;
		// フェード継続時間に達したら打ち止め
		if (counter_ >= duration_) {
			counter_ = duration_;
		}
		// 0.0fから1.0fの間で、経過時間がフェード継続時間に近づくほどアルファ値を大きくする
		sprite.SetColorRGBA(Vector4(0, 0, 0, 1.0f - std::clamp(counter_ / duration_, 0.0f, 1.0f)));
		
		break;

	case Status::FadeOut:
		// フェードアウト中の更新処理(次ページ以降で実装)
		// 
		// 1フレーム分の秒数をカウントアップ
		counter_ += 1.0f / 60.0f;
		// フェード継続時間に達したら打ち止め
		if (counter_ >= duration_) {
			counter_ = duration_;
		}
		// 0.0fから1.0fの間で、経過時間がフェード継続時間に近づくほどアルファ値を大きくする
		sprite.SetColorRGBA(Vector4(0, 0, 0, std::clamp(counter_ / duration_, 0.0f, 1.0f)));

		
		break;
	}
};
void Fade::Draw() {

	if (status_ == Status::None) {
		return;
	}

	sprite.Draw();

};

void Fade::Start(Status status, float duration) { 
	status_ = status;
	duration_ = duration;
	counter_ = 0.0f;
}

void Fade::Stop() { status_ = Status::None; };

bool Fade::IsFinished() const {
	// フェード状態による分岐
	switch (status_) {
	case Status::FadeIn:
	case Status::FadeOut:
		if (counter_ >= duration_) {
			return true;
		} else {
			return false;
		}
	}
	
	return true;
};