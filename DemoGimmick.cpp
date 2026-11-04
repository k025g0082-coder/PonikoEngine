#include "DemoGimmick.h"
#include  <misc/cpp/imgui_stdlib.h>
#include <imgui_internal.h>

#include "FieldExample.h"

// ギミックの種類名
const char* DemoGimmick::GimmickTypeName[(int)GimmickType::count] = {
	"Switch",
	"Door",
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

	// しかけ発動
	if (!isProcess_ && pushScaleYEase.getIsFinished()) {

		// なんかエフェクト

		// Field側からインスタンスのマップもらって登録したuniqueIDの全ての発火された関数を実行
		for (uint32_t id : processIDs_) {

			auto* it = FieldExample::GetUniqueIdToFieldObject(id);
			if (!it) continue; // ★ 未登録ならスキップ
			
			DemoGimmick* gimmick = dynamic_cast<DemoGimmick*>(it);

			/* 相手側の発火された関数を実行 */
 			gimmick->ByTriggerGimmick(this);

		}

		isProcess_ = true;

	}

}
void DemoSwitch::Draw()
{
	DemoGimmick::Draw();
}

void DemoSwitch::ImGuiView()
{
#ifdef USE_IMGUI
	DemoFieldObject::BeginImGui();


	ImGui::Text("processIDs: ");
	ImGui::Separator();

	static int v;
	ImGui::InputInt("type: ", &v);

	ImGui::SameLine();

	// 新規ID 追加
	if (ImGui::Button("Add")) {
		AddID((uint32_t)v);
	}
	ImGui::Separator();

	// 連携してるのを表示
	for (auto& id : processIDs_) {
		ImGui::Text("%d,",id);
	}





	DemoFieldObject::EndImGui();
#endif
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


void DemoDoor::Update()
{
	DemoGimmick::Update();

	// 開けるアニメ
	if (openPositionEase.getIsEasing()) {
		openPositionEase.Step();
		worldTransform.translation_.y = openPositionEase.getEasingNum();
	}
}

void DemoDoor::Draw()
{
	DemoGimmick::Draw();
}

/* 
	他のギミックから発火された際
*/
void DemoDoor::ByTriggerGimmick(DemoGimmick* gimmick)
{

	GimmickType subType = static_cast<GimmickType>(gimmick->GetSubId());

	switch (subType) {
	case GimmickType::kSwitch:

		// 開けるアニメ開始 - 適当にY方向に移動
		openPositionEase.Start(EasingVec::EASE_OutSine, 20.0f, worldTransform.translation_.y, worldTransform.translation_.y - 5.0f);

		break;
	}

}
