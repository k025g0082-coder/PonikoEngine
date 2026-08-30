#pragma once
#include <vector>

#include "Camera.h"
#include "Vector.h"

// カメラ位置情報
struct frame {
	Vector3 translation_;
	Vector3 rotation_;
	float duration_; //かかる時間
};

class Event {
public:
	virtual ~Event() = default;

	static void Init();
	virtual void Initialize() = 0;
	virtual void Update() = 0;

	// 発火の条件チェック
	virtual void TriggerCheck() = 0;


	bool IsFinished() { return isFinished_; };
	bool IsExecuted() { return isExecuted_; };

	// 地点の情報
	struct railInfo {
		float railIndex;
		float pointIndex;
	};

	// カメラげったー
	static Camera& GetCamera() { return camera_; };

protected:


	bool isExecuted_ = false;
	bool isFinished_ = false;

private:
	static Camera camera_;
};

//// ボス出現イベ
//class BossEvent : public Event {
//public:
//	void Initialize() override;
//	void Update(GameScene* gamescene_) override;
//
//private:
//	railInfo targetRail_;
//
//	float timer_ = 0.0f;
//
//	/* こっからはいつか必ずやる */
//	/* イージングさせるから、最初のカメラの場所と角度が必要になるはず。　*/
//
//	// ターゲットのカメラ位置にイージングする時間
//	float easingInterval_ = 60.0f;
//
//	// ボス全体の
//	Vector3 targetTranslation_ = {-126.0f, 107.0f, -366.0f};
//	Vector3 targetRotation_ = {0.0f, 0.440f, 0.0f};
//
//	Vector3 beforeTranslation_;
//	Vector3 beforeRotation_;
//
//	Vector3 translation_ = {0, 0, 0};
//	Vector3 rotation_ = {0, 0, 0};
//
//	std::vector<frame> frames_;
//	float nowFrameStep_ = 0.0f;
//
//	enum Phase {
//		phaseAppearanse,
//		phaseZoomout
//	};
//	Phase currentPhase = phaseAppearanse;
//
//};
