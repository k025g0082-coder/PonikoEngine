#include "DemoGimmick.h"


// ギミックの種類名
const char* DemoGimmick::GimmickTypeName[(int)GimmickType::count] = {
	"Switch",
	"Goal"
};


// 基底
void DemoGimmick::Initialize()
{
	//
}

void DemoGimmick::Update()
{
	DemoFieldObject::Update();
}
void DemoGimmick::Draw()
{
	DemoFieldObject::Draw();
}


void DemoSwitch::Update()
{
	DemoGimmick::Update();

	// 押されたときの
	if (isPush_) {
		pushScaleYEase.Step();
		worldTransform.scale_.y = pushScaleYEase.getEasingNum();
		aabb_.height_ = pushScaleYEase.getEasingNum();
	}

	if (pushScaleYEase.getIsFinished()) {
		// なんかエフェクト
	}

}
void DemoSwitch::Draw()
{
	DemoGimmick::Draw();
}

void DemoSwitch::onCollisionToPlayer(DemoPlayer* player, CollisionResult result)
{
	(void)player;
	(void)result;

	if (isPush_)return;

	// もし上から踏まれた
	if (!result.isGrounded) return;

	// おされたアニメ開始
	pushScaleYEase.Start(EasingVec::EASE_None, 10.0f, 1.0f, 0.0f);

	isPush_ = true;
}


void DemoGoal::Update()
{
	DemoGimmick::Update();
}
void DemoGoal::Draw()
{
	DemoGimmick::Draw();
}

void DemoGoal::onCollisionToPlayer(DemoPlayer* player)
{
	player;
}

